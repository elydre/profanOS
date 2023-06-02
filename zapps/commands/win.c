#include <i_libdaube.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int list_win() {
    desktop_t *desktop = desktop_get_main();
    window_t *window;

    int win_count = desktop->nb_windows;

    printf("%d windows open on main desktop:\n", win_count);

    for (int i = 0; i < win_count; i++) {
        window = desktop->windows[i];

        printf("  %s (usid: %d) %dx%d [%s, %s, %s]\n",
            window->name,
            window->usid,
            window->in_width,
            window->in_height,

            window->is_lite ? "lite" : "full",
            window->cant_move ? "unmovable" : "movable",
            window->always_on_top ? "on top" : "floating"
        );
    }

    return 0;
}

int help() {
    puts("Usage: win [OPTION] [ARGUMENT]\n"
         "Options:\n"
         "  -h          Display this help message\n"
         "  -l          List all windows open\n"
         "  -c <ID>     Close a window\n"
         "  -t <ID t/f> on top/floating\n"
         "  -a <ID u/m> unmovable/movable\n"
         "  -s <ID>     Switch to a window\n"
    );

    return 0;
}

int usid_to_index(desktop_t *desktop, int usid) {
    window_t *window;

    int win_count = desktop->nb_windows;

    for (int i = 0; i < win_count; i++) {
        window = desktop->windows[i];

        if (window->usid == usid) return i;
    }

    return -1;
}

int close_window(int usid) {
    desktop_t *desktop = desktop_get_main();
    window_t *window;

    int index = usid_to_index(desktop, usid);

    if (index == -1) {
        printf("win: window with usid %d not found\n", usid);
        return 1;
    }

    window = desktop->windows[index];
    window_delete(window);

    return 0;
}

int floating_window(int usid, int on_top) {
    desktop_t *desktop = desktop_get_main();
    window_t *window;

    int index = usid_to_index(desktop, usid);

    if (index == -1) {
        printf("win: window with usid %d not found\n", usid);
        return 1;
    }

    printf("win: changing window %d to %s\n", usid, on_top ? "floating" : "on top");

    window = desktop->windows[index];
    window->always_on_top = on_top;
    window->changed = 1;

    desktop_refresh(desktop);

    return 0;
}

int movable_window(int usid, int movable) {
    desktop_t *desktop = desktop_get_main();
    window_t *window;

    int index = usid_to_index(desktop, usid);

    if (index == -1) {
        printf("win: window with usid %d not found\n", usid);
        return 1;
    }

    printf("win: changing window %d to %s\n", usid, movable ? "movable" : "unmovable");

    window = desktop->windows[index];
    window->cant_move = !movable;

    return 0;
}

int switch_window(int usid) {
    desktop_t *desktop = desktop_get_main();
    window_t *window;

    int index = usid_to_index(desktop, usid);

    if (index == -1) {
        printf("win: window with usid %d not found\n", usid);
        return 1;
    }

    printf("win: switching to window %d\n", usid);

    window = desktop->windows[index];
    window_auto_switch(desktop, window);
    desktop_refresh(desktop);

    return 0;
}

#define ARG_DEC 1

int main(int argc, char **argv) {
    argc -= ARG_DEC;

    if (argc == 1) {
        return list_win();
    }

    if (!strcmp(argv[1 + ARG_DEC], "-h")) {
        return help();
    }

    if (!strcmp(argv[1 + ARG_DEC], "-l")) {
        return list_win();
    }

    if (!strcmp(argv[1 + ARG_DEC], "-c")) {
        if (argc == 3) {
            return close_window(atoi(argv[2 + ARG_DEC]));
        } else puts("win: invalid arguments (win -c <ID>)\n");
    }

    if (!strcmp(argv[1 + ARG_DEC], "-t")) {
        if (argc == 4) {
            int on_top = !strcmp(argv[3 + ARG_DEC], "t");
            return floating_window(atoi(argv[2 + ARG_DEC]), on_top);
        } else puts("win: invalid arguments (win -t <ID> <t/f>)\n");
    }

    if (!strcmp(argv[1 + ARG_DEC], "-a")) {
        if (argc == 4) {
            int movable = !strcmp(argv[3 + ARG_DEC], "m");
            return movable_window(atoi(argv[2 + ARG_DEC]), movable);
        } else puts("win: invalid arguments (win -a <ID> <u/m>)\n");
    }

    if (!strcmp(argv[1 + ARG_DEC], "-s")) {
        if (argc == 3) {
            return switch_window(atoi(argv[2 + ARG_DEC]));
        } else puts("win: invalid arguments (win -s <ID>)\n");
    }

    puts("win: invalid arguments\n");
    help();
    return 1;
}
