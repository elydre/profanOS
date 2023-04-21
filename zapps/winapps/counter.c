#include <i_ocmlib.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char tmp[3];

    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    for (int i = 10; 1; i++) {
        if (i > 98) i = 10;

        itoa(i, tmp, 10);
        ocm_write(0, tmp[0]);
        ocm_write(0, tmp[1]);
        ocm_write(0, '\n');
        
        ms_sleep(50);
    }
}
