#include <filesys.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int main(void) {
    sid_t file = fu_path_to_sid(ROOT_SID, "/dev/stdout");

    fu_fctf_write(file, "print in terminal\n", 0, 18);
    
    fu_file_create(0, "/tmp/test");
    devio_change_redirection(DEVIO_STDOUT, fu_path_to_sid(ROOT_SID, "/tmp/test"));

    fu_fctf_write(file, "print in file\n", 0, 14);

    return 0;
}
