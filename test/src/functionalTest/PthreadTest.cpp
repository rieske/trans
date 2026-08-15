#include "TestFixtures.h"

namespace {

TEST(Compiler, pthreadMutexCondTwoWorkers) {
    SourceProgram program{R"prg(
        #include <pthread.h>
        int printf(const char *, ...);
        static pthread_mutex_t mu;
        static pthread_cond_t cv;
        static int ready;
        static int acc;
        static void *worker(void *arg) {
            int n;
            n = *(int *)arg;
            pthread_mutex_lock(&mu);
            while (!ready)
                pthread_cond_wait(&cv, &mu);
            acc = acc + n;
            pthread_mutex_unlock(&mu);
            return 0;
        }
        int main(void) {
            pthread_t t1;
            pthread_t t2;
            int a;
            int b;
            a = 3;
            b = 4;
            pthread_mutex_init(&mu, 0);
            pthread_cond_init(&cv, 0);
            if (pthread_create(&t1, 0, worker, &a))
                return 1;
            if (pthread_create(&t2, 0, worker, &b))
                return 1;
            pthread_mutex_lock(&mu);
            ready = 1;
            pthread_cond_broadcast(&cv);
            pthread_mutex_unlock(&mu);
            pthread_join(t1, 0);
            pthread_join(t2, 0);
            printf("%d", acc);
            return 0;
        }
    )prg",
            {"-pthread"}};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, pthreadThreadParamsStealLoop) {
    SourceProgram program{R"prg(
        #include <pthread.h>
        int printf(const char *, ...);
        struct tp {
            pthread_t thread;
            int remaining;
            int working;
            int data_ready;
            pthread_mutex_t mutex;
            pthread_cond_t cond;
            int *processed;
        };
        static pthread_mutex_t progress_mu;
        static pthread_cond_t progress_cv;
        static void *work(void *arg) {
            struct tp *me;
            me = arg;
            pthread_mutex_lock(&progress_mu);
            while (me->remaining) {
                pthread_mutex_unlock(&progress_mu);
                *(me->processed) = *(me->processed) + me->remaining;
                me->remaining = 0;
                pthread_mutex_lock(&progress_mu);
                me->working = 0;
                pthread_cond_signal(&progress_cv);
                pthread_mutex_unlock(&progress_mu);
                pthread_mutex_lock(&me->mutex);
                while (!me->data_ready)
                    pthread_cond_wait(&me->cond, &me->mutex);
                me->data_ready = 0;
                pthread_mutex_unlock(&me->mutex);
                pthread_mutex_lock(&progress_mu);
            }
            pthread_mutex_unlock(&progress_mu);
            return 0;
        }
        int main(void) {
            struct tp p[8];
            int processed;
            int i;
            int active;
            processed = 0;
            active = 0;
            pthread_mutex_init(&progress_mu, 0);
            pthread_cond_init(&progress_cv, 0);
            i = 0;
            while (i < 8) {
                p[i].remaining = i + 1;
                p[i].working = 1;
                p[i].data_ready = 0;
                p[i].processed = &processed;
                pthread_mutex_init(&p[i].mutex, 0);
                pthread_cond_init(&p[i].cond, 0);
                if (pthread_create(&p[i].thread, 0, work, &p[i]) == 0)
                    active = active + 1;
                i = i + 1;
            }
            pthread_mutex_lock(&progress_mu);
            while (active) {
                i = 0;
                while (i < 8) {
                    if (!p[i].working && p[i].remaining == 0 && active) {
                        pthread_mutex_unlock(&progress_mu);
                        pthread_mutex_lock(&p[i].mutex);
                        p[i].data_ready = 1;
                        pthread_cond_signal(&p[i].cond);
                        pthread_mutex_unlock(&p[i].mutex);
                        pthread_mutex_lock(&progress_mu);
                        p[i].working = 1;
                        active = active - 1;
                    }
                    i = i + 1;
                }
                if (active)
                    pthread_cond_wait(&progress_cv, &progress_mu);
            }
            pthread_mutex_unlock(&progress_mu);
            i = 0;
            while (i < 8) {
                pthread_join(p[i].thread, 0);
                i = i + 1;
            }
            printf("%d %d", processed, active);
            return 0;
        }
    )prg",
            {"-pthread"}};
    program.compile();
    program.runAndExpect("36 0");
}

} // namespace
