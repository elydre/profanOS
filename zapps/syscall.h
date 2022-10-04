#ifndef SYSCALL_H
#define SYSCALL_H

#define FUNC_ADDR_SAVE 0x199990

#define UNUSED(x) (void)(x)
#define ARYLEN(x) (int)(sizeof(x) / sizeof((x)[0]))

// nothing better than shit code art
#define hi_func_addr(id) ((int (*)(int)) *(int *)FUNC_ADDR_SAVE)(id)

#define c_kprint(message) ((void (*)(char *)) hi_func_addr(47))(message)
#define c_fskprint(...) ((void (*)(char *, ...)) hi_func_addr(38))(__VA_ARGS__)
#define c_free(ptr) ((void (*)(void *)) hi_func_addr(17))(ptr)
#define c_malloc(size) ((void * (*)(int)) hi_func_addr(19))(size)
#define c_time_gen_unix() ((int (*)(void)) hi_func_addr(42))()


#endif
