#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <sys/mman.h>

// Example of creating function specializations at run time from a "canonical" template provided by the compiler

// variations
//#define FORCE_RIP_RELATIVE
#define ADDR_PATCH

int test_func (int a, const char *b);
int test_func_marshaler (int a, const char *b, decltype(&test_func) fn_ptr);

struct test_funct_instance_info {
    decltype(&test_func) fn_ptr;
    decltype(&test_func_marshaler) fn_marshal;
};


// Note: This function is copied around, any functions it uses should be in the same
//       segment, or they will not be bound correctly.
__attribute__((section("test_func_section"))) int test_func_canonical (int a, const char *b)
{
    #if defined(FORCE_RIP_RELATIVE)
        test_funct_instance_info *info;
        // force rip relative loads
        asm("lea test_funct_instance_info_canonical(%%rip), %0" : "=r" (info));
    #elif defined(ADDR_PATCH)
        test_funct_instance_info *info;
        asm("test_func_canonical_patch: movabs $0xAABBCCDDEEFF0011, %0" : "=r" (info));
    #else
        // this only works for x86-64, asm versions are needed for x86-32 and arm64
        void test_funct_instance_info_canonical();
        auto info = (test_funct_instance_info*)&test_funct_instance_info_canonical;
    #endif
    return info->fn_marshal(a, b, info->fn_ptr);
}

// Functions are used here to store data, because the linker doesn't like mixing code and data in a section
#if defined(FORCE_RIP_RELATIVE)
    // extern "C" so we can get the pointer from asm
    extern "C" 
#endif
__attribute__((aligned(16), naked, section("test_func_section"))) void test_funct_instance_info_canonical() {
    __asm__(".quad 0");
    __asm__(".quad 0");
}

#if defined(ADDR_PATCH)
extern "C" void test_func_canonical_patch();
#endif


int test_func_marshaler (int a, const char *b, decltype(&test_func) fn_ptr) {
    printf("%s: Called with %d, %s, %p\n", __func__, a, b, fn_ptr);

    return fn_ptr(a, b);
}

int test_func_to_call_1(int a, const char *b) { printf("%s: Called with %d, %s\n", __func__, a, b); return -1; }
int test_func_to_call_2(int a, const char *b) { printf("%s: Called with %d, %s\n", __func__, a, b); return -2; }

decltype(&test_func) test_func_make_instance(decltype(test_func) fn_ptr, decltype(test_func_marshaler) fn_marshal) {
    extern char __start_test_func_section[];
    extern char __stop_test_func_section[];

    auto test_funct_section_size = __stop_test_func_section - __start_test_func_section;
    printf ("Section len: %d\n", (int)test_funct_section_size);

    // alloc
    auto instance = (uint8_t*)mmap(0, test_funct_section_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    // copy
    memcpy(instance, __start_test_func_section, test_funct_section_size);

    
    // bind
    auto code_offset = (uintptr_t)&test_func_canonical - (uintptr_t)__start_test_func_section;
    auto info_offset = (uintptr_t)&test_funct_instance_info_canonical - (uintptr_t)__start_test_func_section;

    printf("code_offset offset: %d\n", (int)code_offset);
    printf("info_offset offset: %d\n", (int)info_offset);

    auto info = (test_funct_instance_info*)(instance + info_offset);

    #if defined(ADDR_PATCH)
        auto patch_offset = (uintptr_t)&test_func_canonical_patch - (uintptr_t)__start_test_func_section;
        printf("patch_offset offset: %d\n", (int)patch_offset);
        auto patch = (test_funct_instance_info**)(instance + patch_offset + 2); // +2 is architecture specific
        printf("Patching: %p from %p to %p\n", patch, *patch, info);
        *patch = info;
    #endif

    info->fn_ptr = fn_ptr;
    info->fn_marshal = fn_marshal;

    return (decltype(&test_func))(instance + code_offset);
}

int main (void)
{
    auto fn1 = test_func_make_instance(&test_func_to_call_1, &test_func_marshaler);
    auto fn2 = test_func_make_instance(&test_func_to_call_2, &test_func_marshaler);

    auto rv1 = fn1(1, "test stuff here 1");
    printf("rv1: %d\n", rv1);
    auto rv2 = fn2(2, "test stuff here 2");
    printf("rv2: %d\n", rv2);

    return 0;
}