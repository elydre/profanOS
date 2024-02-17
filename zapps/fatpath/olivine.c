#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define STRING_CHAR '\''

#define ENABLE_DEBUG  0  // print function calls
#define PROFANBUILD   1  // enable profan features
#define USE_ENVVARS   1  // enable environment variables
#define STOP_ON_ERROR 0  // stop after first error

#define OLV_VERSION "0.10 rev 5"

#define HISTORY_SIZE  100
#define INPUT_SIZE    1024
#define PROMPT_SIZE   100
#define MAX_PATH_SIZE 256
#define MAX_SUGGESTS  10

#define MAX_VARIABLES 100
#define MAX_PSEUDOS   100
#define MAX_FUNCTIONS 100

#define devio_set_redirection(a, b, c) 0

/******************************
 *                           *
 *  Structs and Definitions  *
 *                           *
******************************/

#ifdef NoProfan
  #ifdef PROFANBUILD
    #undef PROFANBUILD
  #endif
  #define PROFANBUILD 0
#endif

#if PROFANBUILD
  #include <syscall.h>
  #include <filesys.h>
  #include <unistd.h>
  #include <profan.h>

  // profanOS config
  #define DEFAULT_PROMPT "\e[0mprofanOS [\e[95m$d\e[0m] $(\e[91m$)>$(\e[0m$) "
#else
  #define uint32_t unsigned int
  #define uint8_t  unsigned char

  // unix config
  #define DEFAULT_PROMPT "${olivine$}$(-ERROR-$) > "
#endif

#define DEBUG_COLOR  "\e[37m"

#define OTHER_PROMPT "> "
#define CD_DEFAULT   "/"

#define ERROR_CODE ((void *) 1)

#ifndef LOWERCASE
  #define LOWERCASE(x) ((x) >= 'A' && (x) <= 'Z' ? (x) + 32 : (x))
#endif

#ifndef min
  #define min(a, b) ((a) < (b) ? (a) : (b))
#endif

char *keywords[] = {
    "IF",
    "ELSE",
    "WHILE",
    "FOR",
    "FUNC",
    "END",
    "RETURN",
    "BREAK",
    "CONTINUE",
    NULL
};

typedef struct {
    char* name;
    char* (*function)(char**);
} internal_function_t;

typedef struct {
    char* name;
    char* value;
    int is_sync;
    int level;
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

char *g_current_directory, *g_exit_code, *g_prompt;
int g_current_level;

void execute_file(char *file);

/****************************
 *                         *
 *  Local Tools Functions  *
 *                         *
****************************/

void local_itoa(int n, char *buffer) {
    int i = 0;
    int sign = 0;

    if (n < 0) {
        sign = 1;
        n = -n;
    }

    do {
        buffer[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char tmp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = tmp;
    }
}

int local_atoi(char *str, int *result) {
    int sign, found, base;
    int res = 0;

    char *base_str;

    if (str[0] == '-') {
        sign = 1;
        str++;
    } else {
        sign = 0;
    }

    if (str[0] == '0' && str[1] == 'x') {
        base_str = "0123456789abcdef";
        base = 16;
        str += 2;
    } else if (str[0] == '0' && str[1] == 'b') {
        base_str = "01";
        base = 2;
        str += 2;
    } else {
        base_str = "0123456789";
        base = 10;
    }

    for (int i = 0; str[i] != '\0'; i++) {
        found = 0;
        for (int j = 0; base_str[j] != '\0'; j++) {
            if (LOWERCASE(str[i]) == base_str[j]) {
                res *= base;
                res += j;
                found = 1;
                break;
            }
        }
        if (!found) {
            return 1;
        }
    }

    if (sign) {
        res = -res;
    }

    if (result != NULL) {
        *result = res;
    }

    return 0;
}

void raise_error(char *part, char *format, ...) {
    if (part == NULL) fputs("OLIVINE raise: ", stderr);
    else fprintf(stderr, "'%s' raise: ", part);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputs("\n", stderr);

    strcpy(g_exit_code, "1");
}

/*******************************
 *                            *
 * Variable Get/Set Functions *
 *                            *
********************************/

int get_variable_index(char *name) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            break;
        }

        if (strcmp(variables[i].name, name) == 0
            && (variables[i].level == g_current_level
            || variables[i].level == 0)
        ) return i;
    }

    return -1;
}

char *get_variable(char *name) {
    int index = get_variable_index(name);

    if (index != -1) {
        return variables[index].value;
    }

    if (USE_ENVVARS) {
        return getenv(name);
    }

    return NULL;
}

#define set_variable(name, value) set_variable_withlen(name, value, -1)

int set_variable_withlen(char *name, char *value, int str_len) {
    char *value_copy = NULL;

    int index = get_variable_index(name);

    if (str_len == -1) str_len = strlen(value);

    if ((index != -1 && !variables[index].is_sync) || index == -1) {
        value_copy = malloc(str_len + 1);
        strncpy(value_copy, value, str_len);
        value_copy[str_len] = '\0';
    } else value_copy = NULL;

    if (index != -1) {
        if (variables[index].is_sync) {
            strncpy(variables[index].value, value, min(str_len, variables[index].is_sync));
            variables[index].value[min(str_len, variables[index].is_sync)] = '\0';
        } else {
            if (variables[index].value) {
                free(variables[index].value);
            }
            variables[index].value = value_copy;
        }
        return 0;
    }

    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            char *name_copy = malloc(strlen(name) + 1);
            strcpy(name_copy, name);
            variables[i].name = name_copy;
            variables[i].value = value_copy;
            variables[i].level = g_current_level;
            variables[i].is_sync = 0;
            return 0;
        }
    }

    raise_error(NULL, "Cannot set value of '%s', more than %d variables", name, MAX_VARIABLES);
    return 1;
}

int set_sync_variable(char *name, char *value, int max_len) {
    int index = get_variable_index(name);

    if (index != -1) {
        if (variables[index].value && (!variables[index].is_sync)) {
            free(variables[index].value);
        }
        variables[index].level = variables[index].level ? g_current_level : 0;
        variables[index].value = value;
        variables[index].is_sync = max_len;
        return 0;
    }

    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            variables[i].name = name;
            variables[i].value = value;
            variables[i].level = g_current_level;
            variables[i].is_sync = max_len;
            return 0;
        }
    }

    raise_error(NULL, "Cannot set value of '%s', more than %d variables", name, MAX_VARIABLES);
    return 1;
}

int does_variable_exist(char *name) {
    return (get_variable_index(name) != -1) ||
        (USE_ENVVARS && getenv(name));
}

int del_variable(char *name) {
    int index = get_variable_index(name);

    if (index != -1) {
        if (variables[index].value && (!variables[index].is_sync)) {
            free(variables[index].value);
            free(variables[index].name);
        }
        variables[index].value = NULL;
        variables[index].name = NULL;
        variables[index].level = 0;
        variables[index].is_sync = 0;

        // shift all variables down
        for (int j = index; j < MAX_VARIABLES - 1; j++) {
            variables[j] = variables[j + 1];
        }
        return 0;
    }

    if (USE_ENVVARS && getenv(name)) {
        unsetenv(name);
        return 0;
    }

    raise_error(NULL, "Variable '%s' does not exist", name);

    return 1;
}

void del_variable_level(int level) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            break;
        }
        if (variables[i].level >= level) {
            if (variables[i].value && (!variables[i].is_sync)) {
                free(variables[i].value);
                free(variables[i].name);
            }
            variables[i].value = NULL;
            variables[i].name = NULL;
            variables[i].level = 0;
            variables[i].is_sync = 0;

            // shift all variables down
            for (int j = i; j < MAX_VARIABLES - 1; j++) {
                variables[j] = variables[j + 1];
            }
            i--;
        }
    }
}

/*******************************
 *                            *
 *  pseudo Get/Set Functions  *
 *                            *
*******************************/

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
    raise_error(NULL, "Cannot set value of '%s', more than %d pseudos", name, MAX_PSEUDOS);
    return 1;
}

int del_pseudo(char *name) {
    for (int i = 0; i < MAX_PSEUDOS; i++) {
        if (pseudos[i].name == NULL) {
            break;
        }
        if (strcmp(pseudos[i].name, name) == 0) {
            free(pseudos[i].value);
            free(pseudos[i].name);

            // shift all pseudos down
            for (int j = i; j < MAX_PSEUDOS - 1; j++) {
                pseudos[j] = pseudos[j + 1];
            }
            return 0;
        }
    }
    raise_error(NULL, "Pseudo '%s' does not exist", name);
    return 1;
}

/*******************************
 *                            *
 * Function Get/Set Functions *
 *                            *
********************************/

int print_function(char *name) {
    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name == NULL) {
            break;
        }
        if (strcmp(functions[i].name, name) == 0) {
            printf("Function %s:\n", name);
            for (int j = 0; j < functions[i].line_count; j++) {
                printf("| %s\n", functions[i].lines[j]);
            }
            return 0;
        }
    }
    raise_error(NULL, "Function %s does not exist", name);
    return 1;
}

int del_function(char *name) {
    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name == NULL) {
            break;
        }
        if (strcmp(functions[i].name, name) == 0) {
            free(functions[i].lines);
            free(functions[i].name);

            // shift all functions down
            for (int j = i; j < MAX_FUNCTIONS - 1; j++) {
                functions[j] = functions[j + 1];
            }
            return 0;
        }
    }
    raise_error(NULL, "Function %s does not exist", name);
    return 1;
}

int set_function(char *name, char **lines, int line_count) {
    int char_count = 0;
    for (int i = 0; i < line_count; i++) {
        char_count += strlen(lines[i]) + 1;
    }

    char **lines_copy = malloc(sizeof(char *) * line_count + char_count);
    char *lines_ptr = (char *) lines_copy + sizeof(char *) * line_count;

    for (int i = 0; i < line_count; i++) {
        lines_copy[i] = lines_ptr;
        strcpy(lines_ptr, lines[i]);
        lines_ptr += strlen(lines[i]) + 1;
    }

    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name == NULL) {
            char *name_copy = malloc(strlen(name) + 1);
            strcpy(name_copy, name);
            functions[i].name = name_copy;
            functions[i].line_count = line_count;
            functions[i].lines = lines_copy;
            return 0;
        } if (strcmp(functions[i].name, name) == 0) {
            free(functions[i].lines);
            functions[i].line_count = line_count;
            functions[i].lines = lines_copy;
            return 0;
        }
    }

    raise_error(NULL, "Cannot set function '%s', more than %d functions", name, MAX_FUNCTIONS);
    return 1;
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

char ops[] = "=~<>@.+-*/^()";

void free_ast(ast_t *ast) {
    if (ast->left.type == AST_TYPE_AST) {
        free_ast((ast_t *) ast->left.ptr);
    }

    if (ast->right.type == AST_TYPE_AST) {
        free_ast((ast_t *) ast->right.ptr);
    }

    free(ast);
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
        raise_error("eval", "No operator found in expression");
        free_ast(ast);
        return NULL;
    }

    // generate ast
    if (op_index == 0) {
        ast->left.type = AST_TYPE_STR;
        ast->left.ptr = str[0];
    } else {
        ast->left.type = AST_TYPE_AST;
        ast->left.ptr = gen_ast(str, op_index);
        if (ast->left.ptr == NULL) {
            free_ast(ast);
            return NULL;
        }
    }

    ast->center.type = AST_TYPE_STR;
    ast->center.ptr = str[op_index];

    if (len - op_index - 1 == 1) {
        ast->right.type = AST_TYPE_STR;
        ast->right.ptr = str[op_index + 1];
    } else {
        ast->right.type = AST_TYPE_AST;
        ast->right.ptr = gen_ast(str + op_index + 1, len - op_index - 1);
        if (ast->right.ptr == NULL) {
            free_ast(ast);
            return NULL;
        }
    }

    return ast;
}

char *calculate_integers(int left, int right, char *op) {
    int result;
    if (op[0] == '/' && right == 0) {
        raise_error("eval", "Cannot divide by 0");
        return NULL;
    }

    if (op[0] == '^' && right == 0) {
        raise_error("eval", "Cannot modulo by 0");
        return NULL;
    }

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
        case '^':
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
        case '~':
            result = left != right;
            break;
        default:
            raise_error("eval", "Unknown operator '%s' between integers", op);
            return NULL;
    }

    // convert back to string
    char *ret = malloc(sizeof(char) * 12);
    local_itoa(result, ret);
    return ret;
}

char *calculate_strings(char *left, char *right, char *op) {
    char *res, *tmp;
    int nb = 0;

    if (local_atoi(right, &nb)) {
        if (local_atoi(left, &nb)) tmp = NULL;
        else tmp = right;
    } else tmp = left;

    if (op[0] == '+' || op[0] == '.') {
        res = malloc(strlen(left) + strlen(right) + 1);
        strcpy(res, left);
        strcat(res, right);
    } else if (op[0] == '=') {
        res = malloc(sizeof(char) * 2);
        res[0] = (strcmp(left, right) == 0) + '0';
        res[1] = '\0';
    } else if (op[0] == '~') {
        res = malloc(sizeof(char) * 2);
        res[0] = (strcmp(left, right) != 0) + '0';
        res[1] = '\0';
    } else if (op[0] == '*') {
        if (tmp == NULL) {
            raise_error("eval", "Cannot multiply string by string");
            return NULL;
        }
        res = malloc(sizeof(char) * (strlen(tmp) + 1) * nb + 1);
        res[0] = '\0';
        for (int i = 0; i < nb; i++) {
            strcat(res, tmp);
        }
    } else if (op[0] == '@') {
        if (tmp == NULL) {
            raise_error("eval", "Cannot get character from string");
            return NULL;
        }
        if (nb < 0 || nb >= (int) strlen(tmp)) {
            raise_error("eval", "Cannot get character %d from string of length %d", nb, strlen(tmp));
            return NULL;
        }
        res = malloc(sizeof(char) * 2);
        res[0] = tmp[nb];
        res[1] = '\0';
    } else {
        raise_error("eval", "Unknown operator '%s' between strings", op);
        return NULL;
    }
    return res;
}

char *eval(ast_t *ast) {
    int left, right, no_number = 0;
    char *res = NULL;

    // if only one element return it
    if (ast->left.type == AST_TYPE_NIL && ast->right.type == AST_TYPE_NIL) {
        res = malloc(strlen((char *) ast->center.ptr) + 1);
        no_number = local_atoi((char *) ast->center.ptr, &left);
        if (no_number) {
            strcpy(res, (char *) ast->center.ptr);
        } else {
            local_itoa(left, res);
        }
        return res;
    }

    if (ast->center.type == AST_TYPE_NIL) {
        raise_error("eval", "Operators must be surrounded by two elements (got '%s', '%s')",
            ast->left.type == AST_TYPE_NIL ? "nil" : ast->left.type == AST_TYPE_STR ? (char *) ast->left.ptr : "ast",
            ast->right.type == AST_TYPE_NIL ? "nil" :  ast->right.type == AST_TYPE_STR ? (char *) ast->right.ptr : "ast"
        );
        return ERROR_CODE;
    }

    // convert to int
    char *op = (char *) ast->center.ptr;
    char *left_str, *right_str;

    if (ast->left.type == AST_TYPE_AST) {
        left_str = eval((ast_t *) ast->left.ptr);
    } else {
        left_str = (char *) ast->left.ptr;
    }

    if (ast->right.type == AST_TYPE_AST) {
        right_str = eval((ast_t *) ast->right.ptr);
    } else {
        right_str = (char *) ast->right.ptr;
    }

    right_str = right_str == ERROR_CODE ? NULL : right_str;
    left_str = left_str == ERROR_CODE ? NULL : left_str;

    if (left_str != NULL && right_str != NULL) {
        no_number = local_atoi(left_str, &left) || local_atoi(right_str, &right);
        no_number |= op[0] == '.' || op[0] == '@';
        res = no_number ? calculate_strings(left_str, right_str, op) : calculate_integers(left, right, op);
    }

    if (left_str != NULL && ast->left.type == AST_TYPE_AST) {
        free(left_str);
    }

    if (right_str != NULL && ast->right.type == AST_TYPE_AST) {
        free(right_str);
    }

    return res;
}

void eval_help(void) {
    puts("Olivine Integrated Evaluator\n"
        "Usage: eval <string> [string...]\n"
        "Spaces are not required between operators\n\n"
        "Number operators:\n"
        " +  Addition\n"
        " -  Substraction\n"
        " *  Multiplication\n"
        " /  Division\n"
        " ^  Modulo\n"
        " <  Less than\n"
        " >  Greater than\n"
        " =  Equal\n"
        " ~  Not equal\n"
        "String operators:\n"
        " +  Concatenation\n"
        " .  Concatenation (with number)\n"
        " *  Repeat\n"
        " @  Get character\n"
        " =  Equal\n"
        " ~  Not equal\n\n"
        "Operators priority (from lowest to highest):");
    for (uint32_t i = 0; i < sizeof(ops); i++) {
        printf(" %c", ops[i]);
    }
    puts("\n\nExample: eval 1 + 2 * 3\n"
        "         eval 'hello ' * 3\n"
        "         eval (1+3)*2=8\n");
}

char *if_eval(char **input) {
    if (input[0] == NULL) {
        raise_error("eval", "Requires at least one argument");
        return ERROR_CODE;
    }

    if (input[1] == NULL && (
        strcmp(input[0], "-h") == 0 ||
        strcmp(input[0], "--help") == 0
    )) {
        eval_help();
        return NULL;
    }

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
    char *res;

    if (ast) {
        res = eval(ast);
        free_ast(ast);
    } else {
        res = NULL;
    }

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

char *if_cd(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc > 1) {
        raise_error("cd", "Usage: cd [dir]");
        return ERROR_CODE;
    }

    // change to default if no arguments
    if (argc == 0) {
        strcpy(g_current_directory, CD_DEFAULT);
        if (!USE_ENVVARS) return NULL;
        setenv("PWD", g_current_directory, 1);
        return NULL;
    }

    // get dir
    char *dir;

    #if PROFANBUILD
    // check if dir exists
    dir = assemble_path(g_current_directory, input[0]);
    // simplify path
    fu_simplify_path(dir);

    sid_t dir_id = fu_path_to_sid(ROOT_SID, dir);
    if (IS_NULL_SID(dir_id) || !fu_is_dir(dir_id)) {
        raise_error("cd", "Directory '%s' does not exist", dir);
        free(dir);
        return ERROR_CODE;
    }
    #else
    dir = strdup(input[0]);
    #endif

    // change directory
    strcpy(g_current_directory, dir);
    if (USE_ENVVARS) {
        setenv("PWD", g_current_directory, 1);
    }

    free(dir);

    return NULL;
}

char *if_debug(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    int mode = 0;

    if (argc == 0) {
        mode = 0;
    } else if (argc == 1) {
        if (strcmp(input[0], "-v") == 0) {
            mode = 1;
        } else if (strcmp(input[0], "-if") == 0) {
            mode = 2;
        } else if (strcmp(input[0], "-f") == 0) {
            mode = 3;
        } else if (strcmp(input[0], "-p") == 0) {
            mode = 4;
        } else if (strcmp(input[0], "-a") == 0) {
            mode = 5;
        } else {
            raise_error("debug", "Unknown argument '%s'", input[0]);
            puts("expected '-v', '-if', '-f', '-p' or '-a'");
            return ERROR_CODE;
        }
    } else {
        raise_error("debug", "Usage: debug [-v|-if|-f|-p|-a]");
        return ERROR_CODE;
    }

    // print variables
    if (mode == 0 || mode == 1) {
        puts("VARIABLES");
        for (int i = 0; i < MAX_VARIABLES && variables[i].name != NULL; i++) {
            printf("  %s (lvl: %d, sync: %d): '%s'\n", variables[i].name, variables[i].level, variables[i].is_sync, variables[i].value);
        }
    }

    // print internal functions
    if (mode == 0 || mode == 2) {
        puts("INTERNAL FUNCTIONS");
        for (int i = 0; internal_functions[i].name != NULL; i++) {
            printf("  %s: %p\n", internal_functions[i].name, internal_functions[i].function);
        }
    }

    // print functions
    if (mode == 0 || mode == 3) {
        puts("FUNCTIONS");
        for (int i = 0; i < MAX_FUNCTIONS && functions[i].name != NULL; i++) {
            printf("  %s: %d lines (%p)\n", functions[i].name, functions[i].line_count, functions[i].lines);
        }
    }

    // print pseudos
    if (mode == 0 || mode == 4) {
        puts("PSEUDOS");
        for (int i = 0; i < MAX_PSEUDOS && pseudos[i].name != NULL; i++) {
            printf("  %s: '%s'\n", pseudos[i].name, pseudos[i].value);
        }
    }

    if (mode != 5) {
        return NULL;
    }

    // save all info in a file
    FILE *f = fopen("debug.txt", "w");

    // print variables
    fprintf(f, "VARIABLES (max %d):", MAX_VARIABLES);
    for (int i = 0; i < MAX_VARIABLES && variables[i].name != NULL; i++) {
        fprintf(f, " %s(l%d, s%d)='%s'", variables[i].name, variables[i].level, variables[i].is_sync, variables[i].value);
    }

    // print internal functions
    fprintf(f, "\nINTERNAL FUNCTIONS:");
    for (int i = 0; internal_functions[i].name != NULL; i++) {
        fprintf(f, " %s:%p", internal_functions[i].name, internal_functions[i].function);
    }

    // print pseudos
    fprintf(f, "\nPSEUDOS (max %d):", MAX_PSEUDOS);
    for (int i = 0; i < MAX_PSEUDOS && pseudos[i].name != NULL; i++) {
        fprintf(f, " %s='%s'", pseudos[i].name, pseudos[i].value);
    }

    // print functions
    fprintf(f, "\nFUNCTIONS (max %d):", MAX_FUNCTIONS);
    for (int i = 0; i < MAX_FUNCTIONS && functions[i].name != NULL; i++) {
        fprintf(f, "\n  %s: %d lines (%p)\n", functions[i].name, functions[i].line_count, functions[i].lines);
        for (int j = 0; j < functions[i].line_count; j++) {
            fprintf(f, "  | %s\n", functions[i].lines[j]);
        }
    }
    fclose(f);

    return NULL;
}

char *if_del_var(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        raise_error("del", "Usage: del <variable name>");
        return ERROR_CODE;
    }

    // get name
    char *name = input[0];

    // delete variable
    if (del_variable(name)) {
        return ERROR_CODE;
    }

    return NULL;
}

char *if_delfunc(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        raise_error("delfunc", "Usage: delfunc <function name>");
        return ERROR_CODE;
    }

    // get name
    char *name = input[0];

    // delete function
    if (del_function(name)) {
        return ERROR_CODE;
    }

    return NULL;
}

char *if_exec(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        raise_error("exec", "Usage: exec <file>");
        return ERROR_CODE;
    }

    execute_file(input[0]);

    return NULL;
}

char *if_exit(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) argc++;

    if (argc > 1) {
        raise_error("exit", "Usage: exit [code]");
        return ERROR_CODE;
    }

    // get code
    int code = 0;
    if (argc == 1) {
        if (local_atoi(input[0], &code)) {
            raise_error("exit", "Invalid code '%s'", input[0]);
            return ERROR_CODE;
        }
    }

    // exit
    exit(code);

    return NULL;
}

char *if_export(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2) {
        raise_error("export", "Usage: export <name> <value>");
        return ERROR_CODE;
    }

    if (!USE_ENVVARS) {
        raise_error("export", "Environment variables are disabled");
        return ERROR_CODE;
    }

    setenv(input[0], input[1], 1);

    return NULL;
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

    if (argc > 2) {
        raise_error("find", "Usage: find [-f|-d] [dir]");
        return ERROR_CODE;
    }

    int required_type = 0;
    if (strcmp(input[0], "-f") == 0) {
        required_type = 1;
    } else if (strcmp(input[0], "-d") == 0) {
        required_type = 2;
    }

    char *user_path;
    if (argc == 0 || (argc == 1 && required_type)) user_path = "";
    if (argc == 1 && !required_type) user_path = input[0];
    if (argc == 2) user_path = input[1];

    char *path = assemble_path(g_current_directory, user_path);

    sid_t dir_id = fu_path_to_sid(ROOT_SID, path);

    if (IS_NULL_SID(dir_id) || !fu_is_dir(dir_id)) {
        raise_error("find", "Directory '%s' does not exist", path);
        free(path);
        return ERROR_CODE;
    }

    sid_t *out_ids;
    char **names;

    int elm_count = fu_get_dir_content(dir_id, &out_ids, &names);

    if (elm_count < 1) {
        free(path);
        return NULL;
    }

    char *output = malloc(1 * sizeof(char));
    output[0] = '\0';

    char *tmp_path;

    for (int i = 0; i < elm_count; i++) {
        if (!required_type ||
            (fu_is_dir(out_ids[i]) && required_type == 2) ||
            (fu_is_file(out_ids[i]) && required_type == 1)
        ) {
            if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0) {
                continue;
            }
            tmp_path = assemble_path(path, names[i]);
            fu_simplify_path(tmp_path);
            char *tmp = malloc((strlen(output) + strlen(tmp_path) + 4));
            if (output[0] != '\0') {
                sprintf(tmp, "%s '%s'", output, tmp_path);
            } else {
                sprintf(tmp, "'%s'", tmp_path);
            }
            free(tmp_path);
            free(output);
            output = tmp;
        }
    }

    for (int i = 0; i < elm_count; i++) {
        free(names[i]);
    }
    free(out_ids);
    free(names);
    free(path);

    return output;
    #else
    raise_error("find", "Not supported in this build");
    return ERROR_CODE;
    #endif
}

char *if_fsize(char **input) {
    int file_size = 0;

    #if PROFANBUILD
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        raise_error("fsize", "Usage: fsize <file>");
        return ERROR_CODE;
    }

    // get path
    char *path = assemble_path(g_current_directory, input[0]);

    sid_t file_id = fu_path_to_sid(ROOT_SID, path);

    // check if file exists
    if (IS_NULL_SID(file_id)) {
        file_size = -1;
    } else {
        file_size = c_fs_cnt_get_size(c_fs_get_main(), file_id);
    }

    free(path);
    #endif

    char *res = malloc(sizeof(char) * 12);
    if (file_size == -1) {
        strcpy(res, "null");
    } else {
        local_itoa(file_size, res);
    }

    return res;
}

char *if_global(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2) {
        raise_error("global", "Usage: global <name> <value>");
        return ERROR_CODE;
    }

    // get name
    char *name = input[0];

    // get value
    char *value = input[1];

    // copy the current level
    int old_level = g_current_level;
    g_current_level = 0;

    // set variable
    if (set_variable(name, value)) {
        return ERROR_CODE;
    }

    // restore the current level
    g_current_level = old_level;

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
        raise_error("go", "Usage: go <file> [args] [&] [> <stdout>]");
        return ERROR_CODE;
    }

    // get file name
    char *file_path = assemble_path(g_current_directory, input[0]);

    // check if file exists
    sid_t file_id = fu_path_to_sid(ROOT_SID, file_path);

    if (IS_NULL_SID(file_id) || !fu_is_file(file_id)) {
        raise_error("go", "File '%s' does not exist", file_path);
        free(file_path);
        return ERROR_CODE;
    }

    int wait_end = 1;
    if (strcmp(input[argc - 1], "&") == 0) {
        wait_end = 0;
        argc--;
    };

    char *stdout_path = NULL;
    char *stdin_path = NULL;
    sid_t sid;

    if (argc >= 3 && strcmp(input[argc - 2], "<") == 0) {
        stdin_path = assemble_path(g_current_directory, input[argc - 1]);
        argc -= 2;
    }

    if (argc >= 3 && strcmp(input[argc - 2], ">") == 0) {
        stdout_path = assemble_path(g_current_directory, input[argc - 1]);
        argc -= 2;
    }

    if (!stdin_path && argc >= 3 && strcmp(input[argc - 2], "<") == 0) {
        stdin_path = assemble_path(g_current_directory, input[argc - 1]);
        argc -= 2;
    }

    if (strcmp(input[argc - 1], "&") == 0) {
        wait_end = 0;
        argc--;
    }

    if (argc < 1) {
        raise_error("go", "No given executable path");
        free(stdout_path);
        free(stdin_path);
        free(file_path);
        return ERROR_CODE;
    }

    if (stdin_path) {
        sid = fu_path_to_sid(ROOT_SID, stdin_path);
        if (IS_NULL_SID(sid)) {
            raise_error("go", "Cannot redirect stdin from '%s'", stdin_path);
            free(stdout_path);
            free(stdin_path);
            free(file_path);
            return ERROR_CODE;
        }
    }

    if (stdout_path) {
        sid = fu_path_to_sid(ROOT_SID, stdout_path);
        if (IS_NULL_SID(sid)) {
            if (IS_NULL_SID(fu_file_create(0, stdout_path))) {
                free(stdout_path);
                free(stdin_path);
                free(file_path);
                return ERROR_CODE;
            }
        }
        else if (!(fu_is_file(sid) || fu_is_fctf(sid))) {
            raise_error("go", "Cannot redirect stdout to '%s'", stdout_path);
            free(stdout_path);
            free(stdin_path);
            free(file_path);
            return ERROR_CODE;
        }
    }

    // get args
    char *file_name = strdup(input[0]);
    // remove the extension
    char *dot = strrchr(file_name, '.');
    if (dot) *dot = '\0';
    // remove the path
    char *slash = strrchr(file_name, '/');
    if (slash) memmove(file_name, slash + 1, strlen(slash));

    char **argv = malloc((argc + 1) * sizeof(char*));
    argv[0] = file_name;
    for (int i = 1; i < argc; i++) {
        argv[i] = input[i];
    }
    argv[argc] = NULL;

    int pid;
    local_itoa(c_run_ifexist_full(
        (runtime_args_t){file_path, file_id, argc, argv, 0, 0, 0, stdout_path || stdin_path ? 2 : wait_end}, &pid
    ), g_exit_code);

    if (stdin_path) {
        fm_reopen(fm_resol012(0, pid), stdin_path);
        free(stdin_path);
    }

    if (stdout_path) {
        fm_reopen(fm_resol012(1, pid), stdout_path);
        free(stdout_path);
    }

    if (stdout_path || stdin_path) {
        c_process_wakeup(pid);
        if (wait_end) {
            profan_wait_pid(pid);
        }
    }

    if (pid == -1) {
        raise_error("go", "Cannot execute file '%s'", file_path);
        free(file_path);
        free(file_name);
        free(argv);
        return ERROR_CODE;
    }

    if (!wait_end) {
        printf("GO: started with pid %d\n", pid);
    }

    char *tmp = malloc(10 * sizeof(char));

    local_itoa(pid, tmp);
    set_variable("spi", tmp);

    free(file_path);
    free(file_name);
    free(argv);
    free(tmp);
    return g_exit_code[0] == '0' ? NULL : ERROR_CODE;

    #else
    raise_error("go", "Not available in this build");
    return ERROR_CODE;

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
        raise_error("name", "Usage: name <path>");
        return ERROR_CODE;
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

char *if_print(char **input) {
    for (int i = 0; input[i] != NULL; i++) {
        fputs(input[i], stdout);
    }
    fflush(stdout);
    return NULL;
}

char *if_pseudo(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2) {
        raise_error("pseudo", "Usage: pseudo <name> <value>");
        return ERROR_CODE;
    }

    // get name
    char *name = input[0];

    // get value
    char *value = input[1];

    if (strcmp(name, value) == 0) {
        raise_error("pseudo", "Cannot set pseudo '%s' to itself", name);
        return ERROR_CODE;
    }

    // set pseudo
    if (set_pseudo(name, value)) {
        return ERROR_CODE;
    }

    // check for infinite loop
    char **history = calloc(MAX_PSEUDOS, sizeof(char *));
    int history_len = 0, rec_found = 0;

    while ((value = get_pseudo(value)) != NULL) {
        for (int i = 0; i < history_len; i++) {
            if (strcmp(history[i], value) == 0) {
                rec_found = 1;
                break;
            }
        }
        if (rec_found) break;
        history[history_len++] = value;
    }
    history_len--;

    if (rec_found) {
        raise_error("pseudo", "Setting pseudo '%s' would create a infinite loop", name);
        fprintf(stderr, "Detailed chain: [%s -> %s -> ", name, history[history_len]);
        for (int i = 0; i < history_len; i++) {
            fprintf(stderr, "%s%s", history[i], i == history_len - 1 ? "]\n" : " -> ");
        }

        free(history);
        del_pseudo(name);
        return ERROR_CODE;
    }

    free(history);
    return NULL;
}

char *if_range(char **input) {
    /*
     * input: ["1", "5"]
     * output: "1 2 3 4 5"
    */

    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc == 0 || argc > 2) {
        raise_error("range", "Usage: range [start] <end>");
        return ERROR_CODE;
    }

    int start, end, nb_len;

    if (argc == 1) {
        start = 0;
        nb_len = strlen(input[0]);
        end = atoi(input[0]);
    } else {
        start = atoi(input[0]);
        end = atoi(input[1]);
        if (strlen(input[0]) > strlen(input[1])) {
            nb_len = strlen(input[0]);
        } else {
            nb_len = strlen(input[1]);
        }
    }

    if (start > end) {
        raise_error("range", "Start must be less than end, got %d and %d", start, end);
        return ERROR_CODE;
    } else if (start == end) {
        return NULL;
    }

    char *output = malloc((nb_len + 1) * (end - start + 1));
    char *tmp = malloc(12 * sizeof(char));
    tmp[0] = '\0';

    int output_len = 0;

    for (int i = start; i < end; i++) {
        local_itoa(i, tmp);
        strcat(tmp, " ");
        for (int j = 0; tmp[j] != '\0'; j++) {
            output[output_len++] = tmp[j];
        }
    }
    output[--output_len] = '\0';

    free(tmp);

    return output;
}

char *if_rep(char **input) {
    /*
     * input ["hello", "hl", "HP"]
     * output: "HePPo"
    */

    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 2 && argc != 3) {
        raise_error("rep", "Usage: rep <string> <chars> [replacements]");
        return ERROR_CODE;
    }

    if (argc == 3 && strlen(input[1]) != strlen(input[2])) {
        raise_error("rep", "Expected 2nd and 3rd arguments to have the same length, got %d and %d", strlen(input[1]), strlen(input[2]));
        return ERROR_CODE;
    }

    char *ret = malloc((strlen(input[0]) + 1) * sizeof(char));

    if (argc == 3) {
        strcpy(ret, input[0]);

        for (int i = 0; ret[i] != '\0'; i++) {
            for (int j = 0; input[1][j] != '\0'; j++) {
                if (ret[i] == input[1][j]) {
                    ret[i] = input[2][j];
                    break;
                }
            }
        }
        return ret;
    }

    int index = 0;
    for (int i = 0; input[0][i] != '\0'; i++) {
        int found = 0;
        for (int j = 0; input[1][j] != '\0'; j++) {
            if (input[0][i] == input[1][j]) {
                found = 1;
                break;
            }
        }
        if (!found) {
            ret[index++] = input[0][i];
        }
    }
    ret[index] = '\0';

    return ret;
}

char *if_set_var(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc == 0 || argc > 2) {
        raise_error("set", "Usage: set <name> [value]");
        return ERROR_CODE;
    }

    // get value
    char *value = input[1];

    // set variable if a value is given
    if (value) {
        if (set_variable(input[0], value)) {
            return ERROR_CODE;
        }
        return NULL;
    }

    // get value from stdin
    if (argc == 1) {
        value = malloc(INPUT_SIZE * sizeof(char));
        if (!fgets(value, INPUT_SIZE, stdin)) {
            raise_error("set", "Cannot read from stdin");
            free(value);
            return ERROR_CODE;
        }
        value[strlen(value) - 1] = '\0';
    }

    // set variable
    if (set_variable(input[0], value)) {
        return ERROR_CODE;
    }

    free(value);

    return NULL;
}

char *if_sprintf(char **input) {
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc < 1) {
        raise_error("sprintf", "Usage: sprintf <format> [arg1] [arg2] ...");
        return ERROR_CODE;
    }

    char *format = input[0];


    int arg_i = 1;

    char *res = malloc(0x1000);
    res[0] = '\0';
    int res_i = 0;

    for (int format_i = 0; format[format_i] != '\0'; format_i++) {
        if (format[format_i] != '%') {
            res[res_i++] = format[format_i];
            continue;
        }

        format_i++;
        if (format[format_i] == '%') {
            res[res_i++] = '%';
            continue;
        }

        if (input[arg_i] == NULL) {
            raise_error("sprintf", "%%%c requires an argument, but none given", format[format_i]);
            return ERROR_CODE;
        }

        if (format[format_i] == 's') {
            for (int i = 0; input[arg_i][i] != '\0'; i++) {
                res[res_i++] = input[arg_i][i];
            }
        } else if (format[format_i] == 'd') {
            int nb;
            if (local_atoi(input[arg_i], &nb)) {
                raise_error("sprintf", "%%%c requires an integer, but got '%s'", format[format_i], input[arg_i]);
                return ERROR_CODE;
            }
            char *nb_str = malloc(12);
            local_itoa(nb, nb_str);
            for (int i = 0; nb_str[i] != '\0'; i++) {
                res[res_i++] = nb_str[i];
            }
            free(nb_str);
        } else if (format[format_i] == 'c') {
            int nb;
            if (local_atoi(input[arg_i], &nb) || nb < 0 || nb > 255) {
                if (strlen(input[arg_i]) == 1) {
                    res[res_i++] = input[arg_i][0];
                } else {
                    raise_error("sprintf", "%%%c requires a character, but got '%s'", format[format_i], input[arg_i]);
                    return ERROR_CODE;
                }
            } else {
                res[res_i++] = nb;
            }
        } else {
            raise_error("sprintf", "Unknown format specifier '%%%c'", format[format_i]);
            return ERROR_CODE;
        }
        arg_i++;
    }

    res[res_i] = '\0';

    return res;
}

char *if_strlen(char **input) {
    /*
     * input: ["hello world"]
     * output: "11"
    */

    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        raise_error("strlen", "Usage: strlen <string>");
        return ERROR_CODE;
    }

    char *output = malloc(11 * sizeof(char));

    local_itoa(strlen(input[0]), output);

    return output;
}

char *if_ticks(char **input) {
    /*
     * input: []
     * output: "'123456789'"
    */

    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 0) {
        raise_error("ticks", "Usage: ticks");
        return ERROR_CODE;
    }

    char *output = malloc(11 * sizeof(char));

    #if PROFANBUILD
    local_itoa(c_timer_get_ms(), output);
    #else
    strcpy(output, "0");
    #endif

    return output;
}


internal_function_t internal_functions[] = {
    {"cd", if_cd},
    {"debug", if_debug},
    {"del", if_del_var},
    {"delfunc", if_delfunc},
    {"eval", if_eval},
    {"exec", if_exec},
    {"exit", if_exit},
    {"export", if_export},
    {"find", if_find},
    {"fsize", if_fsize},
    {"global", if_global},
    {"go", if_go_binfile},
    {"print", if_print},
    {"pseudo", if_pseudo},
    {"name", if_name},
    {"range", if_range},
    {"rep", if_rep},
    {"set", if_set_var},
    {"sprintf", if_sprintf},
    {"strlen", if_strlen},
    {"ticks", if_ticks},
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
        if (string[i] == '\\' && string[i + 1] == STRING_CHAR) {
            string[i - dec] = STRING_CHAR;
            continue;
        }
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
    if (string == NULL || !*string) {
        char **argv = malloc(1 * sizeof(char*));
        argv[0] = NULL;
        return argv;
    }

    // count the number of arguments
    int in_string = 0;
    int argc = 1;

    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == STRING_CHAR && (i == 0 || string[i - 1] != '\\')) {
            in_string = !in_string;
        } if (string[i] == ' ' && !in_string) {
            argc++;
        }
    }

    if (in_string) {
        raise_error(NULL, "String not closed (%s)", string);
        return ERROR_CODE;
    }

    // allocate the arguments array
    char **argv = malloc((argc + 1) * sizeof(char*));

    // fill the arguments array
    int old_i = 0;
    int arg_i = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        if (string[i] == STRING_CHAR && (i == 0 || string[i - 1] != '\\')) {
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
    argv[argc - 1] = strdup(string + old_i);
    remove_quotes(argv[argc - 1]);

    argv[argc] = NULL;

    return argv;
}

char *args_rejoin(char **input, int to) {
    int required_size = 1;
    for (int i = 0; i < to; i++) {
        required_size += strlen(input[i]) + 3;
    }

    char *joined_input = malloc(required_size * sizeof(char));
    joined_input[0] = '\0';

    // add quotes if needed
    for (int i = 0; i < to; i++) {
        if (strchr(input[i], ' ') != NULL) {
            strcat(joined_input, "'");
            strcat(joined_input, input[i]);
            strcat(joined_input, "'");
        } else {
            strcat(joined_input, input[i]);
        }
        if (i != to - 1) {
            strcat(joined_input, " ");
        }
    }
    return joined_input;
}

char *get_function_name(char *line) {
    int in_string = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == STRING_CHAR) {
            in_string = !in_string;
        }

        if (line[i] == ' ' && !in_string) {
            char *function_name = malloc((i + 1) * sizeof(char));
            strncpy(function_name, line, i);
            function_name[i] = '\0';

            remove_quotes(function_name);
            return function_name;
        }
    }

    char *function_name = strdup(line);

    remove_quotes(function_name);
    return function_name;
}

/*******************************
 *                            *
 *  Freeing Memory Functions  *
 *                            *
*******************************/

void free_args(char **argv) {
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

void free_vars(void) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name != NULL && (!variables[i].is_sync)) {
            free(variables[i].value);
            free(variables[i].name);
        }
    }
    free(variables);
}

void free_pseudos(void) {
    for (int i = 0; i < MAX_PSEUDOS; i++) {
        if (pseudos[i].name != NULL) {
            free(pseudos[i].name);
            free(pseudos[i].value);
        }
    }
    free(pseudos);
}

void free_functions(void) {
    for (int i = 0; i < MAX_FUNCTIONS; i++) {
        if (functions[i].name != NULL) {
            free(functions[i].name);
            free(functions[i].lines);
        }
    }
    free(functions);
}

/*****************************
 *                          *
 *  Olivine Pipe Processor  *
 *                          *
*****************************/

char *execute_line(char *full_line);

char *pipe_processor(char **input) {
    #if PROFANBUILD
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc == 0) {
        raise_error("Pipe Processor", "Requires at least one argument");
        return ERROR_CODE;
    }

    int old_fds[4] = {
        dup(0),
        dup(1),
        dup(3),
        dup(4)
    };

    char *line, *tmp, *ret = NULL;
    int fd[2], fdin, start = 0;
    fdin = dup(0);
    for (int i = 0; i < argc + 1; i++) {
        if (strcmp(input[i], "|") && i != argc)
            continue;

        if (argc == i) {
            dup2(old_fds[1], 1);
            dup2(old_fds[3], 4);
            dup2(fdin, 0);
            dup2(fdin, 3);
        }

        if (i == start) {
            if (i != argc) {
                raise_error("Pipe Processor", "Empty command");
                ret = ERROR_CODE;
                break;
            }

            // copy pipe data to ret
            int n, s = 0;
            ret = malloc(101);
            while ((n = read(fdin, ret + s, 100)) > 0) {
                s += n;
                ret = realloc(ret, s + 101);
            }
            while (ret[s - 1] == '\n' || ret[s - 1] == '\0') {
                s--;
            }
            ret[s] = '\0';
            break;
        }

        line = args_rejoin(input + start, i - start);
        start = i + 1;

        if (argc != i) {
            pipe(fd);
            dup2(fdin, 3);
            dup2(fdin, 0);
            dup2(fd[1], 4);
            dup2(fd[1], 1);
        }

        tmp = execute_line(line);
        free(line);

        close(fd[1]);
        close(fdin);

        fdin = fd[0];

        if (tmp == ERROR_CODE) {
            ret = ERROR_CODE;
            break;
        }
        free(tmp);
    }

    dup2(old_fds[1], 1);
    dup2(old_fds[3], 4);
    dup2(old_fds[0], 0);
    dup2(old_fds[2], 3);

    // if line execution failed, print pipe content
    if (tmp == ERROR_CODE) {
        raise_error("Pipe Processor", "Command failed");
        puts("\e[37m  --- Pipe Content ---\e[0m");
        int n;
        char buffer[101];
        while ((n = read(fdin, buffer, 100)) > 0) {
            buffer[n] = '\0';
            fputs(buffer, stdout);
        }
        puts("\e[37m  --------------------\e[0m");
    }

    close(fdin);

    for (int i = 0; i < 4; i++) {
        close(old_fds[i]);
    }

    return ret;
    #endif
    raise_error("Pipe Processor", "Not supported in this build");
    return ERROR_CODE;
}

/**************************
 *                       *
 *  Execution Functions  *
 *                       *
**************************/

void debug_print(char *function_name, char **function_args) {
    printf(DEBUG_COLOR "'%s' (", function_name);

    for (int i = 0; function_args[i] != NULL; i++) {
        if (function_args[i + 1] != NULL) {
            printf("'%s', ", function_args[i]);
            continue;
        }
        printf(DEBUG_COLOR "'%s') [%d]\e[0m\n", function_args[i], i + 1);
        return;
    }
    puts(DEBUG_COLOR ") [0]\e[0m");
}

int execute_lines(char **lines, int line_end, char **result);

char *execute_function(function_t *function, char **args) {
    // set variables:
    // !0: first argument
    // !1: second argument
    // !2: third argument
    // !#: argument count

    g_current_level++;

    int argc;
    char tmp[4];
    for (argc = 0; args[argc] != NULL; argc++) {
        local_itoa(argc, tmp);
        if (!set_variable(tmp, args[argc]))
            continue;
        g_current_level--;
        return ERROR_CODE;
    }

    local_itoa(argc, tmp);
    if (set_variable("#", tmp)) {
        g_current_level--;
        return ERROR_CODE;
    }

    char *result = malloc(sizeof(char));
    int ret = execute_lines(function->lines, function->line_count, &result);

    // free variables
    del_variable_level(g_current_level);

    g_current_level--;

    if (ret == -1) {
        // function failed
        free(result);
        return ERROR_CODE;
    }

    return result;
}

char *check_subfunc(char *line);
char *check_variables(char *line);
char *check_pseudos(char *line);

char *execute_line(char *full_line) {
    // check for function and variable
    char *line = check_subfunc(full_line);
    int pipe_after = 0;

    if (line == NULL) {
        // subfunction failed
        return ERROR_CODE;
    }

    // get the function name
    int isif;
    char *function_name = get_function_name(line);

    // get the function address
    void *function = get_if_function(function_name);
    if (function == NULL) {
        function = get_function(function_name);
        isif = 0;
    } else isif = 1;

    char *result;

    if (function == NULL) {
        raise_error(NULL, "Function '%s' not found", function_name);
        result = ERROR_CODE;
    } else {
        // generate the arguments array
        char **function_args = gen_args(line);
        if (function_args == ERROR_CODE) {
            free(function_name);

            if (line != full_line) {
                free(line);
            }

            return ERROR_CODE;
        }

        // check if "|" is present
        for (int i = 0; function_args[i] != NULL; i++) {
            if (function_args[i][0] == '|' && function_args[i][1] == '\0') {
                pipe_after = 1;
                break;
            }
        }

        if (ENABLE_DEBUG)
            debug_print(function_name, function_args);

        // execute the function
        if (pipe_after)
            result = pipe_processor(function_args);
        else if (isif)
            result = ((char* (*)(char**)) function)(function_args + 1);
        else
            result = execute_function(function, function_args + 1);

        free_args(function_args);
    }

    if (line != full_line)
        free(line);
    free(function_name);

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
        if (pseudo_line != var_line && var_line != line) {
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
        raise_error(NULL, "Missing closing parenthesis in '%s'", line);
        return NULL;
    }

    char *subfunc = malloc((end - start - 1) * sizeof(char));
    strncpy(subfunc, line + start + 2, end - start - 2);
    subfunc[end - start - 2] = '\0';

    // execute the subfunc
    char *subfunc_result = execute_line(subfunc);
    free(subfunc);

    if (subfunc_result == ERROR_CODE) {
        // subfunc execution failed
        return NULL;
    }

    // replace the subfunc with its result
    char *new_line;

    if (subfunc_result) {
        new_line = malloc((strlen(line) - (end - start) + strlen(subfunc_result) + 1) * sizeof(char));
        strncpy(new_line, line, start);
        new_line[start] = '\0';

        strcat(new_line, subfunc_result);
        strcat(new_line, line + end + 1);
        free(subfunc_result);
    } else {
        new_line = malloc((strlen(line) - (end - start) + 1) * sizeof(char));
        strncpy(new_line, line, start);
        new_line[start] = '\0';

        strcat(new_line, line + end + 1);
    }

    char *rec = check_subfunc(new_line);

    if (rec != new_line && new_line != line) {
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

    return pseudo_line;
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
        raise_error(NULL, "Variable '%s' not found", var_name);
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

    char *rec = check_pseudos(new_line);

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
    int line_index = 1;
    for (int i = 0; program[i] != '\0'; i++) {
        if (program[i] == '\n' || program[i] == ';') {
            line_index++;
        }
    }
    int index_size = (line_index + 1) * sizeof(char*);
    line_index = 0;

    int program_len = strlen(program) + 1;
    char *tmp = malloc((program_len) * sizeof(char));

    char **lines = calloc(index_size + (program_len), sizeof(char));

    char *line_ptr = (char*) lines + index_size;

    int tmp_index = 0;
    int is_string_begin = 1;
    for (int i = 0; program[i] != '\0'; i++) {
        if (program[i] == '\n' || program[i] == ';') {
            is_string_begin = 1;

            while (tmp_index > 0 && tmp[tmp_index - 1] == ' ') {
                tmp_index--;
            }

            if (tmp_index == 0) {
                continue;
            }

            tmp[tmp_index++] = '\0';

            lines[line_index] = line_ptr;
            strcpy(lines[line_index], tmp);

            line_ptr += tmp_index;
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

        // interpret double backslashes
        if (program[i] == '\\') {
            if (program[i + 1] == 'n') {
                tmp[tmp_index++] = '\n';
            } else if (program[i + 1] == 't') {
                tmp[tmp_index++] = '\t';
            } else if (program[i + 1] == 'r') {
                tmp[tmp_index++] = '\r';
            } else if (program[i + 1] == '\\') {
                tmp[tmp_index++] = '\\';
            } else {
                tmp[tmp_index++] = '\\';
                tmp[tmp_index++] = program[i + 1];
            }
            i++;
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
            while (program[i] != '\0' && program[i + 1] != '\n' && program[i + 1] != ';') i++;
            if (program[i] == '\0') break;
            continue;
        }

        tmp[tmp_index++] = program[i];
    }

    if (tmp_index != 0) {
        while (tmp_index > 0 && tmp[tmp_index - 1] == ' ') {
            tmp_index--;
        }
        tmp[tmp_index++] = '\0';
        lines[line_index] = line_ptr;
        strcpy(lines[line_index], tmp);
    }

    free(tmp);

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
        // error in condition
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
            does_startwith(lines[i], "FUNC")  ||
            does_startwith(lines[i], "ELSE")
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

int execute_if(int line_count, char **lines, char **result, int *cnd_state);
int execute_else(int line_count, char **lines, char **result, int *last_if_state);
int execute_while(int line_count, char **lines, char **result);
int execute_for(int line_count, char **lines, char **result);
int save_function(int line_count, char **lines);
int execute_return(char *line, char **result);

int execute_lines(char **lines, int line_end, char **result) {
    // return -4 : return
    // return -3 : break
    // return -2 : continue
    // return -1 : error
    // return  0 : no error
    // return >0 : number of lines executed

    if (result != NULL) {
        *result[0] = '\0';
    }

    if (line_end == 0) {
        strcpy(g_exit_code, "0");
        return 0;
    }

    int lastif_state = 2; // 0: false, 1: true, 2: not set
    char *res = NULL;

    for (int i = 0; i < line_end; i++) {
        if (i >= line_end) {
            raise_error(NULL, "Trying to execute line after END");
            return -1;
        }

        else if (does_startwith(lines[i], "FOR")) {
            int ret = execute_for(line_end - i, lines + i, result);
            if (ret == -1) {
                // invalid FOR loop
                return -1;
            } else if (ret < -1) {
                return ret;
            }

            i += ret;
        }

        else if (does_startwith(lines[i], "IF")) {
            int ret = execute_if(line_end - i, lines + i, result, &lastif_state);

            if (ret == -1) {
                // invalid IF statement
                return -1;
            } else if (ret < -1) {
                return ret;
            }

            i += ret;
        }

        else if (does_startwith(lines[i], "ELSE")) {
            int ret = execute_else(line_end - i, lines + i, result, &lastif_state);

            if (ret == -1) {
                // invalid ELSE statement
                return -1;
            } else if (ret < -1) {
                return ret;
            }

            i += ret;
        }

        else if (does_startwith(lines[i], "WHILE")) {
            int ret = execute_while(line_end - i, lines + i, result);

            if (ret == -1) {
                // invalid WHILE loop
                return -1;
            } else if (ret < -1) {
                return ret;
            }

            i += ret;
        }

        else if (does_startwith(lines[i], "FUNC")) {
            int ret = save_function(line_end - i, lines + i);

            if (ret == -1) {
                // invalid FUNCTION declaration
                return -1;
            } else if (ret < -1) {
                return ret;
            }

            i += ret;
        }

        else if (does_startwith(lines[i], "END")) {
            raise_error(NULL, "Suspicious END line %d", i + 1);
            return -1;
        }

        else if (does_startwith(lines[i], "BREAK")) {
            return -3;
        }

        else if (does_startwith(lines[i], "CONTINUE")) {
            return -2;
        }

        else if (does_startwith(lines[i], "RETURN")) {
            return execute_return(lines[i], result);
        }

        else {
            res = execute_line(lines[i]);

            if (res == ERROR_CODE) {
                if (STOP_ON_ERROR)
                    return -1;
            } else if (res) {
                if (res[0] != '\0')
                    puts(res);
                free(res);
                res = NULL;
            }
        }

        if (res != ERROR_CODE) {
            strcpy(g_exit_code, "0");
        }
    }
    return 0;
}

int execute_return(char *line, char **result) {
    if (strlen(line) < 7) {
        return -4;
    }

    char *copy = strdup(line + 7);
    char *res = check_subfunc(copy);
    if (res != copy) {
        free(copy);
    }

    if (res == NULL) {
        // invalid RETURN statement
        return -1;
    }

    if (result != NULL) {
        free(*result);
        *result = res;
    } else {
        free(res);
    }

    return -4;
}

int execute_for(int line_count, char **lines, char **result) {
    char *for_line = check_subfunc(lines[0]);

    if (for_line == NULL) {
        return -1;
    }

    char *var_name = malloc((strlen(for_line) + 1) * sizeof(char));
    char *string = malloc((strlen(for_line) + 1) * sizeof(char));

    if (for_line[3] != ' ') {
        raise_error(NULL, "Missing space after FOR");
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

    int line_end = get_line_end(line_count, lines);

    if (for_line[i] == '\0') {
        string[0] = '\0';
    } else {
        int j;
        for (j = i + 1; for_line[j] != '\0'; j++) {
            string[j - i - 1] = for_line[j];
        }

        string[j - i - 1] = '\0';
    }

    // chek string length
    if (strlen(string) == 0) {
        free(var_name);
        free(string);

        if (for_line != lines[0]) {
            free(for_line);
        }

        return line_end;
    }

    if (line_end == 0) {
        raise_error(NULL, "Missing END for FOR loop");
        free(var_name);
        free(string);

        if (for_line != lines[0]) {
            free(for_line);
        }

        return -1;
    }

    int var_exist_before = does_variable_exist(var_name);


    int var_len = 0;
    int string_index = 0;
    int in_string = 0;

    int res;
    // execute the loop
    while (string[string_index]) {
        // set the the variable

        for (var_len = 0; string[string_index + var_len] != '\0'; var_len++) {
            if (string[string_index + var_len] == STRING_CHAR) {
                in_string = !in_string;
            } else if (!in_string && string[string_index + var_len] == ' ') {
                break;
            }
        }

        set_variable_withlen(var_name, string + string_index, var_len);

        string_index += var_len;

        while (string[string_index] == ' ') {
            string_index++;
        }

        // execute the iteration
        res = execute_lines(lines + 1, line_end - 1, result);
        if (res == -1) {
            // invalid FOR loop
            line_end = -1;
            break;
        } else if (res == -3) {
            break;
        } else if (res == -2) {
            continue;
        } else if (res < 0) {
            line_end = res;
            break;
        }
    }

    // delete variable
    if (!var_exist_before) {
        del_variable(var_name);
    }

    if (for_line != lines[0]) {
        free(for_line);
    }

    free(var_name);
    free(string);

    return line_end;
}

int execute_if(int line_count, char **lines, char **result, int *cnd_state) {
    char *if_line = lines[0];

    char *condition = malloc((strlen(if_line) + 1) * sizeof(char));

    if (if_line[2] != ' ') {
        raise_error(NULL, "Missing space after IF");
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
        raise_error(NULL, "Missing condition for IF statement");
        free(condition);

        if (if_line != lines[0]) {
            free(if_line);
        }

        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        raise_error(NULL, "Missing END for IF statement");
        free(condition);

        if (if_line != lines[0]) {
            free(if_line);
        }

        return -1;
    }

    // execute if statement
    int verif = check_condition(condition);

    if (verif == -1) {
        // invalid condition for WHILE loop
        free(condition);
        return -1;
    }

    *cnd_state = verif;

    if (verif) {
        int ret = execute_lines(lines + 1, line_end - 1, result);
        if (ret < 0) line_end = ret;
    }

    free(condition);

    return line_end;
}

int execute_else(int line_count, char **lines, char **result, int *last_if_state) {
    char *else_line = lines[0];

    if (*last_if_state == 2) {   // not set
        raise_error(NULL, "ELSE statement without IF");
        return -1;
    }

    if (else_line[4] != '\0') {
        raise_error(NULL, "Invalid ELSE statement");
        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        raise_error(NULL, "Missing END for ELSE statement");
        return -1;
    }

    if (!(*last_if_state)) {
        int ret = execute_lines(lines + 1, line_end - 1, result);
        if (ret < 0) line_end = ret;
    }

    *last_if_state = !(*last_if_state);

    return line_end;
}

int execute_while(int line_count, char **lines, char **result) {
    char *while_line = lines[0];

    char *condition = malloc((strlen(while_line) + 1) * sizeof(char));

    if (while_line[5] != ' ') {
        raise_error(NULL, "Missing space after WHILE");
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
        raise_error(NULL, "Missing condition for WHILE loop");
        free(condition);

        if (while_line != lines[0]) {
            free(while_line);
        }

        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        raise_error(NULL, "Missing END for WHILE loop");
        free(condition);

        if (while_line != lines[0]) {
            free(while_line);
        }

        return -1;
    }

    // execute while loop
    int verif = check_condition(condition);

    if (verif == -1) {
        // invalid condition for WHILE loop
        free(condition);
        return -1;
    }

    while (verif) {
        int ret = execute_lines(lines + 1, line_end - 1, result);
        if (ret == -3) {
            break;
        } if (ret < 0 && ret != -2) {
            line_end = ret;
            break;
        }

        verif = check_condition(condition);
        if (verif == -1) {
            // invalid condition for WHILE loop
            line_end = -1;
            break;
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
        raise_error(NULL, "Missing space after FUNC");
        return -1;
    }

    char *func_name = malloc((strlen(func_line) + 1) * sizeof(char));
    int i;
    for (i = 5; func_line[i] != '\0'; i++) {
        func_name[i - 5] = func_line[i];
    }

    func_name[i - 5] = '\0';

    if (strlen(func_name) == 0) {
        raise_error(NULL, "Missing function name");
        free(func_name);
        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == 0) {
        raise_error(NULL, "Missing END for FUNC");
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

    free(lines);
}

int does_syntax_fail(char *program) {
    char **lines = lexe_program(program);

    // check if all 'IF', 'WHILE', 'FOR' and 'FUNC' have a matching 'END'
    int open = 0;
    for (int i = 0; lines[i] != NULL; i++) {
        char *line = lines[i];
        if (strncmp(line, "IF", 2) == 0) {
            open++;
        } else if (strncmp(line, "WHILE", 5) == 0) {
            open++;
        } else if (strncmp(line, "FOR", 3) == 0) {
            open++;
        } else if (strncmp(line, "ELSE", 4) == 0) {
            open++;
        } else if (strncmp(line, "FUNC", 4) == 0) {
            open++;
        } else if (strncmp(line, "END", 3) == 0) {
            open--;
        }
    }
    free(lines);

    if (open > 0) {
        return 1;
    }

    return 0;
}

/**********************
 *                   *
 *  Input Functions  *
 *                   *
**********************/

// input() setings
#define SLEEP_T 11
#define FIRST_L 12

// keyboard scancodes
#define SC_MAX 57

#define LSHIFT 42
#define RSHIFT 54
#define LEFT   75
#define RIGHT  77
#define OLDER  72
#define NEWER  80
#define BACK   14
#define DEL    83
#define ENTER  28
#define RESEND 224
#define TAB    15

#if PROFANBUILD
char *get_func_color(char *str) {
    // keywords: purple
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return "95";
        }
    }

    // functions: dark cyan
    if (get_function(str) != NULL) {
        return "36";
    }

    // pseudos: blue
    if (get_pseudo(str) != NULL) {
        return "94";
    }

    // internal functions: cyan
    if (get_if_function(str) != NULL) {
        return "96";
    }

    // unknown functions: dark red
    return "31";
}

void olv_print(char *str, int len) {
    /* colored print
     * function: cyan
     * keywords: purple
     * unknown function: red
     * variable: yellow
     * brackets: green
     * comments: dark green
    **/

    if (len == 0) {
        return;
    }

    int is_var = 0;
    int i = 0;

    int dec = 0;
    while (str[i] == ' ' && i != len) {
        dec++;
        i++;
    }

    if (str[i] == '/' && str[i + 1] == '/') {
        fputs("\e[32m", stdout);
        for (i = 0; str[i] != ';'; i++) {
            if (i == len) return;
            putchar(str[i]);
        }
        olv_print(str + i, len - i);
        return;
    }

    char *tmp = malloc((len + 1) * sizeof(char));
    while (!(str[i] == '!' || str[i] == ' '  || str[i] == ';') && i != len) {
        i++;
    }

    memcpy(tmp, str, i);
    tmp[i] = '\0';
    printf("\e[%sm%s", get_func_color(tmp + dec), tmp);

    int from = i;
    for (; i < len; i++) {
        if (i < len - 1 && str[i] == '/' && str[i+1] == '/') {
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                printf("\e[9%cm%s", is_var ? '9' : '7', tmp);
            }
            olv_print(str + i, len - i);
            free(tmp);
            return;
        }

        if (str[i] == '!' && str[i + 1] == '(') {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                printf("\e[9%cm%s", is_var ? '9' : '7', tmp);
            }

            // find the closing bracket
            int j, open = 0;
            for (j = i + 1; j < len; j++) {
                if (str[j] == '(') {
                    open++;
                } else if (str[j] == ')') {
                    open--;
                }

                if (open == 0) {
                    break;
                }
            }

            printf("\e[9%cm!(", (j == len) ? '1' : '2');
            olv_print(str + i + 2, j - i - 2);

            if (j != len) {
                fputs("\e[92m)", stdout);
            }

            i = j;
            from = i + 1;
        }

        else if (str[i] == ';') {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                printf("\e[9%cm%s", is_var ? '3' : '7', tmp);
                from = i;
            }
            fputs("\e[37m;\e[0m", stdout);
            olv_print(str + i + 1, len - i - 1);
            free(tmp);
            return;
        }

        else if (str[i] == '|') {
            if (!(i && (str[i + 1] == str[i - 1] && (str[i + 1] == ' ' || str[i + 1] == STRING_CHAR))) &&
                !(i == len - 1 && i && str[i - 1] == ' ')
            ) continue;

            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                printf("\e[9%cm%s", is_var ? '3' : '7', tmp);
                from = i;
            }

            fputs("\e[37m|\e[0m", stdout);
            if (str[i + 1] == STRING_CHAR) {
                putchar(STRING_CHAR);
                i++;
            }
            olv_print(str + i + 1, len - i - 1);
            free(tmp);
            return;
        }

        // variable
        else if (str[i] == '!') {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                printf("\e[9%cm%s", is_var ? '3' : '7', tmp);
                from = i;
            }
            is_var = 1;
        }

        else if (str[i] == ' ' || str[i] == STRING_CHAR) {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                printf("\e[9%cm%s", is_var ? '3' : '7', tmp);
                from = i;
            }
            is_var = 0;
        }
    }

    if (from != i) {
        memcpy(tmp, str + from, i - from);
        tmp[i - from] = '\0';
        printf("\e[9%cm%s", is_var ? '3' : '7', tmp);
    }

    free(tmp);
}

int add_to_suggest(char **suggests, int count, char *str) {
    if (count < MAX_SUGGESTS) {
        suggests[count] = strdup(str);
        count++;
        suggests[count] = NULL;
        return count;
    }
    return count;
}

char *olv_autocomplete(char *str, int len, char **other, int *dec_ptr) {
    other[0] = NULL;

    if (len == 0) {
        return NULL;
    }

    char *ret = NULL;
    char *tmp;

    int suggest = 0;
    int is_var = 0;
    int dec = 0;
    int i = 0;

    *dec_ptr = 0;

    // check for ';' or '!('
    for (int j = 0; j < len; j++) {
        if (str[j] == '!' && str[j + 1] == '(') {
            dec = j + 2;
            i = dec;
            j++;
        } else if (str[j] == ';') {
            dec = j + 1;
            i = dec;
        }
    }

    while (str[i] == ' ' && i != len) {
        dec++;
        i++;
    }

    // check if we are in the middle of a variable
    int in_var = 0;
    for (int j = i; j < len; j++) {
        if (str[j] == '!') {
            in_var = j + 1;
        } else if (str[j] == ' ' || str[j] == STRING_CHAR) {
            in_var = 0;
        }
    }

    if (in_var) i++;

    while (!(str[i] == '!' || str[i] == ' ' || str[i] == STRING_CHAR) && i != len) {
        i++;
    }

    if (i - dec == 0) {
        return NULL;
    }

    // path
    if (i < len && !in_var) {
        #if PROFANBUILD
        // ls the current directory
        char *path = malloc(MAX_PATH_SIZE * sizeof(char));
        char *inp_end = malloc(MAX_PATH_SIZE * sizeof(char));

        memcpy(inp_end, str + i + 1, len - (i + 1));
        len = len - (i + 1);
        inp_end[len] = '\0';

        // find the last space
        for (int j = len - 1; j >= 0; j--) {
            if (inp_end[j] != ' ') continue;
            // copy to the beginning
            memcpy(inp_end, inp_end + j + 1, len - (j + 1));
            len = len - (j + 1);
            inp_end[len] = '\0';
            break;
        }

        char *tmp = assemble_path(g_current_directory, inp_end);
        strcpy(path, tmp);
        free(tmp);

        if (path[strlen(path) - 1] != '/'
            && (inp_end[strlen(inp_end) - 1] == '/'
            || !inp_end[0])
        ) strcat(path, "/");

        // cut the path at the last '/'
        int last_slash = 0;
        for (int j = strlen(path) - 1; j >= 0; j--) {
            if (path[j] == '/') {
                last_slash = j;
                break;
            }
        }
        strcpy(inp_end, path + last_slash + 1);
        path[last_slash + (last_slash == 0)] = '\0';

        dec = strlen(inp_end);
        *dec_ptr = dec;

        // check if the path is valid
        sid_t dir = fu_path_to_sid(ROOT_SID, path);
        free(path);

        if (IS_NULL_SID(dir) || !fu_is_dir(dir)) {
            free(inp_end);
            return NULL;
        }

        // get the directory content
        char **names;
        sid_t *out_ids;

        int elm_count = fu_get_dir_content(dir, &out_ids, &names);

        for (int j = 0; j < elm_count; j++) {
            if (names[j][0] == '.' && inp_end[0] != '.') continue;
            if (strncmp(names[j], inp_end, dec) == 0) {
                tmp = malloc((strlen(names[j]) + 2) * sizeof(char));
                strcpy(tmp, names[j]);
                if (fu_is_dir(out_ids[j]))
                    strcat(tmp, "/");
                suggest = add_to_suggest(other, suggest, tmp);
                free(tmp);
            }
        }

        for (int j = 0; j < elm_count; j++) {
            free(names[j]);
        }

        free(inp_end);
        free(out_ids);
        free(names);

        if (suggest == 1) {
            ret = strdup(other[0] + dec);
            free(other[0]);
            return ret;
        }

        #endif

        return NULL;
    }

    tmp = malloc((len + 1) * sizeof(char));

    // variables
    if (in_var) {
        int size = len - in_var;
        *dec_ptr = size;

        if (size != 0) {
            memcpy(tmp, str + in_var, size);
            tmp[size] = '\0';
        }

        for (int j = 0; variables[j].name != NULL; j++) {
            if (!size || strncmp(tmp, variables[j].name, size) == 0) {
                suggest = add_to_suggest(other, suggest, variables[j].name);
            }
        }

        free(tmp);

        if (suggest == 1) {
            ret = strdup(other[0] + size);
            free(other[0]);
        }

        return ret;
    }

    memcpy(tmp, str + dec, i - dec);
    tmp[i] = '\0';

    *dec_ptr = i - dec;

    // keywords
    for (int j = 0; keywords[j] != NULL; j++) {
        if (strncmp(tmp, keywords[j], i - dec) == 0) {
            suggest = add_to_suggest(other, suggest, keywords[j]);
        }
    }

    // pseudos
    for (int j = 0; pseudos[j].name != NULL; j++) {
        if (strncmp(tmp, pseudos[j].name, i - dec) == 0) {
            suggest = add_to_suggest(other, suggest, pseudos[j].name);
        }
    }

    // internal functions
    for (int j = 0; internal_functions[j].name != NULL; j++) {
        if (strncmp(tmp, internal_functions[j].name, i - dec) == 0) {
            suggest = add_to_suggest(other, suggest, internal_functions[j].name);
        }
    }

    // functions
    for (int j = 0; functions[j].name != NULL; j++) {
        if (strncmp(tmp, functions[j].name, i - dec) == 0) {
            suggest = add_to_suggest(other, suggest, functions[j].name);
        }
    }

    free(tmp);

    if (suggest == 1) {
        ret = strdup(other[0] + i - dec);
        free(other[0]);
    }

    return ret;
}

#endif


int local_input(char *buffer, int size, char **history, int history_end, int buffer_index) {
    // return -1 if the input is valid, else return the cursor position
    #if PROFANBUILD

    history_end++;

    // save the current cursor position and show it
    fputs("\e[s\e[?25l", stdout);

    int sc, last_sc = 0, last_sc_sgt = 0;

    int buffer_actual_size = strlen(buffer);
    if (buffer_index == -1) buffer_index = buffer_actual_size;

    if (buffer_actual_size) {
        olv_print(buffer, buffer_actual_size);
        printf(" \e[0m\e[u\e[%dC\e[?25l", buffer_index);
    }

    fflush(stdout);

    int key_ticks = 0;
    int shift = 0;

    int history_index = history_end;

    char **other_suggests = malloc((MAX_SUGGESTS + 1) * sizeof(char *));
    int ret_val = -1;

    sc = 0;
    while (sc != ENTER) {
        usleep(SLEEP_T * 1000);
        sc = c_kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        key_ticks = (sc != last_sc) ? 0 : key_ticks + 1;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            continue;
        }

        if (sc == LSHIFT || sc == RSHIFT) {
            shift = 1;
            continue;
        }

        if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift = 0;
            continue;
        }

        if (sc == LEFT) {
            if (!buffer_index) continue;
            buffer_index--;
        }

        else if (sc == RIGHT) {
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
        }

        else if (sc == BACK) {
            if (!buffer_index) continue;
            buffer_index--;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == DEL) {
            if (buffer_index == buffer_actual_size) continue;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == OLDER) {
            // read from history
            int old_index = history_index;
            history_index = (history_index - 1);
            if (history_index < 0) history_index += HISTORY_SIZE;
            if (history[history_index] == NULL || history_index == history_end) {
                history_index = old_index;
            } else {
                fputs("\e[u\e[K", stdout);
                strcpy(buffer, history[history_index]);
                buffer_actual_size = strlen(buffer);
                buffer_index = buffer_actual_size;
            }
        }

        else if (sc == NEWER) {
            // read from history
            int old_index = history_index;
            if (history[history_index] == NULL || history_index == history_end) continue;
            history_index = (history_index + 1) % HISTORY_SIZE;
            fputs("\e[u\e[K", stdout);
            if (history[history_index] == NULL || history_index == history_end) {
                buffer[0] = '\0';
                buffer_actual_size = 0;
                buffer_index = 0;
            } else {
                strcpy(buffer, history[history_index]);
                buffer_actual_size = strlen(buffer);
                buffer_index = buffer_actual_size;
            }
        }

        else if (sc == TAB) {
            int dec;
            char *suggestion = olv_autocomplete(buffer, buffer_index, other_suggests, &dec);
            if (suggestion == NULL && other_suggests[0] != NULL) {
                // print other suggestions
                char *common_beginning = strdup(other_suggests[0] + dec);
                putchar('\n');

                for (int i = 0; other_suggests[i] != NULL; i++) {
                    printf("%s   ", other_suggests[i]);
                    for (int j = 0;; j++) {
                        if (other_suggests[i][j + dec] != common_beginning[j]) {
                            common_beginning[j] = '\0';
                            break;
                        }
                        if (other_suggests[i][j + dec] == '\0') {
                            break;
                        }
                    }
                    free(other_suggests[i]);
                }

                // add the common beginning to the buffer
                int common_len = strlen(common_beginning);
                if (buffer_actual_size + common_len >= size) {
                    ret_val = buffer_index;
                    break;
                }

                for (int i = buffer_actual_size; i >= buffer_index; i--) {
                    buffer[i + common_len] = buffer[i];
                }
                for (int i = 0; i < common_len; i++) {
                    buffer[buffer_index + i] = common_beginning[i];
                }
                buffer_actual_size += common_len;
                buffer_index += common_len;
                free(common_beginning);

                ret_val = buffer_index;
                break;
            }

            if (suggestion == NULL) {
                continue;
            }

            int suggestion_len = strlen(suggestion);
            if (buffer_actual_size + suggestion_len >= size) continue;
            for (int i = buffer_actual_size; i >= buffer_index; i--) {
                buffer[i + suggestion_len] = buffer[i];
            }
            for (int i = 0; i < suggestion_len; i++) {
                buffer[buffer_index + i] = suggestion[i];
            }
            buffer_actual_size += suggestion_len;
            buffer_index += suggestion_len;
            free(suggestion);
        }

        else if (sc <= SC_MAX) {
            if (size < buffer_actual_size + 2) continue;
            if (profan_kb_get_char(sc, shift) == '\0') continue;
            for (int i = buffer_actual_size; i > buffer_index; i--) {
                buffer[i] = buffer[i - 1];
            }
            buffer[buffer_index] = profan_kb_get_char(sc, shift);
            buffer_actual_size++;
            buffer_index++;
        }

        else continue;

        fputs("\e[?25h\e[u", stdout);
        olv_print(buffer, buffer_actual_size);
        printf(" \e[0m\e[u\e[%dC\e[?25l", buffer_index);
        fflush(stdout);
    }

    free(other_suggests);

    buffer[buffer_actual_size] = '\0';

    puts("\e[?25h");

    return ret_val;

    #else

    if (!fgets(buffer, size, stdin)) {
        puts("");
        exit(0);
    }

    return -1;
    #endif
}


/***************************
 *                        *
 *  Shell/File functions  *
 *                        *
***************************/

void display_prompt(void) {
    for (int i = 0; g_prompt[i] != '\0'; i++) {
        if (g_prompt[i] != '$') {
            putchar(g_prompt[i]);
            continue;
        }
        switch (g_prompt[i + 1]) {
            case 'v':
                fputs(OLV_VERSION, stdout);
                break;
            case 'd':
                fputs(g_current_directory, stdout);
                break;
            case '(':
                if (g_exit_code[0] != '0') break;
                for (; g_prompt[i] != ')'; i++);
                i--;
                break;
            case '{':
                if (g_exit_code[0] == '0') break;
                for (; g_prompt[i] != '}'; i++);
                i--;
                break;
            case ')':
                break;
            case '}':
                break;
            default:
                putchar('$');
                break;
        }
        i++;
    }
    fflush(stdout);
}

void start_shell(void) {
    // use execute_program() to create a shell
    char *line = malloc(INPUT_SIZE * sizeof(char));

    char **history = calloc(HISTORY_SIZE, sizeof(char *));
    int history_index = 0;
    int cursor_pos, len;
    cursor_pos = -1;

    while (1) {
        line[0] = '\0';
        do {
            display_prompt();
            cursor_pos = local_input(line, INPUT_SIZE, history, history_index, cursor_pos);
        } while (cursor_pos != -1);

        if (strcmp(line, "shellexit") == 0) {
            puts("Exiting olivine shell, bye!");
            break;
        }

        while (does_syntax_fail(line)) {
            // multiline program
            strcat(line, ";");
            len = strlen(line);
            line[len] = '\0';
            do {
                fputs(OTHER_PROMPT, stdout);
                fflush(stdout);
                cursor_pos = local_input(line + len, INPUT_SIZE - len, history, history_index, cursor_pos);
            } while(cursor_pos != -1);
        }

        // add to history if not empty and not the same as the last command
        if (line[0] && (history[history_index] == NULL || strcmp(line, history[history_index]))) {
            history_index = (history_index + 1) % HISTORY_SIZE;

            if (history[history_index] != NULL) {
                free(history[history_index]);
            }

            history[history_index] = strdup(line);
        }

        execute_program(line);
    }

    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }
    free(history);
    free(line);
}

void execute_file(char *file) {
    #if PROFANBUILD

    char *path = assemble_path(g_current_directory, file);

    sid_t id = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(id) || !fu_is_file(id)) {
        printf("file '%s' does not exist\n", path);
        free(path);
        return;
    }

    int file_size = fu_get_file_size(id);
    char *contents = malloc((file_size + 1) * sizeof(char));
    contents[file_size] = '\0';

    fu_file_read(id, (uint8_t *) contents, 0, file_size);

    execute_program(contents);

    free(contents);
    free(path);

    #else

    FILE *f = fopen(file, "r");
    if (f == NULL) {
        printf("file '%s' does not exist\n", file);
        return;
    }

    char *line = malloc(INPUT_SIZE * sizeof(char));
    char *program = malloc(sizeof(char));
    program[0] = '\0';

    while (fgets(line, INPUT_SIZE, f) != NULL) {
        // realloc program
        int len = strlen(line);
        program = realloc(program, (strlen(program) + len + 1) * sizeof(char));
        strcat(program, line);
    }

    fclose(f);
    free(line);

    execute_program(program);

    free(program);

    #endif
}

/************************
 *                     *
 *   Argument Parser   *
 *                     *
************************/

typedef struct {
    int help;
    int version;
    int no_init;
    int inter;

    char *file;
    char *command;
} olivine_args_t;

int show_help(int full, char *name) {
    printf("Usage: %s [options] [file]\n", name);
    if (!full) {
        printf("Try '%s --help' for more information.\n", name);
        return 1;
    }
    puts("Options:\n"
        "  -c, --command  execute argument as code line\n"
        "  -i, --inter    start a shell after executing\n\n"
        "  -h, --help     show this help message and exit\n"
        "  -n, --no-init  don't execute the init program\n\n"
        "  -v, --version  show program's version number\n"

        "Without file, the program will start a shell.\n"
        "Use 'exec' to execute a file from the shell instead\n"
        "of relaunching a new instance of olivine."
    );
    return 0;
}

void show_version(void) {
    printf("Olivine %s, profanOS feature %s\n", OLV_VERSION, PROFANBUILD ? "enabled" : "disabled");
}

olivine_args_t *parse_args(int argc, char **argv) {
    olivine_args_t *args = malloc(sizeof(olivine_args_t));
    args->help = 0;
    args->version = 0;
    args->no_init = 0;
    args->file = NULL;
    args->command = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "--help") == 0
        ) {
            args->help = 2;
        } else if (strcmp(argv[i], "-v") == 0
            || strcmp(argv[i], "--version") == 0
        ) {
            args->version = 1;
        } else if (strcmp(argv[i], "-n") == 0
            || strcmp(argv[i], "--no-init") == 0
        ) {
            args->no_init = 1;
        } else if (strcmp(argv[i], "-i") == 0
            || strcmp(argv[i], "--inter") == 0
        ) {
            args->inter = 1;
        } else if (strcmp(argv[i], "-c") == 0
            || strcmp(argv[i], "--command") == 0
        ) {
            if (i + 1 == argc) {
                puts("Error: no command given");
                args->help = 1;
                return args;
            }
            args->command = argv[i + 1];
            i++;
        } else if (argv[i][0] != '-') {
            args->file = argv[i];
        } else {
            printf("Error: unknown option '%s'\n", argv[i]);
            args->help = 1;
            return args;
        }
    }

    return args;
}


/********************
 *                 *
 *  Main Function  *
 *                 *
********************/

char init_prog[] =
"IF !profan;"
" exec '/zada/olivine/init.olv';"
"END";


int main(int argc, char **argv) {
    olivine_args_t *args = parse_args(argc, argv);
    int ret_val = 0;

    if (args == NULL) {
        return 1;
    }

    if (args->help) {
        ret_val = show_help(args->help == 2, argv[0]);
        free(args);
        return ret_val;
    }

    if (args->version) {
        show_version();
        free(args);
        return 0;
    }

    g_current_level = 0;

    g_current_directory = malloc((MAX_PATH_SIZE + 1) * sizeof(char));
    strcpy(g_current_directory, "/");
    if (USE_ENVVARS) setenv("PWD", "/", 1);

    g_exit_code = malloc(5 * sizeof(char));
    strcpy(g_exit_code, "0");

    g_prompt = malloc((PROMPT_SIZE + 1) * sizeof(char));
    strcpy(g_prompt, DEFAULT_PROMPT);

    variables = calloc(MAX_VARIABLES, sizeof(variable_t));
    pseudos   = calloc(MAX_PSEUDOS, sizeof(pseudo_t));
    functions = calloc(MAX_FUNCTIONS, sizeof(function_t));

    set_variable("version", OLV_VERSION);
    set_variable("profan", PROFANBUILD ? "1" : "0");
    set_variable("spi", "0");

    set_sync_variable("path", g_current_directory, MAX_PATH_SIZE);
    set_sync_variable("exit", g_exit_code, 4);
    set_sync_variable("prompt", g_prompt, PROMPT_SIZE);

    // init pseudo commands
    if (!args->no_init) {
        execute_program(init_prog);
    }

    if (args->file != NULL) {
        execute_file(args->file);
    }

    if (args->command != NULL) {
        execute_program(args->command);
    }

    if (args->inter || (args->file == NULL && args->command == NULL)) {
        start_shell();
    }

    ret_val = atoi(g_exit_code);

    free_functions();
    free_pseudos();
    free_vars();

    free(g_current_directory);
    free(g_exit_code);
    free(g_prompt);
    free(args);

    return ret_val;
}
