#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>

typedef struct {
    char* name;
    char* (*function)(char**);
} internal_function_t;

char program[] = "echo !(upper kernel version: !(kver))";

/**************************************
 *                                   *
 *  Olivine Lang Internal Functions  *
 *                                   *
**************************************/

char *if_clear(char **input) {
    printf("clear function executed!\n");
    return NULL;
}

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
        required_size += strlen(input[i]) + 1;
    }

    char *result = malloc(required_size * sizeof(char));

    int result_i = 0;
    for (int i = 0; input[i] != NULL; i++) {
        for (int j = 0; input[i][j] != '\0'; j++) {
            if (input[i][j] >= 'a' && input[i][j] <= 'z') {
                result[result_i] = input[i][j] - 32;
            } else {
                result[result_i] = input[i][j];
            }
            result_i++;
        }
        result[result_i] = ' ';
        result_i++;
    }

    result[required_size - 1] = '\0';

    return result;
}

char *if_kver(char **input) {
    char *result = malloc((64) * sizeof(char));
    c_sys_kinfo(result);
    return result;
}

internal_function_t internal_functions[] = {
    {"clear", if_clear},
    {"echo", if_echo},
    {"upper", if_upper},
    {"kver", if_kver},
    {NULL, NULL}
};

/************************************
 *                                 *
 *  String Manipulation Functions  *
 *                                 *
************************************/

void* get_function(char* name) {
    for (int i = 0; internal_functions[i].name != NULL; i++) {
        if (strcmp(internal_functions[i].name, name) == 0) {
            return internal_functions[i].function;
        }
    }
    return NULL;
}

char **gen_args(char *string) {
    if (string == NULL || strlen(string) == 0) {
        char **argv = malloc(1 * sizeof(char*));
        argv[0] = NULL;
        return argv;
    }

    // count the number of arguments
    int argc = 1;
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == ' ') {
            argc++;
        }
    }

    // allocate the arguments array
    char **argv = malloc((argc + 1) * sizeof(char*));

    // fill the arguments array
    int old_i = 0;
    int arg_i = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == ' ') {
            argv[arg_i] = malloc((i - old_i + 1) * sizeof(char));
            strncpy(argv[arg_i], string + old_i, i - old_i);
            old_i = i + 1;
            arg_i++;
        }
    }

    // add the last argument
    argv[arg_i] = malloc((strlen(string) - old_i + 1) * sizeof(char));
    strcpy(argv[arg_i], string + old_i);

    // add the NULL terminator
    argv[argc] = NULL;

    return argv;
}

void free_args(char **argv) {
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

/**************************
 *                       *
 *  Execution Functions  *
 *                       *
**************************/

char *get_function_name(char *string) {
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == ' ') {
            char *function_name = malloc((i + 1) * sizeof(char));
            strncpy(function_name, string, i);
            return function_name;
        }
    }

    char *function_name = malloc((strlen(string) + 1) * sizeof(char));
    strcpy(function_name, string);

    return function_name;
}

void debug_print(char *function_name, char **function_args) {
    printf("$6%s(", function_name);

    for (int i = 0; function_args[i] != NULL; i++) {
        if (function_args[i + 1] != NULL) {
            printf("$6'%s', ", function_args[i]);
            continue;
        }
        printf("$6'%s') [%d]\n", function_args[i], i + 1);
    }
}

char *check_subfunc(char *line);

char *execute_line(char* full_line) {
    // check if !( ... ) is present
    char *line = check_subfunc(full_line);

    if (line == NULL) {
        printf("Error: subfunction failed\n");
        return NULL;
    }

    // get the function name
    char *function_name = get_function_name(line);

    // get the function address
    void *function = get_function(function_name);

    if (function == NULL) {
        printf("Function '%s' not found\n", function_name);
        free(function_name);
        return NULL;
    }

    // generate the arguments array
    char **function_args = gen_args(line + strlen(function_name) + 1);

    debug_print(function_name, function_args);

    // execute the function
    char *result = ((char* (*)(char**))function)(function_args);

    if (result == NULL) {
        result = malloc(1 * sizeof(char));
        result[0] = '\0';
    }

    free_args(function_args);
    free(function_name);

    if (line != full_line) {
        free(line);
    }
    free(full_line);

    return result;
}

char *check_subfunc(char *line) {
    int start = -1;
    int end = -1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '!' && line[i + 1] == '(') {
            start = i;
            break;
        }
    }

    if (start == -1) {
        return line;
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
        printf("Error: missing closing parenthesis\n");
        return NULL;
    }

    char *subfunc = malloc((end - start - 1) * sizeof(char));
    strncpy(subfunc, line + start + 2, end - start - 2);

    // execute the subfunc
    char *subfunc_result = execute_line(subfunc);
    free(subfunc);

    if (subfunc_result == NULL) {
        printf("Error: subfunc returned NULL\n");
        return NULL;
    }

    // replace the subfunc with its result
    char *new_line = malloc((strlen(line) - (end - start) + strlen(subfunc_result) + 1) * sizeof(char));
    strncpy(new_line, line, start);
    strcat(new_line, subfunc_result);
    strcat(new_line, line + end + 1);

    free(subfunc_result);

    char *rec = check_subfunc(new_line);
    if (rec != new_line) {
        free(new_line);
    }
    return rec;
}

int main(int argc, char** argv) {
    char *copy = malloc((strlen(program) + 1) * sizeof(char));
    strcpy(copy, program);

    char *res = execute_line(copy);

    free(res);

    return 0;
}
