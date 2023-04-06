#include <i_libdaube.h>
#include <i_winadds.h>
#include <i_time.h>

#include <syscall.h>
#include <stdlib.h>
#include <string.h>

int is_running;

typedef struct pid_runtime {
    int pid;
    int old_runtime;
    int new_runtime;
    int usage;
} pid_runtime_t;

void exit_callback(clickevent_t *event) {
    is_running = 0;
    window_delete(((button_t *) event->button)->window);
}

void local_print_char(window_t *window, char c, int x, int y, uint32_t color) {
    uint8_t *glyph = c_font_get(0) + c * 16;
    for (int j = 0; j < 16; j++) {
        for (int k = 0; k < 8; k++) {
            window_set_pixel(window, x + 8 - k, y + j, (glyph[j] & (1 << k)) ? color : 0);
        }
    }
}

int main(int argc, char **argv) {
    // wake up the parent process
    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    window_t *window = window_create(desktop_get_main(), "process usage", 100, 450, 350, 208, 0, 0);
    wadds_create_exitbt(window, exit_callback);
    desktop_refresh(desktop_get_main());

    // reset pixel buffer
    window_refresh(window);

    int *running_pid = calloc(20, sizeof(int));
    pid_runtime_t *pid_runtime = calloc(20, sizeof(pid_runtime_t));
    char *buffer = calloc(200, sizeof(char));

    for (int i = 0; i < 20; i++) {
        pid_runtime[i].pid = -1;
    }

    int count, pid, runtime, loop_time, start, tmp;

    loop_time = 1000;

    is_running = 1;
    while (is_running) {
        start = c_timer_get_ms();
        window_fill(window, 0x000000);

        // get the list of running processes
        count = c_process_generate_pid_list(running_pid, 20);

        for (int i = 0; i < count; i++) {
            pid = running_pid[i];
            runtime = c_process_get_run_time(pid);

            // check if the process is already in the list
            int found = 0;
            for (int j = 0; j < 20; j++) {
                if (pid_runtime[j].pid == pid) {
                    found = 1;
                    pid_runtime[j].old_runtime = pid_runtime[j].new_runtime;
                    pid_runtime[j].new_runtime = runtime;
                    pid_runtime[j].usage = (pid_runtime[j].new_runtime - pid_runtime[j].old_runtime) * 100 / loop_time;
                    break;
                }
            }

            // if not, add it
            if (!found) {
                for (int j = 0; j < 20; j++) {
                    if (pid_runtime[j].pid == -1) {
                        pid_runtime[j].pid = pid;
                        pid_runtime[j].new_runtime = runtime;
                        pid_runtime[j].usage = 0;
                        break;
                    }
                }
            }
        }

        // print the list
        for (int i = 0; i < 20; i++) {
            if (pid_runtime[i].pid != -1) {
                c_process_get_name(pid_runtime[i].pid, buffer);
                tmp = strlen(buffer);
                buffer[tmp] = ' ';
                buffer[tmp + 1] = '(';
                itoa(pid_runtime[i].pid, buffer + tmp + 2, 10);
                tmp = strlen(buffer);
                buffer[tmp] = ')';
                buffer[tmp + 1] = ' ';
                itoa(pid_runtime[i].usage, buffer + tmp + 2, 10);
                tmp = strlen(buffer);
                buffer[tmp] = '%';
                buffer[tmp + 1] = 0;
                tmp = strlen(buffer);
                for (int j = 0; j < tmp; j++) {
                    local_print_char(window, buffer[j], j * 8, i * 16, 0x00bb00);
                }
            }
        }

        window_refresh(window);
        ms_sleep(1000);
        loop_time = c_timer_get_ms() - start;
    }

    free(running_pid);
    free(pid_runtime);
    free(buffer);

    return 0;
}
