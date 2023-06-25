#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STRING_CHAR '\''

#define ENABLE_DEBUG 0  // debug level 1
#define MORE_DEBUG   0  // debug level 2

#define MAX_INPUT_SIZE 256
#define MAX_PATH_SIZE  256

#define PROFANBUILD     // enable binary execution

#ifdef PROFANBUILD
  #include <syscall.h>
  #include <profan.h>

  // profanOS config
  #define OLV_PROMPT "olivine [$4%s$7] -> "
  #define PROFAN_COLOR "$6"
#else
  #define uint32_t unsigned int

  // unix config
  #define OLV_PROMPT "olivine [%s] -> "
  #define PROFAN_COLOR ""
#endif

#define MAX_VARIABLES 100
#define MAX_PSEUDOS   100

#define OLV_VERSION "0.2"

typedef struct {
    char* name;
    char* (*function)(char**);
} internal_function_t;

typedef struct {
    char* name;
    char* value;
} variable_t;

typedef struct {
    char* name;
    char* value;
} pseudo_t;

variable_t *variables;
pseudo_t *pseudos;
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
            return 0;
        }
        if (strcmp(variables[i].name, name) == 0) {
            if (variables[i].value) {
                free(variables[i].value);
            }
            variables[i].value = value_copy;
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
    return 1;
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

char *if_eval(char **input) {
    printf("we need to evaluate:\n");
    for (int i = 0; input[i] != NULL; i++) {
        printf(" %s", input[i]);
    }
    printf("\n");
    return NULL;
}

char *if_go_binfile(char **input) {
    #ifdef PROFANBUILD

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
    #ifdef PROFANBUILD
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

    if (start > end) {
        printf("RANGE: start is bigger than end\n");
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

    #ifdef PROFANBUILD
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
    {"SET", if_set_var},
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

void* get_function(char* name) {
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

char *get_function_name(char *string, int *size) {
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
        if (variables[i].name != NULL) {
            free(variables[i].name);
            free(variables[i].value);
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

char *check_subfunc(char *line);
char *check_variables(char *line);
char *check_pseudos(char *line);

char *execute_line(char* full_line) {
    // check for function and variable
    char *line = check_subfunc(full_line);

    if (line == NULL) {
        if (MORE_DEBUG)
            printf("Error: subfunction failed\n");

        return NULL;
    }

    // get the function name
    int name_size;
    char *function_name = get_function_name(line, &name_size);

    // get the function address
    void *function = get_function(function_name);

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
        result = ((char* (*)(char**))function)(function_args);

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
        if (MORE_DEBUG)
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
    if (
        strcmp(condition, "false") == 0 ||
        strcmp(condition, "0")     == 0 ||
        strcmp(condition, "False") == 0 ||
        strcmp(condition, "FALSE") == 0
    ) return 0;
    return 1;
}

int execute_if(int line_count, char **lines);
int execute_for(int line_count, char **lines);

int execute_for(int line_count, char **lines) {
    char *for_line = check_subfunc(lines[0]);

    if (for_line == NULL) {
        return -1;
    }

    char *var_name = malloc((strlen(for_line) + 1) * sizeof(char));
    char *string = malloc((strlen(for_line) + 1) * sizeof(char));

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

    int line_end = 0;

    int end_offset = 1;
    for (int i = 1; i < line_count; i++) {
        if (does_startwith(lines[i], "FOR") || does_startwith(lines[i], "IF")) {
            end_offset++;
        } else if (does_startwith(lines[i], "END")) {
            end_offset--;
        }

        if (end_offset == 0) {
            line_end = i;
            break;
        }
    }

    if (line_end == 0) {
        printf("Error: missing END for FOR loop\n");
        free(var_name);
        free(string);

        if (for_line != lines[0]) {
            free(for_line);
        }

        return -1;
    }

    // execute for loop
    for (int i = 0; string_array[i] != NULL; i++) {
        for (int j = 1; j < line_end; j++) {
            set_variable(var_name, string_array[i]);
            if (does_startwith(lines[j], "FOR")) {
                int ret = execute_for(line_end - j, lines + j);
                if (ret == -1) {
                    if (MORE_DEBUG)
                        printf("Error: invalid FOR loop\n");

                    return -1;
                }

                j += ret;
                continue;
            }
            if (does_startwith(lines[j], "IF")) {
                int ret = execute_if(line_end - j, lines + j);
                if (ret == -1) {
                    if (MORE_DEBUG)
                        printf("Error: invalid IF statement\n");

                    return -1;
                }

                j += ret;
                continue;
            }

            char *result = execute_line(lines[j]);

            if (result != NULL) {
                if (result[0] != '\0') {
                    printf("%s\n", result);
                }
                free(result);
            }
        }
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
    char *if_line = check_subfunc(lines[0]);

    if (if_line == NULL) {
        return -1;
    }

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

    int line_end = 0;
    
    int end_offset = 1;
    for (int i = 1; i < line_count; i++) {
        if (does_startwith(lines[i], "IF") || does_startwith(lines[i], "FOR")) {
            end_offset++;
        } else if (does_startwith(lines[i], "END")) {
            end_offset--;
        }

        if (end_offset == 0) {
            line_end = i;
            break;
        }
    }

    if (line_end == 0) {
        printf("Error: missing END for IF statement\n");
        free(condition);

        if (if_line != lines[0]) {
            free(if_line);
        }

        return -1;
    }

    // execute if statement
    if (check_condition(condition)) {
        for (int i = 1; i < line_end; i++) {
            if (does_startwith(lines[i], "FOR")) {
                int ret = execute_for(line_end - i, lines + i);
                if (ret == -1) {
                    if (MORE_DEBUG)
                        printf("Error: invalid FOR loop\n");

                    return -1;
                }

                i += ret;
                continue;
            }

            if (does_startwith(lines[i], "IF")) {
                int ret = execute_if(line_end - i, lines + i);
                if (ret == -1) {
                    if (MORE_DEBUG)
                        printf("Error: invalid IF statement\n");

                    return -1;
                }

                i += ret;
                continue;
            }

            char *result = execute_line(lines[i]);

            if (result != NULL) {
                if (result[0] != '\0') {
                    printf("%s\n", result);
                }
                free(result);
            }
        }
    }

    if (if_line != lines[0]) {
        free(if_line);
    }

    free(condition);

    return line_end;
}

void execute_lines(int line_count, char **lines) {
    for (int line_index = 0; line_index < line_count; line_index++) {
        if (does_startwith(lines[line_index], "FOR")) {
            int ret = execute_for(line_count - line_index, lines + line_index);
            if (ret == -1) {
                if (MORE_DEBUG)
                    printf("Error: invalid FOR loop\n");

                return;
            }

            line_index += ret;
            continue;
        }
        if (does_startwith(lines[line_index], "IF")) {
            int ret = execute_if(line_count - line_index, lines + line_index);
            if (ret == -1) {
                if (MORE_DEBUG)
                    printf("Error: invalid IF statement\n");

                return;
            }

            line_index += ret;
            continue;
        }

        char *result = execute_line(lines[line_index]);

        if (result != NULL) {
            if (result[0] != '\0') {
                printf("%s\n", result);
            }
            free(result);
        }
    }
}

void execute_program(char *program) {
    char **lines = lexe_program(program);
    int line_count = 0;
    for (int i = 0; lines[i] != NULL; i++) {
        line_count++;
    }

    execute_lines(line_count, lines);

    free_args(lines);
}

void start_shell() {
    // use execute_program() and fgets() to create a shell
    char *line = malloc(MAX_INPUT_SIZE * sizeof(char));

    while (1) {
        printf(OLV_PROMPT, current_directory);
        fgets(line, MAX_INPUT_SIZE, stdin);

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
"FOR dir '/bin/commands' '/bin/fatpath';"
" FOR e !(find -f !dir);"
"  pseudo !(name !e) 'go !e';"
" END;"
"END";

char test_prog[] = ""
"FOR i !(range 0 10);"
" echo !i;"
" IF !i;"
"  echo 'i is not 0';"
" END;"
"echo coucou;"
"END";

int main(int argc, char** argv) {
    current_directory = malloc(MAX_PATH_SIZE * sizeof(char));
    strcpy(current_directory, "/");

    variables = calloc(MAX_VARIABLES, sizeof(variable_t));
    set_variable("version", OLV_VERSION);

    pseudos = calloc(MAX_PSEUDOS, sizeof(pseudo_t));

    // init pseudo commands
    execute_program(init_prog);

    // execute test program
    execute_program(test_prog);

    start_shell();

    free(current_directory);
    free_pseudos();
    free_vars();

    return 0;
}