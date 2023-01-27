#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <type.h>

#define TO_JOIN_SIZE 0x1000

// TODO: file not hardcoded
#define TO_JOIN_DATA ".text/.data/.bss/.rodata"

char **to_join;
int to_join_count;

typedef struct {
    char name[32];
    uint32_t size;
    uint32_t offset;
} section_t;

section_t *sections;
int section_count;

uint8_t *data;
uint32_t data_size;

int gen_data(char *input_file) {
    // read the INPUT_FILE and generate the data
    if (!c_fs_does_path_exists(input_file)) {
        printf("[BINC ERROR] - input file does not exist\n");
        return 1;
    }

    data_size = c_fs_get_file_size(input_file);
    data = malloc(data_size + 1);
    c_fs_read_file(input_file, data);

    if (data[0] != 0x7f || data[1] != 'E' || data[2] != 'L' || data[3] != 'F') {
        printf("[BINC ERROR] - input file is not an ELF file\n");
        free(data);
        return 1;
    }

    return 0;
}

uint32_t bytes_to_uint(uint8_t *data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

uint32_t bytes_to_uint_reverse(uint8_t *data) {
    return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
}

void read_file_header(uint8_t *data) {
    // read header to find section header table

    uint32_t header_table_offset = bytes_to_uint_reverse(data + 32);
    printf("header table offset: %x\n", header_table_offset);

    uint8_t *header_table = data + header_table_offset;

    for (uint32_t i = 0; i < data_size - header_table_offset; i += 40) {
        uint8_t *header = header_table + i;
        uint32_t offset = bytes_to_uint_reverse(header + 16);
        uint32_t size = bytes_to_uint_reverse(header + 20);

        if (offset == 0 && size == 0) continue;

        section_count++;
        sections = realloc(sections, sizeof(section_t) * section_count);
        section_t *section = sections + section_count - 1;
        section->offset = offset;
        section->size = size;

        strcpy(section->name, "unknown");
    }
}

void set_names(uint32_t name_section_offset, uint32_t name_section_size) {
    uint8_t *name_section = data + name_section_offset + 1;
    int j;

    for (int i = 0; i < section_count; i++) {
        section_t *section = sections + i;

        for (j = 0; *name_section; j++) {
            section->name[j] = *name_section++;
        }
        section->name[j] = 0;
        name_section++;
    }
}

void patch_names() {
    char temp[32];
    char first[32];
    strcpy(first, sections[0].name);
    for (int i = 0; i < section_count - 1; i++) {
        strcpy(sections[i].name, sections[i + 1].name);
    }
    strcpy(sections[section_count - 1].name, first);
}

void print_section() {
    printf("\nsection count: %d\n", section_count);
    for (int i = 0; i < section_count; i++) {
        section_t *section = sections + i;
        printf("section %d: %s, offset: %x, size: %x\n",
                i,
                section->name,
                section->offset,
                section->size
        );
    }
    printf("\n");
}

void gen_to_join() {
    char *str_data_addr = malloc(strlen(TO_JOIN_DATA) + 1);
    char *str_data = str_data_addr;
    int j;

    strcpy(str_data, TO_JOIN_DATA);
    // count the number of sections to join
    to_join_count = 1;
    for (uint32_t i = 0; i < strlen(str_data); i++) {
        if (str_data[i] == '/') to_join_count++;
    }

    // allocate the array
    to_join = malloc(sizeof(char *) * to_join_count);

    // fill the array
    char *temp = malloc(strlen(str_data) + 1);
    for (int i = 0; i < to_join_count; i++) {
        for (j = 0; *str_data != '/' && *str_data != 0; j++) {
            temp[j] = *str_data++;
        }
        temp[j] = 0;
        to_join[i] = malloc(strlen(temp) + 1);
        strcpy(to_join[i], temp);
        str_data++;
    }

    free(temp);
    free(str_data_addr);
}

void binc_make_file(char *output_file) {
    // split the path into parent and file
    // then call c_fs_make_file(parent, file)

    char *parent = calloc(strlen(output_file) + 1, 1);
    char *file = calloc(strlen(output_file) + 1, 1);
    
    for (int i = strlen(output_file) - 1; i >= 0; i--) {
        if (output_file[i] == '/') {
            strcpy(parent, output_file);
            parent[i] = 0;
            strcpy(file, output_file + i + 1);
            break;
        }
    }

    printf("parent: %s, file: %s\n", parent, file);

    c_fs_make_file(parent, file);

    free(parent);
    free(file);
}

void generate_output(char *output_file) {
    uint8_t *output = malloc(TO_JOIN_SIZE * to_join_count + data_size);
    uint32_t output_size = 0;
    uint32_t to_add;

    printf("generating output...\n");

    for (int j = 0; j < section_count; j++) {
        section_t *section = sections + j;
        for (int i = 0; i < to_join_count; i++) {
            if (!strcmp(section->name, to_join[i])) {
                to_add = TO_JOIN_SIZE - output_size % TO_JOIN_SIZE;
                if (to_add != TO_JOIN_SIZE && to_add != 0) {
                    memset(output + output_size, 0, to_add);
                    output_size += to_add;
                }
                for (int k = 0; k < TO_JOIN_SIZE; k++) {
                    output[output_size + k] = data[section->offset + k];
                }
                output_size += section->size;
                printf("added %s (%d)\n", section->name, section->size);
            }
        }
    }
    if (!c_fs_does_path_exists(output_file)) {
        printf("creating output file...\n");
        binc_make_file(output_file);
    }
    printf("output size: %x\n", output_size);

    c_fs_write_in_file(output_file, output, output_size);

    free(output);
}

void generate_full_path(char *path, char *dir) {
    char *temp = malloc(strlen(dir) + 1);
    strcpy(temp, dir);
    strcpy(dir, path);
    if (dir[strlen(dir) - 1] != '/') strcat(dir, "/");
    strcat(dir, temp);
    free(temp);
}

int parse_args(int argc, char **argv, char *input_file, char *output_file) {
    if (argc < 3) {
        printf("usage: binc <file> [-o <output>]\n");
        return 1;
    }
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-o")) {
            if (i + 1 >= argc) {
                printf("[BINC ERROR] - missing output file after -o\n");
                return 1;
            }
            strcpy(output_file, argv[i + 1]);
            i++;
        } else {
            strcpy(input_file, argv[i]);
        }
    }
    if (!input_file[0]) {
        printf("[BINC ERROR] - missing input file\n");
        return 1;
    }

    if (!output_file[0]) {
        for (uint32_t i = 0; i < strlen(input_file); i++) {
            if (input_file[i] == '.') {
                output_file[i] = 0;
                break;
            }
            output_file[i] = input_file[i];
        }
        strcat(output_file, ".bin");
    }

    generate_full_path(argv[1], input_file);
    generate_full_path(argv[1], output_file);

    return 0;
}

void free_to_join() {
    for (int i = 0; i < to_join_count; i++) {
        free(to_join[i]);
    }

    free(to_join);
}

int main(int argc, char **argv) {

    char *input_file = calloc(128, sizeof(char));
    char *output_file = calloc(128, sizeof(char));

    if (parse_args(argc, argv, input_file, output_file)) {
        free(input_file);
        free(output_file);
        return 1;
    }

    printf("input file: %s\n", input_file);
    printf("output file: %s\n", output_file);

    gen_to_join();

    printf("to join (%d) [ ", to_join_count);
    for (int i = 0; i < to_join_count; i++) {
        printf("%s ", to_join[i], to_join[i]);
    }
    printf("]\n\n");

    if (gen_data(input_file)) {
        free(input_file);
        free(output_file);
        free_to_join();
        return 1;
    }
    printf("data size: %x\n", data_size);

    read_file_header(data);

    set_names(sections[section_count - 1].offset, sections[section_count - 1].size);

    while (strcmp(sections[section_count - 1].name, ".shstrtab")) {
        patch_names();
    }

    print_section();

    generate_output(output_file);

    free(data);
    free(sections);

    free(input_file);
    free(output_file);

    free_to_join();

    return 0;
}
