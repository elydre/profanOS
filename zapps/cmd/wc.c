#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define OPTION_L 1
#define OPTION_W 2
#define OPTION_C 3

void show_help(void) {
    puts("Usage: <CMD> | wc [option]\n"
        "Print newline, word, and byte counts from stdin.\n"
        "  -l    only print newline count\n"
        "  -w    only print word count\n"
        "  -c    only print byte count\n"
        "  -h    show this help message");
}

int parse_option(char *option) {
    if (option[0] != '-') {
        puts("Invalid option.");
        exit(1);
    }

    switch (option[1]) {
        case 'l':
            return OPTION_L;
        case 'w':
            return OPTION_W;
        case 'c':
            return OPTION_C;
        case 'h':
            show_help();
            exit(0);
        default:
            break;
    }
    printf("Invalid option: %s\nTry 'wc -h' "
        "for more information.\n", option);
    exit(1);
    return 0;
}

typedef struct {
    int lines;
    int words;
    int bytes;
} wc_t;

void stdin_count(wc_t *wc) {
    char c;
    int in_word = 0;

    while ((c = getchar()) != EOF) {
        wc->bytes++;
        if (c == '\n') {
            wc->lines++;
        }
        if (c == ' ' || c == '\n' || c == '\t') {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            wc->words++;
        }
    }
}

int main(int argc, char **argv) {
    int option;

    if (argc > 2) {
        puts("Usage: <CMD> | wc [option]\n"
            "Try 'wc -h' for more information.");
        return 1;
    }

    if (argc == 1) {
        option = 0;
    } else {
        option = parse_option(argv[1]);
    }

    if (isatty(STDIN_FILENO)) {
        puts("wc: stdin is a tty");
        return 1;
    }

    wc_t wc = {0, 0, 0};
    stdin_count(&wc);

    switch (option) {
        case OPTION_L:
            printf("%d\n", wc.lines);
            break;
        case OPTION_W:
            printf("%d\n", wc.words);
            break;
        case OPTION_C:
            printf("%d\n", wc.bytes);
            break;
        default:
            printf("lines: %d\nwords: %d\nbytes: %d\n",
                wc.lines, wc.words, wc.bytes);
            break;
    }

    return 0;
}
