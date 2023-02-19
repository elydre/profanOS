typedef int jmp_buf[6];

int main() {
    return 0;
}

int setjmp(jmp_buf var) {
    asm (
        "mov    8(%ebp), %eax   \n" // get pointer to jmp_buf, passed as argument on stack
        "mov    %ebx, (%eax)    \n" // jmp_buf[0] = ebx
        "mov    %esi, 4(%eax)   \n" // jmp_buf[1] = esi
        "mov    %edi, 8(%eax)   \n" // jmp_buf[2] = edi
        "mov    (%ebp), %ecx    \n"
        "mov    %ecx, 12(%eax)  \n" // jmp_buf[3] = ebp
        "lea    8(%ebp), %ecx   \n" // get previous value of esp, before call
        "mov    %ecx, 16(%eax)  \n" // jmp_buf[4] = esp before call
        "mov    4(%ebp), %ecx   \n" // get saved caller eip from top of stack
        "mov    %ecx, 20(%eax)  \n" // jmp_buf[5] = saved eip
        "xor    %eax, %eax      \n" // eax = 0
    );
    return 0;
}

void longjmp(jmp_buf var,int m) {
    asm (
        "mov    8(%ebp),%edx    \n" // get pointer to jmp_buf, passed as argument 1 on stack
        "mov    12(%ebp),%eax   \n" // get int val in eax, passed as argument 2 on stack
        "test   %eax,%eax       \n" // is int val == 0?
        "jnz    1f              \n"
        "inc    %eax            \n" // if so, eax++
        "1:                     \n"
        "mov    (%edx),%ebx     \n" // ebx = jmp_buf[0]
        "mov    4(%edx),%esi    \n" // esi = jmp_buf[1]
        "mov    8(%edx),%edi    \n" // edi = jmp_buf[2]
        "mov    12(%edx),%ebp   \n" // ebp = jmp_buf[3]
        "mov    16(%edx),%ecx   \n" // ecx = jmp_buf[4]
        "mov    %ecx,%esp       \n" // esp = ecx
        "mov    20(%edx),%ecx   \n" // ecx = jmp_buf[5]
        "jmp     *%ecx          \n" // eip = ecx
    );
}
