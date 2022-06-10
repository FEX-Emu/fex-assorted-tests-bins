
template<typename Fn>
struct CopyableTrampoline;

template<typename R, typename... Args>
struct CopyableTrampoline<R(Args...)> {
    using rv_t = R;

    using target_t = R(Args...);
    using marshaler_t = R(Args..., target_t *target);

    struct instance_info_t {
        target_t *target;
        marshaler_t *marshaler;
    };

    static R trampoline_canonical(Args... args) {
        //instance_info *info;
        //asm("movabs $0xAABBCCDDEEFF0011, %0" : "=r" (info));
        auto info = (instance_info_t*)&instance_info_canonical;
        return info->marshaler(args..., info->target);
    }

    static __attribute__((aligned(16), naked)) void instance_info_canonical() {
        __asm__(".quad 0");
        __asm__(".quad 0");
    }

    static target_t *make_instance(target_t *target, marshaler_t *marshaler, char *section_start, char *section_end) {

        auto section_size = section_end - section_start;
        printf ("Section len: %d\n", (int)section_size);

        // alloc
        auto instance = (uint8_t*)mmap(0, section_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        // copy
        memcpy(instance, section_start, section_size);

        // bind
        auto code_offset = (uintptr_t)&trampoline_canonical - (uintptr_t)section_start;
        auto info_offset = (uintptr_t)&instance_info_canonical - (uintptr_t)section_start;

        printf("code_offset offset: %d\n", (int)code_offset);
        printf("info_offset offset: %d\n", (int)info_offset);

        auto info = (instance_info_t*)(instance + info_offset);

        #if defined(ADDR_PATCH)
            auto patch_offset = (uintptr_t)&test_func_canonical_patch - (uintptr_t)__start_test_func_section;
            printf("patch_offset offset: %d\n", (int)patch_offset);
            auto patch = (instance_info_t**)(instance + patch_offset + 2); // +2 is architecture specific
            printf("Patching: %p from %p to %p\n", patch, *patch, info);
            *patch = info;
        #endif

        info->target = target;
        info->marshaler = marshaler;

        return (target_t *)(instance + code_offset);
    }
};

/*
// Oof, this cannot be done
template __attribute__((section("test_func_section"))) auto CopyableTrampoline<decltype(test_func)>:: 
trampoline_canonical(int a, const char *b) -> rv_t;
*/

#define DECL_COPYABLE_TRAMPLOLINE(name, rv, ...) \
template __attribute__((section(#name "_copy_section"))) void CopyableTrampoline<rv(__VA_ARGS__)>::instance_info_canonical(); \
template __attribute__((section(#name "_copy_section"))) auto CopyableTrampoline<rv(__VA_ARGS__)>:: \
    trampoline_canonical(__VA_ARGS__) -> rv_t; \
template auto CopyableTrampoline<rv(__VA_ARGS__)>:: \
    make_instance(target_t *target, marshaler_t *marshaler, char *section_start, char *section_end) -> target_t *; \
CopyableTrampoline<rv(__VA_ARGS__)>::target_t *make_##name##_instance(CopyableTrampoline<rv(__VA_ARGS__)>::target_t *target, CopyableTrampoline<rv(__VA_ARGS__)>::marshaler_t *marshal) { \
    extern char __start_##name##_copy_section[]; \
    extern char __stop_##name##_copy_section[]; \
    return CopyableTrampoline<rv(__VA_ARGS__)>::make_instance(target, marshal, __start_##name##_copy_section, __stop_##name##_copy_section); \
}
