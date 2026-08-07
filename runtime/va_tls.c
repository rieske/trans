/*
 * Thread-local va_list save-area pointer stack for pure-trans.
 *
 * Variadic prologues push the reg_save_area / overflow_arg_area addresses
 * here; va_start reads the top into the SysV va_list tag; returns pop.
 *
 * A single process-wide or single TLS slot is insufficient:
 * - Concurrent threads need per-thread state.
 * - Nested variadic calls need a stack so the outer frame's areas survive.
 *
 * Built with -fno-pic for non-PIE product links; keep this file free of
 * absolute string relocations (no fprintf) so PIE unit tests can link it too.
 */
#include <stdlib.h>

#define TRANS_VA_MAX_DEPTH 64

_Thread_local void *__trans_va_reg_save_stack[TRANS_VA_MAX_DEPTH];
_Thread_local void *__trans_va_overflow_stack[TRANS_VA_MAX_DEPTH];
_Thread_local int __trans_va_depth;

void __trans_va_set_areas(void *reg_save, void *overflow)
{
	if (__trans_va_depth >= TRANS_VA_MAX_DEPTH) {
		abort(); /* nested variadic depth exceeded */
	}
	__trans_va_reg_save_stack[__trans_va_depth] = reg_save;
	__trans_va_overflow_stack[__trans_va_depth] = overflow;
	__trans_va_depth++;
}

void __trans_va_pop_areas(void)
{
	if (__trans_va_depth > 0) {
		__trans_va_depth--;
	}
}

void *__trans_va_get_reg_save(void)
{
	if (__trans_va_depth <= 0) {
		return 0;
	}
	return __trans_va_reg_save_stack[__trans_va_depth - 1];
}

void *__trans_va_get_overflow(void)
{
	if (__trans_va_depth <= 0) {
		return 0;
	}
	return __trans_va_overflow_stack[__trans_va_depth - 1];
}
