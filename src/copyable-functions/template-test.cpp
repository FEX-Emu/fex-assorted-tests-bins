#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <sys/mman.h>

// Example of creating function specializations at run time from a "canonical" template provided by the compiler

#include "copyable-functions.h"

DECL_COPYABLE_TRAMPLOLINE(one_name, int, int, const char *)

DECL_COPYABLE_TRAMPLOLINE(two_name, int, const char *, int)

int test_func (int a, const char *b);
int test_func_marshaler (int a, const char *b, decltype(&test_func) fn_ptr) {
    printf("%s: Called with %d, %s, %p\n", __func__, a, b, fn_ptr);

    return fn_ptr(a, b);
}
int test_func_to_call_1(int a, const char *b) { printf("%s: Called with %d, %s\n", __func__, a, b); return -1; }
int test_func_to_call_2(int a, const char *b) { printf("%s: Called with %d, %s\n", __func__, a, b); return -2; }


int test_func2 (const char *a, int b);
int test_func2_marshaler (const char *a, int b, decltype(&test_func2) target) {
    printf("%s: Called with %s, %d, %p\n", __func__, a, b, target);

    return target(a, b);
}
int test_func2_to_call_1(const char *b, int a) { printf("%s: Called with %d, %s\n", __func__, a, b); return -1; }
int test_func2_to_call_2(const char *b, int a) { printf("%s: Called with %d, %s\n", __func__, a, b); return -2; }

int main (void)
{
    auto fn1 = make_one_name_instance(&test_func_to_call_1, &test_func_marshaler);
    auto fn2 = make_one_name_instance(&test_func_to_call_2, &test_func_marshaler);

    auto fn3 = make_two_name_instance(&test_func2_to_call_2, &test_func2_marshaler);
    auto fn4 = make_two_name_instance(&test_func2_to_call_2, &test_func2_marshaler);

    auto rv1 = fn1(1, "test stuff here 1");
    printf("rv1: %d\n", rv1);
    auto rv2 = fn2(2, "test stuff here 2");
    printf("rv2: %d\n", rv2);

    auto rv3 = fn3("test stuff here 1", 3);
    printf("rv3: %d\n", rv1);
    auto rv4 = fn4("test stuff here 2", 4);
    printf("rv4: %d\n", rv2);

    return 0;
}