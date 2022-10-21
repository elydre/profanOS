#include "syscall.h"

#define LSIZE 1024
#define PSIZE 1024

/* https://github.com/elydre/ivra *
    end     0
    while   1 r
    set     2 r v
    cp      3 r r
    show    4 r
    add     5 r r o
    sub     6 r r o
    mul     7 r r o
    div     8 r r o
    egal    9 r r o
    sup     10 r r o
    or      11 r r o
    not     12 r
    mod     13 r r o
*/

int arg_cont[] = {
    0, 1, 2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 1, 3
};

int * mem;

void start_inter(int * code, int code_size, int while_id);
int lexer(char path[], int * code);

int main(int argc, char **argv) {
    int * meml = c_calloc(LSIZE * sizeof(int));
    int * prog = c_calloc(PSIZE * sizeof(int));
    mem = meml;

    meml[0] = 1;

    int code_size = lexer("/user/prog.li", prog);
    
    start_inter(prog, code_size, 0);

    c_free(meml);
    c_free(prog);
    return 0;
}

int lexer(char path[], int * code) {
    uint32_t *data_uint32 = c_fs_declare_read_array(path);
    char *data_char = c_fs_declare_read_array(path);
    c_fs_read_file(path, data_uint32);

    int i;
    for (i = 0; data_uint32[i] != (uint32_t) -1; i++) {
        data_char[i] =  data_uint32[i];
    } i++; data_char[i] = '\0';

    c_free(data_uint32);

    int code_size = 0;

    char temp[10];
    for (i = 0; data_char[i] != '\0'; i++) {
        if (!(data_char[i] >= '0' && data_char[i] <= '9')) continue;
        int j = 0;
        while (data_char[i] >= '0' && data_char[i] <= '9') {
            temp[j] = data_char[i];
            i++; j++;
        }
        temp[j] = '\0';
        code[code_size] = c_ascii_to_int(temp);
        code_size++;
    }

    c_free(data_char);
    return code_size;
}

void start_inter(int * code, int code_size, int while_id) {
    int to_pass, mode;
    while (mem[while_id]) {
        to_pass = 0;
        for (int instruc = 0; instruc < code_size; instruc++) {
            mode = code[instruc];

            if (mode == 0) {
                to_pass--;
                if (to_pass < 0) {
                    break;
                }
            }

            else if (to_pass) {
                if (mode == 1) {
                    to_pass++;
                }
            }

            else if (mode == 2) mem[code[instruc + 1]] = code[instruc + 2];
            else if (mode == 3) mem[code[instruc + 1]] = mem[code[instruc + 2]];
            else if (mode == 4) c_fskprint("%d\n", mem[code[instruc + 1]]);
            else if (mode == 5) mem[code[instruc + 3]] = mem[code[instruc + 1]] + mem[code[instruc + 2]];
            else if (mode == 6) mem[code[instruc + 3]] = mem[code[instruc + 1]] - mem[code[instruc + 2]];
            else if (mode == 7) mem[code[instruc + 3]] = mem[code[instruc + 1]] * mem[code[instruc + 2]];
            else if (mode == 8) mem[code[instruc + 3]] = mem[code[instruc + 1]] / mem[code[instruc + 2]];
            else if (mode == 9) mem[code[instruc + 3]] = mem[code[instruc + 1]] == mem[code[instruc + 2]];
            else if (mode == 10) mem[code[instruc + 3]] = mem[code[instruc + 1]] > mem[code[instruc + 2]];
            else if (mode == 11) mem[code[instruc + 3]] = mem[code[instruc + 1]] || mem[code[instruc + 2]];
            else if (mode == 12) mem[code[instruc + 1]] = !mem[code[instruc + 1]];
            else if (mode == 13) mem[code[instruc + 3]] = mem[code[instruc + 1]] % mem[code[instruc + 2]];

            instruc += arg_cont[mode];

            if (mode == 1 && to_pass == 0) {
                int * new_code = c_malloc((code_size - instruc - 1) * sizeof(int));
                for (int i = 0; i < code_size - instruc - 1; i++) {
                    new_code[i] = code[instruc + i + 1];
                }
                start_inter(new_code, code_size - instruc - 1, code[instruc]);
                c_free(new_code);
                to_pass++;
            }
        }
    }
}
