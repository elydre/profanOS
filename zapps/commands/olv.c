#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STRING_CHAR '\''

#define ENABLE_DEBUG 0  // print function calls
#define SHOW_ALLFAIL 0  // show all failed checks
#define PROFANBUILD  1  // filesys usage

#define MAX_INPUT_SIZE 256
#define MAX_PATH_SIZE  256

#if PROFANBUILD
  #include <syscall.h>
  #include <i_time.h>
  #include <profan.h>

  // profanOS config
  #define OLV_PROMPT "olivine [$4%s$7] -> "
  #define PROFAN_COLOR "$6"
#else
  #define uint32_t unsigned int
  #define uint8_t unsigned char

  // unix config
  #define OLV_PROMPT "olivine [\033[1;34m%s\033[0m] -> "
  #define PROFAN_COLOR ""
#endif

#define MAX_VARIABLES 100
#define MAX_PSEUDOS   100
#define MAX_FUNCTIONS 100

#define OLV_VERSION "0.3"

typedef struct {
    char* name;
    char* (*function)(char**);
} internal_function_t;

typedef struct {
    char* name;
    char* value;
    int sync;
} variable_t;

typedef struct {
    char* name;
    char* value;
} pseudo_t;

typedef struct {
    char* name;
    char** lines;
    int line_count;
} function_t;

variable_t *variables;
pseudo_t *pseudos;
function_t *functions;
internal_function_t internal_functions[];

char *current_directory;

/*******************************
 *                            *
 * Variable Get/Set Functions *
 *                            *
********************************/

char *get_variable(char *name) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            return NULL;
        }
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return NULL;
}

int set_variable(char *name, char *value) {
    char *value_copy = malloc(strlen(value) + 1);
    strcpy(value_copy, value);

    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            char *name_copy = malloc(strlen(name) + 1);
            strcpy(name_copy, name);
            variables[i].name = name_copy;
            variables[i].value = value_copy;
            variables[i].sync = 0;
            return 0;
        }
        if (strcmp(variables[i].name, name) == 0) {
            if (variables[i].value && (!variables[i].sync)) {
                free(variables[i].value);
            }
            variables[i].value = value_copy;
            variables[i].sync = 0;
            return 0;
        }
    }
    printf("Error: Too many variables\n");
    return 1;
}

int set_sync_variable(char *name, char *value) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            variables[i].name = name;
            variables[i].value = value;
            variables[i].sync = 1;
            return 0;
        }
        if (strcmp(variables[i].name, name) == 0) {
            if (variables[i].value && (!variables[i].sync)) {
                free(variables[i].value);
            }
            variables[i].value = value;
            variables[i].sync = 1;
            return 0;
        }
    }
    return 1;
}

int does_variable_exist(char *name) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            return 0;
        }
        if (strcmp(variables[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

int del_variable(char *name) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            return 1;
        }
        if (strcmp(variables[i].name, name) == 0) {
            if (variables[i].value && (!variables[i].sync)) {
                free(variables[i].value);
                free(variables[i].name);
            }
            variables[i].name = NULL;
            variables[i].value = NULL;
            variables[i].sync = 0;

            // shift all variables down
            for (int j = i; j < MAX_VARIABLES - 1; j++) {
                variables[j] = variables[j + 1];
            }
            return 0;
        }
    }
    return 1;
}

/*****************************
 *                          *
 * pseudo Get/Set Functions  *
 *                          *
*****************************/

char *get_pseudo(char *name) {
    for (int i = 0; i < MAX_PSEUDOS; i++) {
        if (pseudos[i].name == NULL) {
            return NULL;
        }
        if (strcmp(pseudos[i].name, name) == 0) {
            return pseudos[i].value;
        }
    }
    return NULL;
}

int set_pseudo(char *name, char *value) {
    char *value_copy = malloc(strlen(value) + 1);
    strcpy(value_copy, value);

    for (int i = 0; i < MAX_PSEUDOS; i++) {
        if (pseudos[i].name == NULL) {
            char *name_copy = malloc(strlen(name) + 1);
            strcpy(name_copy, name);
            pseudos[i].name = name_copy;
            pseudos[i].value = value_copy;
            return 0;
        }
        if (strcmp(pseudos[i].name, name) == 0) {
            if (pseudos[i].value) {
                free(pseudos[i].value);
            }
            pseudos[i].value = value_copy;
            return 0;
        }
    }
    printf("Error: Too many pseudos\n");
    return 1;
}

/*******************************
 *                            *
 * Function Get/Set Functions *
 *                            *
********************************/

int set_function(char *name, char **lines, int line_count) {
    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name == NULL) {
            char *name_copy = malloc(strlen(name) + 1);
            strcpy(name_copy, name);
            functions[i].name = name_copy;
            functions[i].line_count = line_count;

            // we need to copy the lines
            functions[i].lines = malloc(sizeof(char *) * line_count);
            for (int j = 0; j < line_count; j++) {
                functions[i].lines[j] = malloc(strlen(lines[j]) + 1);
                strcpy(functions[i].lines[j], lines[j]);
            }

            return 0;
        } if (strcmp(functions[i].name, name) == 0) {
            printf("Function %s already exists\n", name);
            return 1;
        }
    }
    printf("Too many functions\n");
    return 1;
}

void print_function(char *name) {
    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name == NULL) {
            printf("Function %s does not exist\n", name);
            return;
        }
        if (strcmp(functions[i].name, name) == 0) {
            printf("Function %s:\n", name);
            for (int j = 0; j < functions[i].line_count; j++) {
                printf("| %s\n", functions[i].lines[j]);
            }
            return;
        }
    }
}

function_t *get_function(char *name) {
    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name == NULL) {
            return NULL;
        }
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    return NULL;
}

/***********************************
 *                                *
 *  Olivine Integrated Evaluator  *
 *                                *
***********************************/

typedef struct {
    void *ptr;
    uint8_t type;
} ast_leaf_t;

typedef struct {
    ast_leaf_t left;
    ast_leaf_t center;
    ast_leaf_t right;
} ast_t;

#define AST_TYPE_AST   0
#define AST_TYPE_NIL   1
#define AST_TYPE_STR   3

char ops[] = "=<>+-*/~^&|~()";

void printsplit(char **split) {
    for (int i = 0; split[i] != NULL; i++) {
        printf(i == 0 ?
                "[\"%s\", " :
                split[i + 1] ?
                    "\"%s\", " :
                    "\"%s\"]\n"
            , split[i]
        );
    }
}

ast_t *gen_ast(char **str, int len) {
    ast_t *ast = malloc(sizeof(ast_t));
    ast->left.type = AST_TYPE_NIL;
    ast->right.type = AST_TYPE_NIL;
    ast->center.type = AST_TYPE_NIL;

    // if start with parenthesis and end with parenthesis remove them
    if (len > 2 && str[0][0] == '(') {
        int count = 1;
        int good = 1;
        for (int i = 1; i < len; i++) {
            if (str[i][0] == '(') {
                count++;
            } else if (str[i][0] == ')') {
                count--;
            }
            if (count == 0 && i != len - 1) {
                good = 0;
            }
        } if (good) {
            str++;
            len -= 2;
        }
    }

    // check if only one element
    if (len == 1) {
        ast->center.type = AST_TYPE_STR;
        ast->center.ptr = str[0];
        return ast;
    }

    // check if only two elements
    if (len == 2) {
        ast->left.type = AST_TYPE_STR;
        ast->left.ptr = str[0];
        ast->right.type = AST_TYPE_STR;
        ast->right.ptr = str[1];
        return ast;
    }

    // check if only one operator
    if (len == 3) {
        ast->left.type = AST_TYPE_STR;
        ast->left.ptr = str[0];
        ast->center.type = AST_TYPE_STR;
        ast->center.ptr = str[1];
        ast->right.type = AST_TYPE_STR;
        ast->right.ptr = str[2];
        return ast;
    }

    // divide and rule

    // find operator with lowest priority
    int op_index = -1;
    int op_priority = 999;
    int op_parenthesis = 0;
    for (int i = 0; i < len; i++) {
        if (str[i][0] == '(') {
            op_parenthesis++;
        } else if (str[i][0] == ')') {
            op_parenthesis--;
        } else if (op_parenthesis == 0) {
            for (int j = 0; j < (int) sizeof(ops); j++) {
                if (str[i][0] == ops[j] && j < op_priority) {
                    op_index = i;
                    op_priority = j;
                    break;
                }
            }
        }
    }

    // check if no operator
    if (op_index == -1) {
        printf("input: ");
        printsplit(str);
        printf("no operator found\n");
        exit(1);
    }

    // generate ast
    if (op_index == 0) {
        ast->left.type = AST_TYPE_STR;
        ast->left.ptr = str[0];
    } else {
        ast->left.type = AST_TYPE_AST;
        ast->left.ptr = gen_ast(str, op_index);
    }

    ast->center.type = AST_TYPE_STR;
    ast->center.ptr = str[op_index];

    if (len - op_index - 1 == 1) {
        ast->right.type = AST_TYPE_STR;
        ast->right.ptr = str[op_index + 1];
    } else {
        ast->right.type = AST_TYPE_AST;
        ast->right.ptr = gen_ast(str + op_index + 1, len - op_index - 1);
    }

    return ast;
}

char *eval(ast_t *ast) {
    // if only one element return it
    if (ast->left.type == AST_TYPE_NIL && ast->right.type == AST_TYPE_NIL) {
        char *res = malloc(strlen((char *) ast->center.ptr) + 1);
        strcpy(res, (char *) ast->center.ptr);
        return res;
    }

    if (ast->center.type == AST_TYPE_NIL) {
        printf("operator not supported\n");
        return NULL;
    }

    // convert to int
    char *op = (char *) ast->center.ptr;
    int left, right;
    char *res = NULL;

    if (ast->left.type == AST_TYPE_AST) {
        res = eval((ast_t *) ast->left.ptr);
        if (res == NULL) return NULL;
        left = atoi(res);
        free(res);
    } else {
        left = atoi((char *) ast->left.ptr);
    }

    if (ast->right.type == AST_TYPE_AST) {
        res = eval((ast_t *) ast->right.ptr);
        if (res == NULL) return NULL;
        right = atoi(res);
        free(res);
    } else {
        right = atoi((char *) ast->right.ptr);
    }

    // calculate
    int result;
    switch (op[0]) {
        case '+':
            result = left + right;
            break;
        case '-':
            result = left - right;
            break;
        case '*':
            result = left * right;
            break;
        case '/':
            result = left / right;
            break;
        case '~':
            result = left % right;
            break;
        case '<':
            result = left < right;
            break;
        case '>':
            result = left > right;
            break;
        case '=':
            result = left == right;
            break;
        default:
            printf("unknown operator: %s\n", op);
            return NULL;
    }

    // printf("%d %s %d -> %d\n", left, op, right, result);

    // convert back to string
    char *ret = malloc(sizeof(char) * 12);
    sprintf(ret, "%d", result);
    return ret;
}

void free_ast(ast_t *ast) {
    if (ast->left.type == AST_TYPE_AST) {
        free_ast((ast_t *) ast->left.ptr);
    }

    if (ast->right.type == AST_TYPE_AST) {
        free_ast((ast_t *) ast->right.ptr);
    }

    free(ast);
}

char *if_eval(char **input) {
    // join input
    int required_size = 1;
    for (int i = 0; input[i] != NULL; i++) {
        required_size += strlen(input[i]);
    }

    char *joined_input = malloc(required_size * sizeof(char));
    joined_input[0] = '\0';
    for (int i = 0; input[i] != NULL; i++) {
        strcat(joined_input, input[i]);
    }

    char **elms = malloc(sizeof(char *) * (strlen(joined_input) + 1));
    int len = 0;
    int str_len = strlen(joined_input);
    int old_cut = 0;

    for (int i = 0; i < str_len; i++) {
        // check if operator
        for (uint32_t j = 0; j < sizeof(ops); j++) {
            if (joined_input[i] != ops[j]) continue;

            if (old_cut != i) {
                elms[len] = malloc(sizeof(char) * (i - old_cut + 1));
                memcpy(elms[len], joined_input + old_cut, i - old_cut);
                elms[len][i - old_cut] = '\0';
                len++;
            }

            elms[len] = malloc(sizeof(char) * 2);
            elms[len][0] = joined_input[i];
            elms[len][1] = '\0';
            len++;

            old_cut = i + 1;
            break;
        }
    }

    if (old_cut != str_len) {
        elms[len] = malloc(sizeof(char) * (str_len - old_cut + 1));
        memcpy(elms[len], joined_input + old_cut, str_len - old_cut);
        elms[len][str_len - old_cut] = '\0';
        len++;
    }

    elms[len] = NULL;

    free(joined_input);

    ast_t *ast = gen_ast(elms, len);
    char *res = eval(ast);

    free_ast(ast);

    for (int i = 0; i < len; i++) {
        free(elms[i]);
    }

    free(elms);

    return res;
}


/**************************************
 *                                   *
 *  Olivine Lang Internal Functions  *
 *                                   *
**************************************/


char *if_echo(char **input) {
    for (int i = 0; input[i] != NULL; i++) {
        printf("%s ", input[i]);
    }
    printf("\n");
    return NULL;
}

char *if_upper(char **input) {
    int required_size = 0;
    for (int i = 0; input[i] != NULL; i++) {
        required_size += strlen(input[i]) + 3;
    }

    char *result = malloc(required_size * sizeof(char));

    int result_i = 0;
    for (int i = 0; input[i] != NULL; i++) {
        result[result_i++] = STRING_CHAR;
        for (int j = 0; input[i][j] != '\0'; j++) {
            if (input[i][j] >= 'a' && input[i][j] <= 'z') {
                result[result_i] = input[i][j] - 32;
            } else {
                result[result_i] = input[i][j];
            }
            result_i++;
        }
        result[result_i++] = STRING_CHAR;
        result[result_i++] = ' ';
    }

    result[required_size - 1] = '\0';

    return result;
}

char *if_join(char **input) {
    /*
     * input: ["hello", "world"]
     * output: "'hello world'"
    */

    int required_size = 2;
    for (int i = 0; input[i] != NULL; i++) {
        required_size += strlen(input[i]) + 1;
    }

    char *result = malloc(required_size * sizeof(char));

    int result_i = 0;
    result[result_i++] = STRING_CHAR;

    for (int i = 0; input[i] != NULL; i++) {
        for (int j = 0; input[i][j] != '\0'; j++) {
            result[result_i++] = input[i][j];
        }
        result[result_i++] = ' ';
    }

    result[required_size - 2] = STRING_CHAR;
    result[required_size - 1] = '\0';

    return result;
}

char *if_split(char **input) {
    /*
     * input: ["hello world", "test"]
     * output: "'hello' 'world' 'test'"
    */

    int required_size = 0;
    for (int i = 0; input[i] != NULL; i++) {
        for (int j = 0; input[i][j] != '\0'; j++) {
            if (input[i][j] == ' ') {
                required_size += 2;
            }
            required_size++;
        }
        required_size += 3;
    }

    char *result = malloc(required_size * sizeof(char));

    int result_i = 0;

    for (int i = 0; input[i] != NULL; i++) {
        result[result_i++] = STRING_CHAR;
        for (int j = 0; input[i][j] != '\0'; j++) {
            if (input[i][j] == ' ') {
                result[result_i++] = STRING_CHAR;
                result[result_i++] = ' ';
                result[result_i++] = STRING_CHAR;
            } else {
                result[result_i++] = input[i][j];
            }
        }
        result[result_i++] = STRING_CHAR;
        result[result_i++] = ' ';
    }

    result[required_size - 1] = '\0';

    return result;
}

char *if_set_var(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2) {
        printf("VAR: expected 2 arguments, got %d\n", argc);
        return NULL;
    }

    // get name
    char *name = input[0];

    // get value
    char *value = input[1];

    // set variable
    if (set_variable(name, value)) {
        printf("VAR: no more space for variables\n");
    }

    return NULL;
}

char *if_show(char **input) {
    for (int i = 0; input[i] != NULL; i++) {
        printf("arg[%d]: '%s'\n", i, input[i]);
    }
    return NULL;
}

char *if_debug(char **input) {
    // print variables
    printf("VARIABLES\n");
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name != NULL) {
            printf("  %s: '%s'\n", variables[i].name, variables[i].value);
        }
    }

    // print functions
    printf("FUNCTIONS\n");
    for (int i = 0; internal_functions[i].name != NULL; i++) {
        printf("  %s: %p\n", internal_functions[i].name, internal_functions[i].function);
    }

    // print pseudos
    printf("PSEUDOS\n");
    for (int i = 0; pseudos[i].name != NULL; i++) {
        printf("  %s: '%s'\n", pseudos[i].name, pseudos[i].value);
    }

    return NULL;
}


char *if_go_binfile(char **input) {
    #if PROFANBUILD

    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc < 1) {
        printf("GO: expected at least 1 argument, got %d\n", argc);
        return NULL;
    }

    // get file name
    char *file_name = malloc((strlen(input[0]) + strlen(current_directory) + 2) * sizeof(char));
    assemble_path(current_directory, input[0], file_name);
    // check if file exists

    if (!(c_fs_does_path_exists(file_name) && c_fs_get_sector_type(c_fs_path_to_id(file_name)) == 2)) {
        printf("GO: file '%s' does not exist\n", file_name);
        free(file_name);
        return NULL;
    }

    // rebuild the arguments whis profan format:
    // 1. full path to the binary
    // 2. current working directory
    // 3. all the arguments

    argc++;

    char **argv = malloc((argc + 1) * sizeof(char*));
    argv[0] = file_name;
    argv[1] = current_directory;
    for (int i = 1; input[i] != NULL; i++) {
        argv[i + 1] = input[i];
    }
    argv[argc] = NULL;

    c_run_ifexist(file_name, argc, argv);

    free(file_name);
    free(argv);

    #else
    printf("GO: not available in this build\n");
    #endif

    return NULL;
}

char *if_change_dir(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        printf("CD: expected 1 argument, got %d\n", argc);
        return NULL;
    }

    // get dir
    char *dir = malloc((strlen(input[0]) + strlen(current_directory) + 2) * sizeof(char));

    // check if dir exists
    #if PROFANBUILD
    assemble_path(current_directory, input[0], dir);

    if (!(c_fs_does_path_exists(dir) && c_fs_get_sector_type(c_fs_path_to_id(dir)) == 3)) {
        printf("CD: directory '%s' does not exist\n", dir);
        return NULL;
    }
    #else
    strcpy(dir, input[0]);
    #endif

    // change directory
    strcpy(current_directory, dir);

    free(dir);

    return NULL;
}

char *if_make_pseudo(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2) {
        printf("PSEUDO: expected 2 arguments, got %d\n", argc);
        return NULL;
    }

    // get name
    char *name = input[0];

    // get value
    char *value = input[1];

    // set pseudo
    if (set_pseudo(name, value)) {
        printf("PSEUDO: no more space for pseudo\n");
    }

    return NULL;
}

char *if_range(char **input) {
    /*
     * input: ["1", "5"]
     * output: "'1' '2' '3' '4' '5'"
    */

    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2) {
        printf("RANGE: expected 2 arguments, got %d\n", argc);
        return NULL;
    }

    int start = atoi(input[0]);
    int end = atoi(input[1]);

    if (start >= end) {
        printf("RANGE: start must be less than end\n");
        return NULL;
    }

    char *output = malloc(1 * sizeof(char));
    output[0] = '\0';
    for (int i = start; i < end; i++) {
        char *tmp = malloc((strlen(output) + strlen(input[1]) + 4) * sizeof(char));
        strcpy(tmp, output);
        sprintf(tmp, "%s '%d'", tmp, i);
        free(output);
        output = tmp;
    }
    char *copy = malloc((strlen(output)) * sizeof(char));
    strcpy(copy, output + 1);
    free(output);

    return copy;
}

char *if_find(char **input) {
    /*
     * input: ["-f", "/dir/subdir"]
     * output: "'/dir/subdir/file1' '/dir/subdir/file2'"
    */

    #if PROFANBUILD
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2) {
        printf("FIND: expected 2 arguments, got %d\n", argc);
        return NULL;
    }

    int required_type = 0;
    if (strcmp(input[0], "-f") == 0) {
        required_type = 2;
    } else if (strcmp(input[0], "-d") == 0) {
        required_type = 3;
    } else {
        printf("FIND: expected -f or -d as first argument, got '%s'\n", input[0]);
        return NULL;
    }

    char *path = malloc((strlen(input[1]) + strlen(current_directory) + 2) * sizeof(char));
    assemble_path(current_directory, input[1], path);

    int elm_count = c_fs_get_dir_size(path);

    uint32_t *out_ids = malloc(elm_count * sizeof(uint32_t));
    int *out_types = malloc(elm_count * sizeof(int));
    c_fs_get_dir_content(path, out_ids);

    for (int i = 0; i < elm_count; i++)
        out_types[i] = c_fs_get_sector_type(out_ids[i]);

    char *output = malloc(1 * sizeof(char));
    output[0] = '\0';

    char *tmp_name = malloc(MAX_PATH_SIZE * sizeof(char));
    char *tmp_path = malloc(MAX_PATH_SIZE * sizeof(char));

    for (int i = 0; i < elm_count; i++) {
        if (out_types[i] == required_type) {
            c_fs_get_element_name(out_ids[i], tmp_name);
            assemble_path(path, tmp_name, tmp_path);

            char *tmp = malloc((strlen(output) + strlen(tmp_path) + 4) * sizeof(char));
            strcpy(tmp, output);
            sprintf(tmp, "%s '%s'", tmp, tmp_path);
            free(output);
            output = tmp;
        }
    }

    char *copy = malloc((strlen(output)) * sizeof(char));
    strcpy(copy, output + 1);
    free(output);

    free(tmp_name);
    free(tmp_path);
    free(out_ids);
    free(out_types);
    free(path);

    return copy;
    #else
    printf("FIND: not supported in this build\n");
    return NULL;
    #endif
}

char *if_name(char **input) {
    /*
     * input: ["/dir/subdir/file1.txt"]
     * output: "'file1'"
    */

    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        printf("NAME: expected 1 argument, got %d\n", argc);
        return NULL;
    }

    int len = strlen(input[0]);
    char *name = malloc((len + 1) * sizeof(char));

    for (int i = len - 1; i >= 0; i--) {
        if (input[0][i] == '/') {
            strcpy(name, input[0] + i + 1);
            break;
        }
    }

    // remove extension
    for (int i = strlen(name) - 1; i >= 0; i--) {
        if (name[i] == '.') {
            name[i] = '\0';
            break;
        }
    }

    char *output = malloc((strlen(name) + 3) * sizeof(char));
    sprintf(output, "'%s'", name);
    free(name);

    return output;
}

internal_function_t internal_functions[] = {
    {"echo", if_echo},
    {"upper", if_upper},
    {"join", if_join},
    {"split", if_split},
    {"set", if_set_var},
    {"show", if_show},
    {"debug", if_debug},
    {"eval", if_eval},
    {"go", if_go_binfile},
    {"cd", if_change_dir},
    {"pseudo", if_make_pseudo},
    {"range", if_range},
    {"find", if_find},
    {"name", if_name},
    {NULL, NULL}
};

void *get_if_function(char *name) {
    for (int i = 0; internal_functions[i].name != NULL; i++) {
        if (strcmp(internal_functions[i].name, name) == 0) {
            return internal_functions[i].function;
        }
    }
    return NULL;
}

/************************************
 *                                 *
 *  String Manipulation Functions  *
 *                                 *
************************************/

void remove_quotes(char *string) {
    int i, dec = 0;
    for (i = 0; string[i] != '\0'; i++) {
        if (string[i] == STRING_CHAR) {
            dec++;
            continue;
        }
        string[i - dec] = string[i];
    }
    string[i - dec] = '\0';
}

int does_startwith(char *str, char *start) {
    uint32_t len = strlen(start);
    if (strlen(str) < len) {
        return 0;
    }

    for (uint32_t i = 0; i < len; i++) {
        if (str[i] != start[i]) {
            return 0;
        }
    }
    return 1;
}

char **gen_args(char *string) {
    if (string == NULL || strlen(string) == 0) {
        char **argv = malloc(1 * sizeof(char*));
        argv[0] = NULL;
        return argv;
    }

    // count the number of arguments
    int in_string = 0;
    int argc = 1;

    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == STRING_CHAR) {
            in_string = !in_string;
        } if (string[i] == ' ' && !in_string) {
            argc++;
        }
    }

    // allocate the arguments array
    char **argv = malloc((argc + 1) * sizeof(char*));

    // fill the arguments array
    int old_i = 0;
    int arg_i = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == STRING_CHAR) {
            in_string = !in_string;
        } if (string[i] == ' ' && !in_string) {
            argv[arg_i] = malloc((i - old_i + 1) * sizeof(char));
            strncpy(argv[arg_i], string + old_i, i - old_i);
            argv[arg_i][i - old_i] = '\0';

            remove_quotes(argv[arg_i]);
            old_i = i + 1;
            arg_i++;
        }
    }

    // add the last argument
    argv[argc - 1] = malloc((strlen(string) - old_i + 1) * sizeof(char));
    strcpy(argv[argc - 1], string + old_i);
    remove_quotes(argv[argc - 1]);

    argv[argc] = NULL;

    return argv;
}

char *get_if_function_name(char *string, int *size) {
    int in_string = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == STRING_CHAR) {
            in_string = !in_string;
        }

        if (string[i] == ' ' && !in_string) {
            char *function_name = malloc((i + 1) * sizeof(char));
            strncpy(function_name, string, i);
            function_name[i] = '\0';

            *size = i + 1; // also include the space
            remove_quotes(function_name);
            return function_name;
        }
    }

    char *function_name = malloc((strlen(string) + 1) * sizeof(char));
    strcpy(function_name, string);
    *size = strlen(string);

    remove_quotes(function_name);
    return function_name;
}

void free_args(char **argv) {
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

void free_vars() {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name != NULL && (!variables[i].sync)) {
            free(variables[i].value);
            free(variables[i].name);
        }
    }
    free(variables);
}

void free_pseudos() {
    for (int i = 0; i < MAX_PSEUDOS; i++) {
        if (pseudos[i].name != NULL) {
            free(pseudos[i].name);
            free(pseudos[i].value);
        }
    }
    free(pseudos);
}

void free_functions() {
    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name != NULL) {
            free(functions[i].name);
            for (int l = 0; l < functions[i].line_count; l++) {
                free(functions[i].lines[l]);
            }
            free(functions[i].lines);
        }
    }
    free(functions);
}

/**************************
 *                       *
 *  Execution Functions  *
 *                       *
**************************/

void debug_print(char *function_name, char **function_args) {
    printf(PROFAN_COLOR "'%s'(", function_name);

    for (int i = 0; function_args[i] != NULL; i++) {
        if (function_args[i + 1] != NULL) {
            printf(PROFAN_COLOR "'%s', ", function_args[i]);
            continue;
        }
        printf(PROFAN_COLOR "'%s') [%d]\n", function_args[i], i + 1);
        return;
    }
    printf(PROFAN_COLOR ") [0]\n");
}

int execute_lines(char **lines, int line_end, char *result);

char *execute_function(function_t *function, char **args) {
    // set variables:
    // !0: first argument
    // !1: second argument
    // !2: third argument
    // !#: argument count

    int argc = 0;
    char tmp[4];
    for (argc = 0; args[argc] != NULL; argc++) {
        sprintf(tmp, "%d", argc);
        set_variable(tmp, args[argc]);
    }
    sprintf(tmp, "%d", argc);
    set_variable("#", tmp);

    char *result = malloc(sizeof(char));
    int ret = execute_lines(function->lines, function->line_count, result);

    // free variables
    for (int i = 0; i < argc; i++) {
        sprintf(tmp, "%d", i);
        del_variable(tmp);
    }
    del_variable("#");

    if (ret < 0) {
        if (SHOW_ALLFAIL)
            printf("Error: function failed\n");

        free(result);
        return NULL;
    }

    return result;
}

char *check_subfunc(char *line);
char *check_variables(char *line);
char *check_pseudos(char *line);


char *execute_line(char *full_line) {
    // check for function and variable
    char *line = check_subfunc(full_line);

    if (line == NULL) {
        if (SHOW_ALLFAIL)
            printf("Error: subfunction failed\n");

        return NULL;
    }

    // get the function name
    int name_size, isif;
    char *function_name = get_if_function_name(line, &name_size);

    // get the function address
    void *function = get_if_function(function_name);
    if (function == NULL) {
        function = get_function(function_name);
        isif = 0;
    } else isif = 1;

    char *result;

    if (function == NULL) {
        printf("Function '%s' not found\n", function_name);
        result = NULL;
    } else {
        // generate the arguments array
        char **function_args = gen_args(line + name_size);

        if (ENABLE_DEBUG)
            debug_print(function_name, function_args);

        // execute the function
        if (isif) {
            result = ((char* (*)(char**))function)(function_args);
        } else {
            result = execute_function(function, function_args);
        }

        if (result == NULL) {
            result = malloc(1 * sizeof(char));
            result[0] = '\0';
        }

        free_args(function_args);
    }

    free(function_name);

    if (line != full_line) {
        free(line);
    }

    return result;
}

char *check_subfunc(char *line) {
    // check if !( ... ) is present
    int start = -1;
    int end = -1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '!' && line[i + 1] == '(') {
            start = i;
            break;
        }
    }

    if (start == -1) {
        char *var_line = check_variables(line);

        if (var_line == NULL) {
            return NULL;
        }

        char *pseudo_line = check_pseudos(var_line);
        if (pseudo_line != var_line) {
            free(var_line);
        }

        return pseudo_line;
    }

    int open_parentheses = 1;
    for (int i = start + 2; line[i] != '\0'; i++) {
        if (line[i] == '(') {
            open_parentheses++;
        } else if (line[i] == ')') {
            open_parentheses--;
        }

        if (open_parentheses == 0) {
            end = i;
            break;
        }
    }

    if (end == -1) {
        printf("Error: missing closing parenthesis '%s'\n", line);
        return NULL;
    }

    char *subfunc = malloc((end - start - 1) * sizeof(char));
    strncpy(subfunc, line + start + 2, end - start - 2);
    subfunc[end - start - 2] = '\0';

    // execute the subfunc
    char *subfunc_result = execute_line(subfunc);
    free(subfunc);

    if (subfunc_result == NULL) {
        if (SHOW_ALLFAIL)
            printf("Error: subfunc returned NULL\n");

        return NULL;
    }

    // replace the subfunc with its result
    char *new_line = malloc((strlen(line) - (end - start) + strlen(subfunc_result) + 1) * sizeof(char));
    strncpy(new_line, line, start);
    new_line[start] = '\0';

    strcat(new_line, subfunc_result);
    strcat(new_line, line + end + 1);

    free(subfunc_result);

    char *rec = check_subfunc(new_line);

    if (rec != new_line) {
        free(new_line);
    }

    char *var_line = check_variables(rec);
    if (var_line != rec) {
        free(rec);
    }

    char *pseudo_line = check_pseudos(var_line);
    if (pseudo_line != var_line) {
        free(var_line);
    }

    return var_line;
}

char *check_variables(char *line) {
    // check if !... is present
    int start = -1;
    int end = -1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '!' && line[i + 1] != '!') {
            start = i;
            break;
        }
    }

    if (start == -1) {
        return line;
    }

    int i;
    for (i = start + 1; line[i] != '\0'; i++) {
        if (line[i] == ' ' || line[i] == '!' || line[i] == STRING_CHAR) {
            end = i;
            break;
        }
    }

    if (end == -1) end = i;

    char *var_name = malloc((end - start) * sizeof(char));
    strncpy(var_name, line + start + 1, end - start - 1);
    var_name[end - start - 1] = '\0';

    // get the variable value
    char *var_value = get_variable(var_name);

    if (var_value == NULL) {
        printf("Error: variable '%s' not found\n", var_name);
        free(var_name);
        return NULL;
    }

    free(var_name);

    // replace the variable with its value
    char *new_line = malloc((strlen(line) - (end - start) + strlen(var_value) + 1) * sizeof(char));
    strncpy(new_line, line, start);
    new_line[start] = '\0';

    strcat(new_line, var_value);
    strcat(new_line, line + end);

    char *rec = check_variables(new_line);

    if (rec == NULL) {
        free(new_line);
        return NULL;
    }

    if (rec != new_line) {
        free(new_line);
    }

    return rec;
}

char *check_pseudos(char *line) {
    char *pseudo_name = malloc((strlen(line) + 1) * sizeof(char));
    int len = strlen(line);
    int i;
    for (i = 0; i < len; i++) {
        if (line[i] == ' ') {
            break;
        }
        pseudo_name[i] = line[i];
    }
    pseudo_name[i] = '\0';

    char *pseudo_value = get_pseudo(pseudo_name);

    free(pseudo_name);
    if (pseudo_value == NULL) {
        return line;
    }

    char *new_line = malloc((strlen(line) - i + strlen(pseudo_value) + 2) * sizeof(char));
    strcpy(new_line, pseudo_value);
    strcat(new_line, line + i);

    return new_line;
}

/***********************
 *                    *
 *  Lexing Functions  *
 *                    *
***********************/

char **lexe_program(char *program) {
    int line_count = 1;
    for (int i = 0; program[i] != '\0'; i++) {
        if (program[i] == '\n' || program[i] == ';') {
            line_count++;
        }
    }

    char **lines = malloc((line_count + 1) * sizeof(char*));
    char *tmp = malloc((strlen(program) + 1) * sizeof(char));

    int tmp_index = 0;
    int line_index = 0;
    int is_string_begin = 1;
    for (int i = 0; program[i] != '\0'; i++) {
        if (program[i] == '\n' || program[i] == ';') {
            is_string_begin = 1;

            if (tmp_index == 0) {
                line_count--;
                continue;
            }

            tmp[tmp_index++] = '\0';
            lines[line_index] = malloc(tmp_index * sizeof(char));
            strcpy(lines[line_index], tmp);
            line_index++;
            tmp_index = 0;
            continue;
        }

        // remove multiple spaces
        if (program[i] == ' ' && program[i + 1] == ' ') {
            continue;
        }

        // remove tabs and carriage returns
        if (program[i] == '\t' || program[i] == '\r') {
            continue;
        }

        // remove spaces at the beginning of the line
        if (is_string_begin) {
            if (program[i] == ' ') {
                continue;
            }
            is_string_begin = 0;
        }

        // remove comments
        if (program[i] == '/' && program[i + 1] == '/') {
            while (program[i] != '\n' && program[i] != '\0') i++;
            continue;
        }

        tmp[tmp_index++] = program[i];
    }

    if (tmp_index == 0) {
        line_count--;
    } else {
        tmp[tmp_index++] = '\0';
        lines[line_index] = malloc(tmp_index * sizeof(char));
        strcpy(lines[line_index], tmp);
    }

    free(tmp);

    lines[line_count] = NULL;

    return lines;
}

/*************************
 *                      *
 *  Execution Functions *
 *                      *
*************************/

int check_condition(char *condition) {
    char *verif = check_subfunc(condition);
    if (verif == NULL) {
        if (SHOW_ALLFAIL)
            printf("Error: error in condition '%s'\n", condition);

        return -1;
    }

    int res = 1;

    if (
        strcmp(verif, "false") == 0 ||
        strcmp(verif, "0")     == 0 ||
        strcmp(verif, "False") == 0 ||
        strcmp(verif, "FALSE") == 0
    ) res = 0;

    if (verif != condition) {
        free(verif);
    }

    return res;
}

int get_line_end(int line_count, char **lines) {
    int line_end = 0;

    int end_offset = 1;
    for (int i = 1; i < line_count; i++) {
        if (
            does_startwith(lines[i], "IF")    ||
            does_startwith(lines[i], "FOR")   ||
            does_startwith(lines[i], "WHILE") ||
            does_startwith(lines[i], "FUNC")
        ) {
            end_offset++;
        } else if (does_startwith(lines[i], "END")) {
            end_offset--;
        }

        if (end_offset == 0) {
            line_end = i;
            break;
        }
    }

    return line_end;
}

int execute_return(char *line, char *result);
int execute_if(int line_count, char **lines);
int execute_for(int line_count, char **lines);
int execute_while(int line_count, char **lines);
int save_function(int line_count, char **lines);

int execute_lines(char **lines, int line_end, char *result) {
    // return -4 : return
    // return -3 : break
    // return -2 : continue
    // return -1 : error
    // return 0  : no error
    // return >0 : number of lines executed

    if (result != NULL) {
        result[0] = '\0';
    }

    for (int i = 0; i < line_end; i++) {
        if (i >= line_end) {
            printf("Error: trying to execute line after END\n");
            return -1;
        }

        if (does_startwith(lines[i], "FOR")) {
            int ret = execute_for(line_end - i, lines + i);
            if (ret == -1) {
                if (SHOW_ALLFAIL)
                    printf("Error: invalid FOR loop\n");

                return -1;
            }

            i += ret;
            continue;
        }

        if (does_startwith(lines[i], "IF")) {
            int ret = execute_if(line_end - i, lines + i);

            if (ret == -1) {
                if (SHOW_ALLFAIL)
                    printf("Error: invalid IF statement\n");

                return -1;
            } else if (ret < -1) {
                return ret;
            }

            i += ret;
            continue;
        }

        if (does_startwith(lines[i], "WHILE")) {
            int ret = execute_while(line_end - i, lines + i);

            if (ret == -1) {
                if (SHOW_ALLFAIL)
                    printf("Error: invalid WHILE loop\n");

                return -1;
            } else if (ret < -1) {
                return ret;
            }

            i += ret;
            continue;
        }

        if (does_startwith(lines[i], "FUNC")) {
            int ret = save_function(line_end - i, lines + i);

            if (ret == -1) {
                if (SHOW_ALLFAIL)
                    printf("Error: invalid FUNCTION declaration\n");

                return -1;
            }

            i += ret;
            continue;
        }

        if (does_startwith(lines[i], "END")) {
            printf("Error: suspicious END line %d\n", i + 1);
            return -1;
        }

        if (does_startwith(lines[i], "BREAK")) {
            return -3;
        }

        if (does_startwith(lines[i], "CONTINUE")) {
            return -2;
        }

        if (does_startwith(lines[i], "RETURN")) {
            return execute_return(lines[i], result);
        }

        char *res = execute_line(lines[i]);

        if (res != NULL) {
            if (res[0] != '\0') {
                printf("%s\n", res);
            }
            free(res);
        }
    }
    return 0;
}

int execute_return(char *line, char *result) {
    if (result != NULL) {
        if (strlen(line) < 6) {
            if (SHOW_ALLFAIL)
                printf("Error: invalid RETURN statement\n");

            return -1;
        }

        char *res = check_subfunc(line + 6);

        if (res == NULL) {
            if (SHOW_ALLFAIL)
                printf("Error: invalid RETURN statement\n");

            return -1;
        }

        free(result);
        result = res;
    }

    return -4;
}

int execute_for(int line_count, char **lines) {
    char *for_line = check_subfunc(lines[0]);

    if (for_line == NULL) {
        return -1;
    }

    char *var_name = malloc((strlen(for_line) + 1) * sizeof(char));
    char *string = malloc((strlen(for_line) + 1) * sizeof(char));

    if (for_line[3] != ' ') {
        printf("Error: missing space after FOR\n");
        free(var_name);
        free(string);

        if (for_line != lines[0]) {
            free(for_line);
        }

        return -1;
    }

    int i;
    for (i = 4; for_line[i] != ' ' && for_line[i] != '\0'; i++) {
        var_name[i - 4] = for_line[i];
    }
    var_name[i - 4] = '\0';

    if (for_line[i] == '\0') {
        printf("Error: missing string for FOR loop\n");
        free(var_name);
        free(string);

        if (for_line != lines[0]) {
            free(for_line);
        }

        return -1;
    }

    int j;
    for (j = i + 1; for_line[j] != '\0'; j++) {
        string[j - i - 1] = for_line[j];
    }

    string[j - i - 1] = '\0';

    // chek string length
    if (strlen(string) == 0) {
        printf("Error: missing string for FOR loop\n");
        free(var_name);
        free(string);

        if (for_line != lines[0]) {
            free(for_line);
        }

        return -1;
    }

    // convert string to string array
    char **string_array = gen_args(string);

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        printf("Error: missing END for FOR loop\n");
        free(var_name);
        free(string);

        if (for_line != lines[0]) {
            free(for_line);
        }

        return -1;
    }

    int var_exist_before = does_variable_exist(var_name);
    int res;

    // execute for loop
    for (int i = 0; string_array[i] != NULL; i++) {
        set_variable(var_name, string_array[i]);
        res = execute_lines(lines + 1, line_end - 1, NULL);
        if (res == -1) {
            if (SHOW_ALLFAIL)
                printf("Error: invalid FOR loop\n");
            line_end = -1;
        } else if (res == -3) {
            break;
        } else if (res == -2) {
            continue;
        }
    }

    // delete variable
    if (!var_exist_before) {
        del_variable(var_name);
    }

    free_args(string_array);

    if (for_line != lines[0]) {
        free(for_line);
    }

    free(var_name);
    free(string);

    return line_end;
}

int execute_if(int line_count, char **lines) {
    char *if_line = lines[0];

    char *condition = malloc((strlen(if_line) + 1) * sizeof(char));

    if (if_line[2] != ' ') {
        printf("Error: missing space after IF\n");
        free(condition);

        if (if_line != lines[0]) {
            free(if_line);
        }

        return -1;
    }

    int i;
    for (i = 3; if_line[i] != '\0'; i++) {
        condition[i - 3] = if_line[i];
    }

    condition[i - 3] = '\0';

    // check condition length
    if (strlen(condition) == 0) {
        printf("Error: missing condition for IF statement\n");
        free(condition);

        if (if_line != lines[0]) {
            free(if_line);
        }

        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        printf("Error: missing END for IF statement\n");
        free(condition);

        if (if_line != lines[0]) {
            free(if_line);
        }

        return -1;
    }

    // execute if statement
    int verif = check_condition(condition);

    if (verif == -1) {
        if (SHOW_ALLFAIL)
            printf("Error: invalid condition for WHILE loop\n");
        free(condition);
        return -1;
    }

    if (verif) {
        int ret = execute_lines(lines + 1, line_end - 1, NULL);
        if (ret == -1 && SHOW_ALLFAIL) {
            printf("Error: invalid IF statement\n");
        } if (ret < 0) {
            line_end = ret;
        }
    }

    free(condition);

    return line_end;
}

int execute_while(int line_count, char **lines) {
    char *while_line = lines[0];

    char *condition = malloc((strlen(while_line) + 1) * sizeof(char));

    if (while_line[5] != ' ') {
        printf("Error: missing space after WHILE\n");
        free(condition);

        if (while_line != lines[0]) {
            free(while_line);
        }

        return -1;
    }

    int i;
    for (i = 6; while_line[i] != '\0'; i++) {
        condition[i - 6] = while_line[i];
    }

    condition[i - 6] = '\0';

    // check condition length
    if (strlen(condition) == 0) {
        printf("Error: missing condition for WHILE loop\n");
        free(condition);

        if (while_line != lines[0]) {
            free(while_line);
        }

        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        printf("Error: missing END for WHILE loop\n");
        free(condition);

        if (while_line != lines[0]) {
            free(while_line);
        }

        return -1;
    }

    // execute while loop
    int verif = check_condition(condition);

    if (verif == -1) {
        if (SHOW_ALLFAIL)
            printf("Error: invalid condition for WHILE loop\n");
        free(condition);
        return -1;
    }

    while (verif) {
        int ret = execute_lines(lines + 1, line_end - 1, NULL);
        if (ret == -1 && SHOW_ALLFAIL) {
            printf("Error: invalid WHILE loop\n");
        } if (ret == -3) {
            break;
        } if (ret == -1) {
            line_end = ret;
            break;
        }

        verif = check_condition(condition);
        if (verif == -1) {
            if (SHOW_ALLFAIL)
                printf("Error: invalid condition for WHILE loop\n");

            free(condition);
            return -1;
        }
    }

    free(condition);

    return line_end;
}

int save_function(int line_count, char **lines) {
    // FUNC name;
    //  ...
    // END

    char *func_line = lines[0];

    if (func_line[4] != ' ') {
        printf("Error: missing space after FUNC\n");
        return -1;
    }

    char *func_name = malloc((strlen(func_line) + 1) * sizeof(char));
    int i;
    for (i = 5; func_line[i] != '\0'; i++) {
        func_name[i - 5] = func_line[i];
    }

    func_name[i - 5] = '\0';

    if (strlen(func_name) == 0) {
        printf("Error: missing function name\n");
        free(func_name);
        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        printf("Error: missing END for FUNC\n");
        free(func_name);
        return -1;
    }

    int ret = set_function(func_name, lines + 1, line_end - 1);

    free(func_name);

    return ret ? -1 : line_end;
}

void execute_program(char *program) {
    char **lines = lexe_program(program);
    int line_count = 0;
    for (int i = 0; lines[i] != NULL; i++) {
        line_count++;
    }

    execute_lines(lines, line_count, NULL);

    free_args(lines);
}

// input() setings
#define SLEEP_T 15
#define FIRST_L 12

// keyboard scancodes
#define SC_MAX 57

#define LSHIFT 42
#define RSHIFT 54
#define LEFT 75
#define RIGHT 77
#define OLDER 72
#define NEWER 80
#define BACKSPACE 14
#define DEL 83
#define ENTER 28
#define RESEND 224

char *keywords[] = {
    "IF",
    "WHILE",
    "FOR",
    "FUNC",
    "END",
    "RETURN",
    "BREAK",
    "CONTINUE",
    NULL
};

#if PROFANBUILD
int get_func_color(char *str) {
    // keywords: purple
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return c_magenta;
        }
    }

    // functions: dark yellow
    if (get_function(str) != NULL) {
        return c_dyellow;
    }

    // pseudos: blue
    if (get_pseudo(str) != NULL) {
        return c_blue;
    }

    // internal functions: yellow
    if (get_if_function(str) != NULL) {
        return c_yellow;
    }

    // unknown functions: red
    return c_red;
}
#endif
    

void olv_print(char *str, int len) {
    /* colored print
     * function: yellow/dark yellow
     * keywords: purple
     * unknown function: red
     * variable: cyan
     * brackets: green
    **/

    if (len == 0) {
        return;
    }

    char *tmp = malloc((len + 1) * sizeof(char));

    int is_func = 1;
    int is_var = 0;
    int i = 0;

    while (!(str[i] == '!' || str[i] == ' ' || str[i] == STRING_CHAR) && i != len) {
        i++;
    }

    memcpy(tmp, str, i);
    tmp[i] = '\0';
    c_ckprint(tmp, get_func_color(tmp));

    is_func = 0;
    int from = i;
    for (; i < len; i++) {
        if (str[i] == '!' && str[i + 1] == '(') {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                c_ckprint(tmp, is_var ? c_cyan : c_white);
            }

            // find the closing bracket
            int j;
            for (j = i + 1; j < len; j++) {
                if (str[j] == ')') {
                    break;
                }
            }

            c_ckprint("!(", c_green);
            olv_print(str + i + 2, j - i - 2);
            if (j != len) {
                c_ckprint(")", c_green);
            }
            i = j;
            from = i + 1;
        }

        // variable
        else if (str[i] == '!') {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                c_ckprint(tmp, is_var ? c_cyan : c_white);
                from = i;
            }
            is_var = 1;
        }

        else if (str[i] == ' ' || str[i] == STRING_CHAR) {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                c_ckprint(tmp, is_var ? c_cyan : c_white);
                from = i;
            }
            is_var = 0;
        }
    }

    if (from != i) {
        memcpy(tmp, str + from, i - from);
        tmp[i - from] = '\0';
        c_ckprint(tmp, is_var ? c_cyan : c_white);
    }

    free(tmp);
}

void local_input(char *buffer, int size) {
    #if PROFANBUILD

    buffer[0] = '\0';

    int old_cursor = c_get_cursor_offset();
    int sc, last_sc, last_sc_sgt = 0;

    int buffer_actual_size = 0;
    int buffer_index = 0;

    int key_ticks = 0;
    int shift = 0;

    do {
        sc = c_kb_get_scfh();
    } while (sc == ENTER);

    c_cursor_blink(1);

    while (sc != ENTER) {
        ms_sleep(SLEEP_T);

        sc = c_kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        if (!sc) continue;
        if (sc != last_sc) key_ticks = 0;
        else key_ticks++;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) continue;

        if (sc == LSHIFT || sc == RSHIFT) {
            shift = 1;
            continue;
        }

        else if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift = 0;
            continue;
        }

        else if (sc == LEFT) {
            if (!buffer_index) continue;
            buffer_index--;
        }

        else if (sc == RIGHT) {
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
        }

        else if (sc == BACKSPACE) {
            if (!buffer_index) continue;
            buffer_index--;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc <= SC_MAX) {
            if (size < buffer_actual_size + 2) continue;
            if (c_kb_scancode_to_char(sc, shift) == '?') continue;
            for (int i = buffer_actual_size; i > buffer_index; i--) {
                buffer[i] = buffer[i - 1];
            }
            buffer[buffer_index] = c_kb_scancode_to_char(sc, shift);
            buffer_actual_size++;
            buffer_index++;
        }

        c_set_cursor_offset(old_cursor);
        olv_print(buffer, buffer_actual_size);
        c_kprint(" ");
        c_set_cursor_offset(old_cursor + buffer_index * 2);
    }

    buffer[buffer_actual_size] = '\0';

    c_cursor_blink(0);
    c_kprint("\n");

    #else
    fgets(buffer, size, stdin);
    #endif
}

void start_shell() {
    // use execute_program() to create a shell
    char *line = malloc(MAX_INPUT_SIZE * sizeof(char));

    while (1) {
        printf(OLV_PROMPT, current_directory);
        local_input(line, MAX_INPUT_SIZE);

        if (strncmp(line, "exit", 4) == 0) {
            break;
        }

        execute_program(line);
    }

    free(line);
}

/********************
 *                 *
 *  Main Function  *
 *                 *
********************/

char init_prog[] = ""
"IF !profan;"
" FOR dir '/bin/commands' '/bin/fatpath';"
"  FOR e !(find -f !dir);"
"   pseudo !(name !e) 'go !e';"
"  END;"
" END;"
"END;"

"FUNC test;"
" echo argc: !#;"
" IF !#;"
"  FOR i !(range 0 !#);"
"   echo !i':' !!i;"
"  END;"
" END;"
"END";


int main(int argc, char **argv) {
    current_directory = malloc(MAX_PATH_SIZE * sizeof(char));
    strcpy(current_directory, "/");

    variables = calloc(MAX_VARIABLES, sizeof(variable_t));
    pseudos = calloc(MAX_PSEUDOS, sizeof(pseudo_t));
    functions = calloc(MAX_FUNCTIONS, sizeof(function_t));

    set_variable("version", OLV_VERSION);
    set_variable("profan", PROFANBUILD ? "1" : "0");
    set_sync_variable("path", current_directory);

    // init pseudo commands
    execute_program(init_prog);

    start_shell();

    free(current_directory);
    free_functions();
    free_pseudos();
    free_vars();

    return 0;
}
