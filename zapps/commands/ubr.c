#include <filesys.h>
#include <syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: ubr <link> <redirection>\n");
        return 1;
    }

    char *pwd = getenv("PWD");

    char *link = malloc(strlen(pwd) + strlen(argv[1]) + 2);
    assemble_path(pwd, argv[1], link);

    char *redirection = malloc(strlen(pwd) + strlen(argv[2]) + 2);
    assemble_path(pwd, argv[2], redirection);

    sid_t link_sid = fu_path_to_sid(ROOT_SID, link);
    if (IS_NULL_SID(link_sid) || !fu_is_link(link_sid)) {
        printf("Failed to get link sid\n");
        free(redirection);
        free(link);
        return 1;
    }

    if (IS_NULL_SID(fu_path_to_sid(ROOT_SID, redirection))) {
        printf("Failed to get redirection sid\n");
        free(redirection);
        free(link);
        return 1;
    }

    if (devio_set_redirection(link_sid, redirection, c_process_get_ppid(c_process_get_pid()))) {
        printf("Failed to change redirection\n");
        free(redirection);
        free(link);
        return 1;
    }

    free(redirection);
    free(link);
    return 0;
}
