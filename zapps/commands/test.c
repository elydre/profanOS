#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>

int main(void) {
    int fd[2];
    pipe(fd);

    int p1, p2;

    c_run_ifexist_full((runtime_args_t){
        "/bin/commands/echo.bin", NULL_SID, 3,
        (char *[]){"echo", "coucou", "oui"},
        0, 0, 0, 2
    }, &p1);

    c_run_ifexist_full((runtime_args_t){
        "/bin/commands/upper.bin", NULL_SID, 0,
        NULL, 0, 0, 0, 2
    }, &p2);

    dup2(fd[1], fm_resol012(1, p1));
    dup2(fd[0], fm_resol012(0, p2));

    close(fd[0]);
    close(fd[1]);

    fm_debug(fm_resol012(1, p1));
    fm_debug(fm_resol012(0, p2));

    c_process_wakeup(p1);
    c_process_wakeup(p2);

    profan_wait_pid(p2);
    return 0;
}
