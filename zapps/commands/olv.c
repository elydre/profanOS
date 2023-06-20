#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STRING_CHAR '\''

#define ENABLE_DEBUG 0  // debug level 1
#define MORE_DEBUG 0    // debug level 2

#define MAX_VARIABLES 100

#define OLV_VERSION "0.2"

#define PROFAN_COLOR "$6"

typedef struct {
    char* name;
    char* (*function)(char**);
} internal_function_t;

typedef struct {
    char* name;
    char* value;
} variable_t;


variable_t *variables;
internal_function_t internal_functions[];

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
            printf("  %s: %s\n", variables[i].name, variables[i].value);
        }
    }

    // print functions
    printf("FUNCTIONS\n");
    for (int i = 0; internal_functions[i].name != NULL; i++) {
        printf("  %s: %p\n", internal_functions[i].name, internal_functions[i].function);
    }

    return NULL;
}

char *if_eval(char **input) {
    printf("we need to evaluate:\n");
    for (int i = 0; input[i] != NULL; i++) {
        printf(" %s\n", input[i]);
    }
    return NULL;
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

char *execute_line(char* full_line) {
    // check for function and variable
    char *line = check_subfunc(full_line);

    if (line == NULL) {
        if (MORE_DEBUG)
            printf("Error: subfunction failed\n");

        free(full_line);
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

    free(full_line);

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
        if (var_line != line) {
            free(line);
        }

        return var_line;
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
        if (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' || 
            line[i] == '!' || line[i] == STRING_CHAR) {
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
    for (int i = 0; program[i] != '\0'; i++) {
        if (program[i] == '\n' || program[i] == ';') {
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

void execute_program(char *program) {
    char **lines = lexe_program(program);

    for (int line_index = 0; lines[line_index] != NULL; line_index++) {
        char *result = execute_line(lines[line_index]);

        if (result != NULL) {
            if (result[0] != '\0') {
                printf("%s\n", result);
            }
            free(result);
        }
    }
    free(lines);
}

void start_shell() {
    // use execute_program() and fgets() to create a shell
    printf("Welcome to the olive v%s shell!\n", OLV_VERSION);
    printf("Type 'exit' to exit\n");

    char *line = malloc(256 * sizeof(char));

    while (1) {
        printf(">>> ");
        fgets(line, 256, stdin);

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

int main(int argc, char** argv) {
    variables = calloc(MAX_VARIABLES, sizeof(variable_t));
    set_variable("version", OLV_VERSION);

    // execute_program("echo Hello World");
    start_shell();

    free_vars();

    return 0;
}
