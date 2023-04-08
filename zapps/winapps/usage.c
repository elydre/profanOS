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
    is_running = 1;
    window_delete(((button_t *) event->button)->window);
}

char *get_state(int state) {
    switch (state) {
        case 0: return "RUNG";
        case 1: return "WATG";
        case 2: return "TSLP";
        case 3: return "FSLP";
        case 4: return "ZMBI";
        case 5: return "DEAD";
        case 6: return "IDLE";
        default: return "UNKN";
    }
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

    int count, pid, runtime, loop_time, start, tmp, state, line;

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
        line = 0;
        for (int i = 0; i < 20; i++) {
            if (pid_runtime[i].pid != -1) {
                // get the state of the process
                state = c_process_get_state(pid_runtime[i].pid);
                if (state == 5) {
                    pid_runtime[i].pid = -1;
                    continue;
                }

                // get the name of the process
                c_process_get_name(pid_runtime[i].pid, buffer);
                // keep name from the last slash to the end
                char *ptr = strrchr(buffer, '/');
                if (ptr != NULL) {
                    strcpy(buffer, ptr + 1);
                }

                // add the pid and the usage
                tmp = strlen(buffer);
                buffer[tmp] = ' ';
                buffer[tmp + 1] = '(';
                itoa(pid_runtime[i].pid, buffer + tmp + 2, 10);
                tmp = strlen(buffer);
                buffer[tmp] = ')';
                buffer[tmp + 1] = 0;

                // align with the column 20
                tmp = strlen(buffer);
                for (int j = tmp; j < 20; j++) {
                    buffer[j] = ' ';
                }

                // add the state
                strcpy(buffer + 20, get_state(state));

                // align with the column 26
                tmp = strlen(buffer);
                for (int j = tmp; j < 26; j++) {
                    buffer[j] = ' ';
                }

                // add the cpu usage
                itoa(pid_runtime[i].usage, buffer + 26, 10);
                tmp = strlen(buffer);
                buffer[tmp] = '%';
                buffer[tmp + 1] = 0;

                // align with the column 31
                tmp = strlen(buffer);
                for (int j = tmp; j < 31; j++) {
                    buffer[j] = ' ';
                }

                // add the memory usage
                itoa(c_mem_get_info(8, pid_runtime[i].pid) / 1024, buffer + 31, 10);
                strcpy(buffer + strlen(buffer), "Ko");

                tmp = strlen(buffer);
                for (int j = 0; j < tmp; j++) {
                    local_print_char(window, buffer[j], j * 8 + 6, line * 16 + 5, 0x00bb00);
                }
                line++;
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
