int printf(const char*,...);
struct S { int m; }; union U { int m; };
struct B { unsigned f : 3; };
enum E { E0, E1 };
int g(void) { return 1; } void v(void) { }
int arr[3]; int mat[2][2];
struct S sv; union U uv; struct B bv; enum E ev;
int i; double d; float fl; char ch; int *p; _Bool b; long lo; _Complex double cx;
const int ci = 1; volatile int vi = 2;
int main(void) { i = v() ? 1 : 0 ; return 0; }
