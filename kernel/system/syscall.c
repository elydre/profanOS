#include <kernel/syscall.h>
#include <minilib.h>
#include <system.h>

void syscall_handler(registers_t *r) {
    kprintf_buf(sys_safe_buffer, "syscall %d\n", r->eax);
    sys_report(sys_safe_buffer);
}
