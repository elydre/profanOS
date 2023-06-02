#include <i_libdaube.h>
#include <stdio.h>

int main(int argc, char **argv) {
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
