/*****************************************************************************\
|   === olivine.c : 2024 ===                                                  |
|                                                                             |
|    Command line shell programming language (wiki/olivine)        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define OLV_VERSION "1.5 rev 6"

#define PROFANBUILD   1  // enable profan features
#define UNIXBUILD     0  // enable unix features

#define USE_READLINE  1  // readline for input (unix only)
#define BIN_AS_PSEUDO 1  // check for binaries in path
#define USE_ENVVARS   1  // enable environment variables
#define ENABLE_DEBUG  0  // print function calls
#define STOP_ON_ERROR 0  // stop after first error
#define ENABLE_WILDC  1  // enable wildcard expansion

#define USER_QUOTE '\''  // quote character
#define USER_VARDF '!'   // variable & subfunc character
#define USER_WILDC '*'   // wildcard character

#define HISTORY_SIZE  100
#define INPUT_SIZE    2048
#define PROMPT_SIZE   100
#define MAX_PATH_SIZE 512
#define MAX_SUGGESTS  10

#define MAX_VARIABLES 100
#define MAX_PSEUDOS   100
#define MAX_FUNCTIONS 100

/******************************
 *                           *
 *  Structs and Definitions  *
 *                           *
******************************/

#ifdef olvUnix
  #undef PROFANBUILD
  #undef UNIXBUILD
  #define PROFANBUILD 0
  #define UNIXBUILD   1
#endif

#if PROFANBUILD
  #include <profan/syscall.h>
  #include <profan/filesys.h>
  #include <profan.h>

  #include <sys/wait.h>
  #include <unistd.h>
  #include <fcntl.h>

  #define DEFAULT_PROMPT "\e[0mprofanOS [\e[95m$d\e[0m] $(\e[31m$)>$(\e[0m$) "
#elif UNIXBUILD
  #include <sys/time.h> // if_ticks
  #include <sys/wait.h> // waitpid
  #include <unistd.h>
  #include <fcntl.h>    // open

  #define DEFAULT_PROMPT "\e[0molivine [\e[95m$d\e[0m] $(\e[31m$)>$(\e[0m$) "
#else
  #define DEFAULT_PROMPT "olivine ${>$}$(x$) "
#endif

#if !UNIXBUILD
  #undef USE_READLINE
  #define USE_READLINE 0
#endif

#if !PROFANBUILD && !UNIXBUILD
  #undef BIN_AS_PSEUDO
  #define BIN_AS_PSEUDO 0
  #undef ENABLE_WILDC
  #define ENABLE_WILDC 0
#endif

#if ENABLE_WILDC
  #include <dirent.h>
#endif

#if USE_READLINE
  #include <readline/readline.h>
  #include <readline/history.h>
  #undef  DEFAULT_PROMPT
  #define DEFAULT_PROMPT "olivine [\1\e[95m\2$d\1\e[0m\2] $(\1\e[31m\2$)>$(\1\e[0m\2$) "
#endif

#define DEBUG_COLOR  "\e[90m"

#define OTHER_PROMPT "> "

#if __CHAR_BIT__ != 8
  #error "olivine requires 8-bit bytes"
#endif

#if PROFANBUILD && UNIXBUILD
  #error "Cannot build with both PROFANBUILD and MINIBUILD"
#endif

#if !PROFANBUILD && !UNIXBUILD
  #undef BIN_AS_PSEUDO
  #define BIN_AS_PSEUDO 0
#endif

#define ERROR_CODE ((void *) 1)

#define INTR_QUOTE '\1'
#define INTR_VARDF '\2'
#define INTR_WILDC '\3'

#define INTR_QUOTE_STR "\1"
#define INTR_VARDF_STR "\2"
#define INTR_WILDC_STR "\3"

#undef  LOWERCASE
#define LOWERCASE(x) ((x) >= 'A' && (x) <= 'Z' ? (x) + 32 : (x))

#undef  min
#define min(a, b) ((a) < (b) ? (a) : (b))

#undef  UNUSED
#define UNUSED(x) (void)(x)

#ifndef INT_MAX
  #define INT_MAX 2147483647
#endif

#define IS_NAME_CHAR(x) (         \
    ((x) >= 'a' && (x) <= 'z') || \
    ((x) >= 'A' && (x) <= 'Z') || \
    ((x) >= '0' && (x) <= '9') || \
    ((x) == '_'))

#define IS_SPACE_CHAR(x) (        \
    (x) == ' ' || (x) == '\t'  || \
    (x) == '\n' || (x) == '\r' || \
    (x) == '\v' || (x) == '\f')

char *keywords[] = {
    "END",
    "IF",
    "ELSE",
    "FOR",
    "WHILE",
    "FUNC",
    "RETURN",
    "BREAK",
    "CONTINUE",
    NULL
};

typedef struct {
    const char *str;
    int fileline;
} olv_line_t;

typedef struct {
    char *name;
    char *(*function)(char **);
} internal_function_t;

typedef struct {
    char *name;
    char *value;
    int is_sync;
    int level;
} variable_t;

typedef struct {
    char *name;
    char *value;
} pseudo_t;

typedef struct {
    char *name;
    olv_line_t *lines;
    int line_count;
} function_t;

variable_t *variables;
pseudo_t *pseudos;
function_t *functions = NULL;
internal_function_t internal_functions[];
char **bin_names, *g_current_directory;

char *g_prompt, g_exit_code[5];
int g_current_level, g_fileline;

olv_line_t *lexe_program(const char *program, int make_internal, int *len);
int execute_file(const char *file, char **args);

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

int local_atoi(const char *str, int *result) {
    int sign, found, base = 0;
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
        for (int i = 0; str[i] != '\0'; i++) {
            res *= 10;
            if (str[i] < '0' || str[i] > '9') {
                return 1;
            }
            res += str[i] - '0';
        }
    }

    for (int i = 0; base && str[i] != '\0'; i++) {
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

#define raise_error(part, format, ...) raise_error_line(g_fileline, part, format, ##__VA_ARGS__)

void raise_error_line(int fileline, const char *part, char *format, ...) {
    if (fileline < 0) {
        if (part) fprintf(stderr, "OLIVINE: %s: ", part);
        else fputs("OLIVINE: ", stderr);
    } else {
        if (part) fprintf(stderr, "OLIVINE l%d: %s: ", fileline, part);
        else fprintf(stderr, "OLIVINE l%d: ", fileline);
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputs("\n", stderr);

    strcpy(g_exit_code, "1");
}

void print_internal_string(const char *str, FILE *file) {
    for (int i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '\n':
                fputs("\\n", file);
                break;
            case '\t':
                fputs("\\t", file);
                break;
            case '\r':
                fputs("\\r", file);
                break;
            case INTR_QUOTE:
                fputc(USER_QUOTE, file);
                break;
            case INTR_VARDF:
                fputc(USER_VARDF, file);
                break;
            case INTR_WILDC:
                fputc(USER_WILDC, file);
                break;
            default:
                fputc(str[i], file);
        }
    }
}

int local_strncmp_nocase(const char *str1, const char *str2, int n) {
    for (int i = 0; i < n || n < 0; i++) {
        if (str1[i] == '\0' || str2[i] == '\0') {
            return str1[i] - str2[i];
        }
        if (LOWERCASE(str1[i]) != LOWERCASE(str2[i])) {
            return str1[i] - str2[i];
        }
    }
    return 0;
}

int is_valid_name(const char *name) {
    if (name == NULL || name[0] == '\0') {
        return 0;
    }

    for (int i = 0; keywords[i] != NULL; i++) {
        if (local_strncmp_nocase(name, keywords[i], -1) == 0) {
            return 0;
        }
    }

    for (int i = 0; name[i] != '\0'; i++) {
        if (!IS_NAME_CHAR(name[i])) {
            return 0;
        }
    }

    return 1;
}

/*******************************
 *                            *
 * Variable Get/Set Functions *
 *                            *
********************************/

int get_variable_index(const char *name, int allow_sublvl) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL)
            break;
        if (strcmp(variables[i].name, name) == 0 && (
            variables[i].level == g_current_level   ||
            variables[i].level == -1                ||
            (allow_sublvl && variables[i].level < g_current_level)
        )) return i;
    }
    return -1;
}

char *get_variable(const char *name) {
    int index = get_variable_index(name, 0);

    if (index != -1) {
        return variables[index].value;
    }

    #if USE_ENVVARS
    return getenv(name);
    #else
    return NULL;
    #endif
}

#define set_variable(name, value) set_variable_withlen(name, value, -1, 0)
#define set_variable_global(name, value) set_variable_withlen(name, value, -1, 1)

int set_variable_withlen(const char *name, const char *value, int str_len, int is_global) {
    char *value_copy = NULL;
    int index = get_variable_index(name, is_global);

    if (str_len == -1)
        str_len = strlen(value);

    // strndup the value
    if (index == -1 || (index != -1 && !variables[index].is_sync)) {
        value_copy = malloc(str_len + 1);
        strncpy(value_copy, value, str_len);
        value_copy[str_len] = '\0';
    }

    if (index != -1) {
        if (!variables[index].is_sync) {
            if (variables[index].value)
                free(variables[index].value);
            if (is_global)
                variables[index].level = -1;
            variables[index].value = value_copy;
            return 0;
        }
        int len = min(str_len, variables[index].is_sync);
        strncpy(variables[index].value, value, len);
        variables[index].value[len] = '\0';
        return 0;
    }

    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            char *name_copy = malloc(strlen(name) + 1);
            strcpy(name_copy, name);
            variables[i].name = name_copy;
            variables[i].value = value_copy;
            variables[i].level = is_global ? -1 : g_current_level;
            variables[i].is_sync = 0;
            return 0;
        }
    }

    free(value_copy);

    raise_error(NULL, "Cannot set value of '%s', more than %d variables", name, MAX_VARIABLES);
    return 1;
}

int set_sync_variable(char *name, char *value, int max_len) {
    int index = get_variable_index(name, 1);

    if (index != -1) {
        if (variables[index].value && (!variables[index].is_sync)) {
            free(variables[index].value);
        }
        variables[index].level = -1;
        variables[index].value = value;
        variables[index].is_sync = max_len;
        return 0;
    }

    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].name == NULL) {
            variables[i].name = name;
            variables[i].value = value;
            variables[i].level = -1;
            variables[i].is_sync = max_len;
            return 0;
        }
    }

    raise_error(NULL, "Cannot set value of '%s', more than %d variables", name, MAX_VARIABLES);
    return 1;
}

int del_variable(const char *name) {
    int index = get_variable_index(name, 0);

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

char *get_pseudo(const char *name) {
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

int set_pseudo(const char *name, const char *value) {
    for (int i = 0; i < MAX_PSEUDOS; i++) {
        if (pseudos[i].name == NULL) {
            pseudos[i].name  = strdup(name);
            pseudos[i].value = strdup(value);
            return 0;
        }
        if (strcmp(pseudos[i].name, name) == 0) {
            if (pseudos[i].value)
                free(pseudos[i].value);
            pseudos[i].value = strdup(value);
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

int del_function(const char *name) {
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

int set_function(const char *name, olv_line_t *lines, int line_count) {
    int char_count = 0;
    for (int i = 0; i < line_count; i++) {
        char_count += strlen(lines[i].str) + 1;
    }

    olv_line_t *lines_copy = malloc(sizeof(olv_line_t) * line_count + char_count);
    char *lines_ptr = (char *) lines_copy + sizeof(olv_line_t) * line_count;

    for (int i = 0; i < line_count; i++) {
        lines_copy[i].fileline = lines[i].fileline;
        lines_copy[i].str = lines_ptr;

        strcpy(lines_ptr, lines[i].str);
        lines_ptr += strlen(lines_ptr) + 1;
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

function_t *get_function(const char *name) {
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

/*******************************
 *                            *
 *   bin Get/load Functions   *
 *                            *
*******************************/

char **load_bin_names(void) {
    #if BIN_AS_PSEUDO && PROFANBUILD && USE_ENVVARS
    uint32_t size = 0;
    int bin_count = 0;

    char *path = getenv("PATH");
    if (path == NULL) {
        return NULL;
    }

    char *path_copy = strdup(path);
    char *path_ptr = path_copy;
    char *path_end = path_ptr;

    uint32_t *cnt_ids;
    char **cnt_names;

    char **tmp_names = NULL;

    while (path_ptr != NULL) {
        path_end = strchr(path_ptr, ':');

        if (path_end != NULL) {
            *path_end = '\0';
        }

        uint32_t dir_id = fu_path_to_sid(SID_ROOT, path_ptr);
        if (IS_SID_NULL(dir_id) || !fu_is_dir(dir_id))
            goto next;

        int elm_count = fu_get_dir_content(dir_id, &cnt_ids, &cnt_names);
        if (!elm_count)
            goto next;

        tmp_names = realloc(tmp_names, sizeof(char *) * (bin_count + elm_count));

        for (int i = 0; i < elm_count; i++) {
            if (fu_is_file(cnt_ids[i]) && strstr(cnt_names[i], ".elf")) {
                size += strlen(cnt_names[i]) - 3;
                tmp_names[bin_count++] = cnt_names[i];
            } else {
                profan_kfree(cnt_names[i]);
            }
        }
        profan_kfree(cnt_names);
        profan_kfree(cnt_ids);

        next:
        if (path_end != NULL) {
            *path_end = ':';
            path_ptr = path_end + 1;
        } else {
            path_ptr = NULL;
        }
    }

    size += sizeof(char *) * (bin_count + 1);
    char **ret = malloc(size);
    char *ret_ptr = (char *) ret + sizeof(char *) * (bin_count + 1);

    for (int i = 0; i < bin_count; i++) {
        int tmp = strlen(tmp_names[i]) - 4;
        ret[i] = ret_ptr;
        strncpy(ret_ptr, tmp_names[i], tmp);
        ret_ptr[tmp] = '\0';
        ret_ptr += tmp + 1;
        profan_kfree(tmp_names[i]);
    }

    if (size != (uint32_t) ret_ptr - (uint32_t) ret) {
        raise_error(NULL, "Error while loading bin names");
        free(tmp_names);
        free(path_copy);
        free(ret);
        return NULL;
    }

    ret[bin_count] = NULL;
    free(tmp_names);
    free(path_copy);

    return ret;
    #else
    return NULL;
    #endif
}

int in_bin_names(const char *name) {
    if (bin_names == NULL) {
        return 0;
    }

    for (int i = 0; bin_names[i] != NULL; i++) {
        if (strcmp(bin_names[i], name) == 0) {
            return 1;
        }
    }

    return 0;
}

char *get_bin_path(const char *name) {
    #if BIN_AS_PSEUDO && (PROFANBUILD || UNIXBUILD) && USE_ENVVARS
    char *src_path = getenv("PATH");
    if (!src_path)
        return NULL;
    char *path = strdup(src_path);
    #if PROFANBUILD
    char *fullname = malloc(strlen(name) + 5); // 5 => .elf + null
    strcpy(fullname, name);
    strcat(fullname, ".elf");

    int start = 0;
    for (int i = 0;; i++) {
        if (path[i] != ':' && path[i] != '\0')
            continue;
        path[i] = '\0';
        uint32_t sid = fu_path_to_sid(SID_ROOT, path + start);
        if (!IS_SID_NULL(sid) && fu_is_file(fu_path_to_sid(sid, fullname))) {
            char *result = profan_join_path(path + start, fullname);
            free(fullname);
            free(path);
            return result;
        }
        if (src_path[i] == '\0')
            break;
        start = i + 1;
    }

    free(fullname);

    #elif UNIXBUILD
    char *path_copy = path;
    char *path_end = path;

    while (path_copy != NULL) {
        path_end = strchr(path_copy, ':');

        if (path_end != NULL) {
            *path_end = '\0';
        }

        char *result = malloc(strlen(path_copy) + strlen(name) + 2);
        strcpy(result, path_copy);
        strcat(result, "/");
        strcat(result, name);

        if (access(result, F_OK) == 0) {
            free(path);
            return result;
        }

        free(result);

        if (path_end != NULL) {
            *path_end = ':';
            path_copy = path_end + 1;
        } else {
            path_copy = NULL;
        }
    }

    #endif
    free(path);
    return NULL;
    #endif // BIN_AS_PSEUDO
    UNUSED(name);
    return NULL;
}

/*******************************
 *                            *
 *    Wildcards Resolution    *
 *                            *
*******************************/

#if ENABLE_WILDC

int is_wildcard_match(char *wildcard, char *string) {
    if (*wildcard == '\0')
        return *string == '\0';

    if (*wildcard == *string)
        return is_wildcard_match(wildcard + 1, string + 1);

    if (*wildcard != INTR_WILDC)
        return 0;

    return is_wildcard_match(wildcard + 1, string) ||
            (*string && is_wildcard_match(wildcard, string + 1));
}

char *resolve_wildcard(char *base, char *wildcard) {
    if (*wildcard == '/' && *base == '\0') {
        wildcard++;
        base = "/";
    }

    char *next = strchr(wildcard, '/');

    if (next)
        wildcard[next - wildcard] = '\0';

    DIR *dir = opendir(*base ? base : ".");
    if (dir == NULL)
        return NULL;

    char *output = malloc(1);
    output[0] = '\0';

    for (struct dirent *entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
        if (entry->d_name[0] == '.' && wildcard[0] != '.')
            continue;
        if (!is_wildcard_match(wildcard, entry->d_name))
            continue;
        if (!next) {
            output = realloc(output, strlen(output) + strlen(base) + strlen(entry->d_name) + 2);
            strcat(output, base);
            strcat(output, entry->d_name);
            strcat(output, " ");
            continue;
        }
        char *new_base = malloc(strlen(base) + strlen(entry->d_name) + 2);
        strcpy(new_base, base);
        strcat(new_base, entry->d_name);
        strcat(new_base, "/");
        char *new_output = resolve_wildcard(new_base, next + 1);
        free(new_base);
        if (new_output == NULL)
            continue;
        output = realloc(output, strlen(output) + strlen(new_output) + 1);
        strcat(output, new_output);
        free(new_output);
    }

    closedir(dir);

    if (next)
        wildcard[next - wildcard] = '/';

    if (output[0])
        return output;

    free(output);
    return NULL;
}

#endif

/***********************************
 *                                *
 *  Olivine Integrated Evaluator  *
 *                                *
***********************************/

typedef struct {
    void *ptr;
    unsigned char type;
} ast_leaf_t;

typedef struct {
    ast_leaf_t left;
    ast_leaf_t center;
    ast_leaf_t right;
} ast_t;

#define AST_TYPE_AST   0
#define AST_TYPE_NIL   1
#define AST_TYPE_STR   3

#define IS_THIS_OP(str, op) ((str)[0] == (op) && (str)[1] == '\0')

char ops[] = "=~<>@.+-x/%()";

void free_ast(ast_t *ast) {
    if (!ast) return;

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
    if (len > 2 && IS_THIS_OP(str[0], '(')) {
        int i, parenthesis = 1;
        for (i = 1; i < len && parenthesis; i++) {
            if (IS_THIS_OP(str[i], '(')) {
                parenthesis++;
            } else if (IS_THIS_OP(str[i], ')')) {
                parenthesis--;
            }
        }
        if (parenthesis == 0 && i == len) {
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
        if (IS_THIS_OP(str[i], '(')) {
            op_parenthesis++;
            continue;
        }

        if (IS_THIS_OP(str[i], ')')) {
            op_parenthesis--;
            continue;
        }

        if (op_parenthesis) {
            continue;
        }

        for (int j = 0; j < (int) sizeof(ops); j++) {
            if (IS_THIS_OP(str[i], ops[j]) && j < op_priority) {
                op_index = i;
                op_priority = j;
                break;
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

    if (op[0] == '%' && right == 0) {
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
        case 'x':
            result = left * right;
            break;
        case '/':
            result = left / right;
            break;
        case '%':
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
    char *ret = malloc(12);
    local_itoa(result, ret);
    return ret;
}

char *eval_string_copy(char *dest, char *src) {
    int len = strlen(src);

    if (src[0] != '"') {
        raise_error("eval", "String must start with '\"' - got '%s'", src);
        free(dest);
        return NULL;
    }

    if (src[len - 1] != '"') {
        raise_error("eval", "String must end with '\"' - got '%s'", src);
        free(dest);
        return NULL;
    }

    if (dest == NULL) {
        dest = malloc(len + 1);
    }

    strncpy(dest, src + 1, len - 2);
    dest[len - 2] = '\0';

    return dest;
}

char *calculate_strings(char *left, char *right, char *op) {
    char *res, *tmp;
    int len, nb = 0;

    if (local_atoi(right, &nb)) {
        if (local_atoi(left, &nb)) tmp = NULL;
        else tmp = right;
    } else tmp = left;

    switch (op[0]) {
        case '+':
        case '.':
            len = strlen(left);
            res = malloc(len + strlen(right) + 3);
            res[0] = '"';
            strcpy(res + 1, left);
            strcpy(res + len + 1, right);
            strcat(res + len + 1, "\"");
            break;
        case '=':
            res = malloc(2);
            res[0] = (strcmp(left, right) == 0) + '0';
            res[1] = '\0';
            break;
        case '~':
            res = malloc(2);
            res[0] = (strcmp(left, right) != 0) + '0';
            res[1] = '\0';
            break;
        case 'x':
            if (tmp == NULL) {
                raise_error("eval", "Integer expected for string repeat");
                return NULL;
            }
            len = strlen(tmp);
            res = malloc(len * nb + 3);
            res[0] = '"';
            for (int i = 0; i < nb; i++)
                memcpy(res + 1 + i * len, tmp, len);
            res[1 + len * nb] = '"';
            res[2 + len * nb] = '\0';
            break;
        case '@':
            if (tmp == NULL) {
                raise_error("eval", "Integer expected for string character");
                return NULL;
            }
            if (nb < 0 || nb >= (int) strlen(tmp)) {
                raise_error("eval", "Cannot get character %d from string of length %d", nb, strlen(tmp));
                return NULL;
            }
            res = malloc(4);
            res[0] = '"';
            res[1] = tmp[nb];
            res[2] = '"';
            res[3] = '\0';
            break;
        case '>':
            if (tmp == NULL) {
                raise_error("eval", "Integer expected for string shift");
                return NULL;
            }
            len = strlen(tmp);
            if (nb < 0 || nb > len) {
                raise_error("eval", "Cannot shift string by %d", nb);
                return NULL;
            }
            res = malloc(len + 3 - nb);
            res[0] = '"';
            memcpy(res + 1, tmp + nb, len - nb);
            res[len - nb + 1] = '"';
            res[len - nb + 2] = '\0';
            break;
        case '<':
            if (tmp == NULL) {
                raise_error("eval", "Integer expected for string shift");
                return NULL;
            }
            len = strlen(tmp);
            if (nb < 0 || nb > len) {
                raise_error("eval", "Cannot shift string by %d", nb);
                return NULL;
            }
            res = malloc(len + 3 - nb);
            res[0] = '"';
            memcpy(res + 1, tmp, len - nb);
            res[len - nb + 1] = '"';
            res[len - nb + 2] = '\0';
            break;
        default:
            raise_error("eval", "Unknown operator '%s' between strings", op);
            return NULL;
    }
    return res;
}

char *eval(ast_t *ast) {
    int left, right;
    char *res = NULL;

    // if only one element return it
    if (ast->left.type == AST_TYPE_NIL && ast->right.type == AST_TYPE_NIL) {
        return strdup(ast->center.ptr);
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
    char *left_str, *right_str, *s1, *s2;

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
        if (local_atoi(left_str, &left))
            left = INT_MAX;
        if (local_atoi(right_str, &right))
            right = INT_MAX;
        if (left != INT_MAX && right != INT_MAX && !(op[0] == '.' || op[0] == '@'))
            res = calculate_integers(left, right, op);
        else {
            s1 = (left == INT_MAX) ? eval_string_copy(NULL, left_str) : left_str;
            s2 = (right == INT_MAX) ? eval_string_copy(NULL, right_str) : right_str;
            res = (s1 == NULL || s2 == NULL) ? NULL : calculate_strings(s1, s2, op);
            if (left == INT_MAX)
                free(s1);
            if (right == INT_MAX)
                free(s2);
        }
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
        "Usage: eval <arg> [operator <arg>] [...]\n"
        "Spaces are required between operators\n\n"
        "Number operators:\n"
        " +  Addition\n"
        " -  Substraction\n"
        " x  Multiplication\n"
        " /  Division\n"
        " %  Modulo\n"
        " <  Less than\n"
        " >  Greater than\n"
        " =  Equal\n"
        " ~  Not equal\n"
        "String operators:\n"
        " +  Concatenation\n"
        " .  Concatenation (with number)\n"
        " x  Repeat\n"
        " @  Get character\n"
        " =  Equal\n"
        " ~  Not equal\n\n"
        "Operators priority (from lowest to highest):");
    for (unsigned i = 0; ops[i]; i++) {
        printf(" %c", ops[i]);
    }
    puts("\n\nExample: eval 1 + 2 x 3\n"
        "         eval \"hello\" x 3\n"
        "         eval ( 1 + 3 ) x 2 = 8\n");
}

char *if_eval(char **input) {
    // get argc
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc < 1) {
        raise_error("eval", "Too few arguments, try 'eval -h'");
        return ERROR_CODE;
    }

    if (argc == 1 && (
        strcmp(input[0], "-h") == 0 ||
        strcmp(input[0], "--help") == 0
    )) {
        eval_help();
        return NULL;
    }

    ast_t *ast = gen_ast(input, argc);
    char *res = NULL;

    if (ast) {
        res = eval(ast);
        free_ast(ast);
    }

    if (res == NULL || res == ERROR_CODE) {
        return ERROR_CODE;
    }

    if (res[0] == '"') {
        int len = strlen(res);
        memmove(res, res + 1, len - 1);
        res[len - 2] = '\0';
    }

    return res;
}

/**************************************
 *                                   *
 *  Olivine Lang Internal Functions  *
 *                                   *
**************************************/

char *if_cd(char **input) {
    #if (PROFANBUILD || UNIXBUILD) && USE_ENVVARS
    // get argc
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc > 1) {
        raise_error("cd", "Usage: cd [dir]");
        return ERROR_CODE;
    }

    char *dir;

    if (argc == 0) {
        dir = getenv("HOME");
        if (dir == NULL) {
            raise_error("cd", "HOME environment variable not set");
            return ERROR_CODE;
        }
    } else {
        dir = input[0];
    }

    if (chdir(dir) != 0) {
        raise_error("cd", "Directory '%s' does not exist", dir);
        return ERROR_CODE;
    }

    dir = getcwd(NULL, 0);

    if (dir == NULL) {
        raise_error("cd", "Error while getting current directory");
        return ERROR_CODE;
    }

    // change directory
    strcpy(g_current_directory, dir);
    setenv("PWD", g_current_directory, 1);

    free(dir);
    return NULL;

    #else
    UNUSED(input);
    raise_error("cd", "Not supported in this build");
    return ERROR_CODE;
    #endif
}

char *if_debug(char **input) {
    int mode = 0;

    if (input[0] && !input[1]) {
        if (strcmp(input[0], "-v") == 0) {
            mode = 1;
        } else if (strcmp(input[0], "-d") == 0) {
            mode = 2;
        } else if (strcmp(input[0], "-l") == 0) {
            mode = 3;
        } else if (strcmp(input[0], "-h") == 0) {
            mode = 4;
        } else if (strcmp(input[0], "-r") == 0) {
            mode = 5;
        }
    }

    if (mode == 0) {
        raise_error("debug", "Wrong usage, try 'debug -h'");
        return ERROR_CODE;
    }

    // print help
    if (mode == 4) {
        puts("Debug mode help\n"
            "Usage: debug [-d|-l|-h|-r]\n"
            "  -d: dump vars, funcs, pseudos\n"
            "  -h: print this help\n"
            "  -l: save functions to file\n"
            "  -r: reload bin names\n"
            "  -v: dump variables only");
        return NULL;
    }

    if (mode < 3) {
        printf("VARIABLES (max %d):\n", MAX_VARIABLES);
        for (int i = 0; i < MAX_VARIABLES && variables[i].name != NULL; i++) {
            printf("  %s (", variables[i].name);
            if (variables[i].is_sync)
                printf("sync, size: %d", variables[i].is_sync);
            else if (variables[i].level == -1)
                printf("global");
            else
                printf("lvl: %d", variables[i].level);
            printf("): '%s'\n", variables[i].value);
        }
    }

    if (mode == 1) {
        return NULL;
    }

    // dump info
    if (mode == 2) {
        puts("INTERNAL FUNCTIONS");
        for (int i = 0; internal_functions[i].name != NULL; i++) {
            printf("  %s: %p\n", internal_functions[i].name, internal_functions[i].function);
        }

        printf("FUNCTIONS (max %d):\n", MAX_FUNCTIONS);
        for (int i = 0; i < MAX_FUNCTIONS && functions[i].name != NULL; i++) {
            printf("  %s: %d lines (%p)\n", functions[i].name, functions[i].line_count, functions[i].lines);
        }

        printf("PSEUDOS (max %d):\n", MAX_PSEUDOS);
        for (int i = 0; i < MAX_PSEUDOS && pseudos[i].name != NULL; i++) {
            printf("  %s: '%s'\n", pseudos[i].name, pseudos[i].value);
        }

        return NULL;
    }

    if (mode == 5) {
        free(bin_names);
        bin_names = load_bin_names();

        // count bin names
        int c;
        for (c = 0; bin_names[c] != NULL; c++);
        printf("Reloaded %d bin names\n", c);

        return NULL;
    }

    // print functions lines
    FILE *file = fopen("funcs.olv", "w");

    for (int i = 0; i < MAX_FUNCTIONS && functions[i].name != NULL; i++) {
        fprintf(file, "FUNC %s\n", functions[i].name);
        for (int j = 0; j < functions[i].line_count; j++) {
            print_internal_string(functions[i].lines[j].str, file);
            fputc('\n', file);
        }
        fputs("END\n", file);
    }

    fclose(file);
    puts("Functions saved to 'funcs.olv' use 'olivine -p funcs.olv' to display them");

    return NULL;
}

char *if_del(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc == 1 && input[0][0] != '-') {
        del_variable(input[0]);
        return NULL;
    }

    if (argc == 2 && input[0][0] == '-') {
        switch (input[0][1]) {
            case 'v':
                del_variable(input[1]);
                return NULL;
            case 'f':
                del_function(input[1]);
                return NULL;
            case 'p':
                del_pseudo(input[1]);
                return NULL;
            #if USE_ENVVARS
            case 'e':
                unsetenv(input[1]);
                return NULL;
            #endif
            default:
                break;
        }
    }

    if (argc == 1 && strcmp(input[0], "-h") == 0) {
        puts("Usage: del [arg] <name>\n"
            "  -v   variable (default)\n"
            "  -f   function\n"
            "  -p   pseudo\n"
            #if USE_ENVVARS
            "  -e   environment"
            #endif
        );
        return NULL;
    }

    raise_error("del", "Usage: del [arg] <name>");
    fputs("Try 'del -h' for more information\n", stderr);

    return ERROR_CODE;
}

typedef struct {
    char *world;
    char *name;
    int fd;
    int append;
} if_dot_redirect_t;

char *if_dot(char **input) {
    #if PROFANBUILD || UNIXBUILD
    // get argc
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc < 1) {
        raise_error("dot", "Usage: . <file> [args] [&] [> <stdout>]");
        return ERROR_CODE;
    }

    // check if file ends with .olv
    int len = strlen(input[0]);
    if (len > 4 && strcmp(input[0] + len - 4, ".olv") == 0) {
        FILE *file = fopen(input[0], "r");
        char line[16];
        if (file != NULL) {
            if (fgets(line, sizeof(line), file) != NULL && strcmp(line, "//olivine:exec\n") == 0) {
                fclose(file);
                return execute_file(input[0], input + 1) ? ERROR_CODE : NULL;
            }
            fclose(file);
        }
    }

    char *file_path = input[0];
    // check if file exists
    if (access(input[0], F_OK | X_OK) == -1) {
        raise_error("dot", "File '%s' does not exist", input[0]);
        return ERROR_CODE;
    }

    if_dot_redirect_t redirect[] = {
        {"<",   "stdin",  0, 0},
        {">",   "stdout", 1, 0},
        {">>",  "stdout", 1, 1},
        {"2>",  "stderr", 2, 0},
        {"2>>", "stderr", 2, 1}
    };

    int fds[3] = {-1, -1, -1};
    int mode, fd, wait_end = 1;

    for (int i = argc - 1; i >= 0; i--) {
        if (strcmp(input[i], "&") == 0) {
            input[i] = NULL;
            wait_end = 0;
            continue;
        }
        i--;
        if (i < 1)
            break;
        for (int j = 0; j < 5; j++) {
            if (strcmp(input[i], redirect[j].world))
                continue;
            fd = redirect[j].fd;
            if (input[i + 1] == NULL) {
                raise_error("dot", "No path specified for %s", redirect[j].name);
                goto dot_redir_error;
            }
            if (fds[fd] != -1) {
                raise_error("dot", "Multiple redirections for %s", redirect[j].name);
                goto dot_redir_error;
            }

            if (fd == 0) {
                mode = O_RDONLY;
            } else {
                mode = O_WRONLY | O_CREAT;
                if (redirect[j].append)
                    mode |= O_APPEND;
                else
                    mode |= O_TRUNC;
            }

            fds[fd] = open(input[i + 1], mode, 0644);
            input[i] = NULL;

            if (fds[fd] == -1) {
                raise_error("dot", "Cannot open file '%s' for %s", input[i + 1], redirect[j].name);
                goto dot_redir_error;
            }
            break;
        }
        if (input[i])
            break;
    }

    for (argc = 0; input[argc] != NULL; argc++);

    if (0) {
        dot_redir_error:
        for (int i = 0; i < 3; i++) {
            if (fds[i] != -1)
                close(fds[i]);
        }
        return ERROR_CODE;
    }

    for (int i = 0; i < 3; i++) {
        if (fds[i] == -1)
            continue;
        fd = dup(i);
        dup2(fds[i], i);
        close(fds[i]);
        fds[i] = fd;
    }

    // get args
    char *file_name = strdup(input[0]);
    // remove the extension
    char *dot = strrchr(file_name, '.');
    if (dot) *dot = '\0';
    // remove the path
    char *slash = strrchr(file_name, '/');
    if (slash) memmove(file_name, slash + 1, strlen(slash));

    char **argv = malloc((argc + 1) * sizeof(char *));
    argv[0] = file_name;
    for (int i = 1; i < argc; i++)
        argv[i] = input[i];
    argv[argc] = NULL;

    pid_t pid;

    #if PROFANBUILD
    char *full_path = profan_join_path(g_current_directory, file_path);

    run_ifexist_full(
        (runtime_args_t) {
            full_path,
            argc, argv,
            environ,
            2
        }, &pid
    );

    free(full_path);

    #else // UNIXBUILD
    pid = fork();

    if (pid == 0) {
        execv(file_path, argv);
        raise_error("dot", "Cannot execute file '%s'", file_path);
        free(file_name);
        free(argv);
        exit(1);
    }

    #endif

    // restore fds
    for (int i = 0; i < 3; i++) {
        if (fds[i] != -1) {
            dup2(fds[i], i);
            close(fds[i]);
        }
    }

    if (pid == -1) {
        raise_error("dot", "Cannot execute file '%s'", file_path);
        free(file_name);
        free(argv);
        return ERROR_CODE;
    }

    int status = 0;

    #if PROFANBUILD
    if (wait_end) {
        syscall_process_wakeup(pid, 1);
    } else {
        syscall_process_wakeup(pid, 0);
    }
    #endif

    if (wait_end) {
        waitpid(pid, &status, 0);
        status = WEXITSTATUS(status);
    } else {
        fprintf(stderr, "DOT: started with pid %d\n", pid);
    }

    // remove zombie processes
    while (waitpid(-1, NULL, WNOHANG) > 0);

    local_itoa(status, g_exit_code);

    char tmp[13];
    local_itoa(pid, tmp);
    set_variable("spi", tmp);

    if (file_path != input[0])
        free(file_path);
    free(file_name);
    free(argv);

    return g_exit_code[0] == '0' ? NULL : ERROR_CODE;
    #else
    UNUSED(input);
    raise_error("dot", "Not supported in this build");
    return ERROR_CODE;
    #endif
}

char *if_exec(char **input) {
    // get argc
    int argc = 0;
    for (int i = 0; input[i] != NULL; i++) {
        argc++;
    }

    if (argc != 1) {
        raise_error("exec", "Usage: exec <file> [arg1 arg2 ...]");
        return ERROR_CODE;
    }

    if (execute_file(input[0], input + 1))
        return ERROR_CODE;
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

    if (!is_valid_name(input[0])) {
        raise_error("export", "Invalid name '%s'", input[0]);
        return ERROR_CODE;
    }

    #if USE_ENVVARS
        setenv(input[0], input[1], 1);
        return NULL;
    #else
        raise_error("export", "Environment variables are disabled");
        return ERROR_CODE;
    #endif
}

char *if_fsize(char **input) {
    int file_size = 0;

    // get argc
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc != 1) {
        raise_error("fsize", "Usage: fsize <file>");
        return ERROR_CODE;
    }

    #if PROFANBUILD
    // get path
    uint32_t file_id = profan_resolve_path(input[0]);

    // check if file exists
    if (IS_SID_NULL(file_id)) {
        file_size = -1;
    } else {
        file_size = syscall_fs_get_size(NULL, file_id);
    }

    #else

    FILE *file = fopen(input[0], "r");
    if (file == NULL) {
        file_size = -1;
    } else {
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fclose(file);
    }

    #endif

    char *res = malloc(12);
    if (file_size == -1) {
        strcpy(res, "null");
    } else {
        local_itoa(file_size, res);
    }

    return res;
}

char *if_global(char **input) {
    // get argc
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc != 2) {
        raise_error("global", "Usage: global <name> <value>");
        return ERROR_CODE;
    }

    // get name
    char *name = input[0];

    if (!is_valid_name(name)) {
        raise_error("global", "Invalid name '%s'", name);
        return ERROR_CODE;
    }

    // set variable
    if (set_variable_global(name, input[1])) {
        return ERROR_CODE;
    }

    return NULL;
}

char *if_inter(char **input) {
    // make variable and quote usable for olivine

    if (input[0] == NULL || input[1] != NULL) {
        raise_error("inter", "Usage: inter <string>");
        return ERROR_CODE;
    }

    char *output = malloc(strlen(input[0]) + 1);

    int i;
    for (i = 0; input[0][i] != '\0'; i++) {
        switch (input[0][i]) {
            case USER_QUOTE:
                output[i] = INTR_QUOTE;
                break;
            case USER_VARDF:
                output[i] = INTR_VARDF;
                break;
            case USER_WILDC:
                output[i] = INTR_WILDC;
                break;
            default:
                output[i] = input[0][i];
        }
    }

    output[i] = '\0';

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
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc != 2) {
        raise_error("pseudo", "Usage: pseudo <name> <value>");
        return ERROR_CODE;
    }

    // get name
    char *name = input[0];

    // get value
    char *value = input[1];

    if (!is_valid_name(name)) {
        raise_error("pseudo", "Invalid name '%s'", name);
        return ERROR_CODE;
    }

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

    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc == 0 || argc > 2) {
        raise_error("range", "Usage: range [start] <end>");
        return ERROR_CODE;
    }

    int start, end;
    unsigned nblen;

    if (argc == 1) {
        start = 0;
        if (local_atoi(input[0], &end))
            return (raise_error("range", "Invalid number '%s'", input[0]), ERROR_CODE);
    } else {
        if (local_atoi(input[0], &start))
            return (raise_error("range", "Invalid number '%s'", input[0]), ERROR_CODE);
        if (local_atoi(input[1], &end))
            return (raise_error("range", "Invalid number '%s'", input[1]), ERROR_CODE);
    }

    if (start == end) {
        return NULL;
    }

    char tmp[13];

    // get the length of the biggest number
    local_itoa(end, tmp);
    nblen = strlen(tmp);
    local_itoa(start, tmp);
    if (nblen < strlen(tmp))
        nblen = strlen(tmp);

    char *output;
    int output_len = 0;

    if (start > end) {
        output = malloc((start - end) * (nblen + 1) + 1);
        for (int i = start; i > end; i--) {
            local_itoa(i, tmp);
            strcat(tmp, " ");
            for (int j = 0; tmp[j]; j++)
                output[output_len++] = tmp[j];
        }
    } else {
        output = malloc((end - start) * (nblen + 1) + 1);
        for (int i = start; i < end; i++) {
            local_itoa(i, tmp);
            strcat(tmp, " ");
            for (int j = 0; tmp[j]; j++)
                output[output_len++] = tmp[j];
        }
    }
    output[--output_len] = '\0';

    return output;
}

char *if_rep(char **input) {
    /*
     * input ["hello", "hl", "HP"]
     * output: "HePPo"
    */

    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc != 2 && argc != 3) {
        raise_error("rep", "Usage: rep <string> <chars> [replacements]");
        return ERROR_CODE;
    }

    if (argc == 3 && strlen(input[1]) != strlen(input[2])) {
        raise_error("rep", "Expected 2nd and 3rd arguments to have the same length, got %d and %d",
                strlen(input[1]), strlen(input[2]));
        return ERROR_CODE;
    }

    char *ret = malloc(strlen(input[0]) + 1);

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

char *if_set(char **input) {
    // get argc
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc == 0 || argc > 2) {
        raise_error("set", "Usage: set <name> [value]");
        return ERROR_CODE;
    }

    if (!is_valid_name(input[0])) {
        raise_error("set", "Invalid name '%s'", input[0]);
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
        value = malloc(INPUT_SIZE + 1);
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

char *if_split(char **input) {
    /*
     * input: ["hello$world", "$"]
     * output: "'hello' 'world'"
    */

    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc < 1 || argc > 3) {
        raise_error("split", "Usage: split <string> [separator] [<from>:<to>]");
        return ERROR_CODE;
    }

    char *sep = argc > 1 ? input[1] : " ";
    int from = 0, to = INT_MAX;

    if (argc == 3) {
        int end;
        for (end = 0; input[2][end] != ':' && input[2][end]; end++);

        char *from_str = malloc(end + 1);
        strncpy(from_str, input[2], end);
        from_str[end] = '\0';
        if (local_atoi(from_str, &from)) {
            raise_error("split", "Invalid number '%s'", from_str);
            free(from_str);
            return ERROR_CODE;
        }

        free(from_str);
        if (input[2][end] != ':') {
            to = from == -1 ? INT_MAX : from + 1;
        } else if (local_atoi(input[2] + end + 1, &to)) {
            raise_error("split", "Invalid number '%s'", input[2] + end + 1);
            return ERROR_CODE;
        }
    }

    int slen = strlen(sep);
    int count, oindex, in;
    char *output;

    if (slen == 0) {
        output = input[0];
        count = strlen(output);
        goto split_range;
    }

    output = malloc(strlen(input[0]) * 4 + 1);
    count = oindex = in = 0;

    for (int i = 0; input[0][i] != '\0'; i++) {
        if (strncmp(input[0] + i, sep, slen) == 0) {
            if (in)
                output[oindex++] = INTR_QUOTE;
            i += slen - 1;
            in = 0;
        } else {
            if (!in) {
                if (oindex)
                    output[oindex++] = ' ';
                output[oindex++] = INTR_QUOTE;
                count++;
                in = 1;
            }
            output[oindex++] = input[0][i];
        }
    }
    if (in)
        output[oindex++] = INTR_QUOTE;
    output[oindex] = '\0';

    split_range:

    if (from == 0 && to == INT_MAX) {
        if (slen == 0)
            return strdup(output);
        if (count == 0) {
            free(output);
            return NULL;
        }
        return realloc(output, oindex + 1);
    }

    if (from < 0)
        from = count + from;
    if (to < 0)
        to = count + to;
    if (to == INT_MAX)
        to = count;

    if (from < 0 || to < 0 || from >= count || to > count || from > to) {
        raise_error("split", "Invalid range %d:%d", from, to);
        free(output);
        return ERROR_CODE;
    }

    if (slen == 0) {
        char *res = malloc(to - from + 1);
        strncpy(res, output + from, to - from);
        res[to - from] = '\0';
        return res;
    }

    in = count = 0;

    for (int i = 0; output[i] != '\0'; i++) {
        while (output[++i] && output[i] != INTR_QUOTE);
        count++;
        i++;

        if (count == to) {
            output[i] = '\0';
            break;
        }

        if (count == from)
            in = i + 1;
    }

    char *res = malloc(oindex - in + 1);
    strcpy(res, output + in);

    free(output);
    return res;
}

char *if_sprintf(char **input) {
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

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

    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc != 1) {
        raise_error("strlen", "Usage: strlen <string>");
        return ERROR_CODE;
    }

    char *output = malloc(11);

    local_itoa(strlen(input[0]), output);

    return output;
}

char *if_ticks(char **input) {
    /*
     * input: []
     * output: "'123456789'"
    */

    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc != 0) {
        raise_error("ticks", "Usage: ticks");
        return ERROR_CODE;
    }

    char *output = malloc(12);

    #if PROFANBUILD
    local_itoa(syscall_timer_get_ms(), output);
    #elif UNIXBUILD
    struct timeval tv;
    gettimeofday(&tv, NULL);
    local_itoa((tv.tv_sec * 1000 + tv.tv_usec / 1000) % 0x7FFFFFFF, output);
    #else
    local_itoa(0, output);
    #endif

    return output;
}


internal_function_t internal_functions[] = {
    {".", if_dot},
    {"cd", if_cd},
    {"debug", if_debug},
    {"del", if_del},
    {"eval", if_eval},
    {"exec", if_exec},
    {"exit", if_exit},
    {"export", if_export},
    {"fsize", if_fsize},
    {"global", if_global},
    {"inter", if_inter},
    {"print", if_print},
    {"pseudo", if_pseudo},
    {"range", if_range},
    {"rep", if_rep},
    {"set", if_set},
    {"split", if_split},
    {"sprintf", if_sprintf},
    {"strlen", if_strlen},
    {"ticks", if_ticks},
    {NULL, NULL}
};

void *get_if_function(const char *name) {
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
        if (string[i] == INTR_QUOTE) {
            dec++;
            continue;
        }
        string[i - dec] = string[i];
    }
    string[i - dec] = '\0';
}

int does_startwith(const char *str, const char *start) {
    int i;
    for (i = 0; start[i] != '\0'; i++) {
        if (LOWERCASE(str[i]) != LOWERCASE(start[i])) {
            return 0;
        }
    }
    if (str[i] == '\0' || IS_SPACE_CHAR(str[i])) {
        return 1;
    }
    return 0;
}

int quotes_less_copy(char *dest, const char *src, int len) {
    int i = 0;
    int in_string = 0;
    for (int j = 0; src[j] != '\0' && i < len; j++) {
        if (src[j] == INTR_QUOTE) {
            in_string = !in_string;
            len--;
        } else if (in_string) {
            dest[i++] = src[j];
        } else if (!IS_SPACE_CHAR(src[j])) {
            dest[i++] = src[j];
        }
    }
    dest[i] = '\0';
    return i;
}

char **gen_args(const char *string) {
    if (string == NULL)
        return calloc(1, sizeof(char *));

    while (IS_SPACE_CHAR(*string))
        string++;

    if (*string == '\0')
        return calloc(1, sizeof(char *));

    // count the number of arguments
    int in_string = 0;
    int argc = 1;
    int len;

    for (len = 0; string[len]; len++) {
        if (string[len] == INTR_QUOTE) {
            in_string = !in_string;
        } if (IS_SPACE_CHAR(string[len]) && !in_string) {
            while (IS_SPACE_CHAR(string[len + 1]))
                len++;
            argc++;
        }
    }

    if (in_string) {
        raise_error(NULL, "String not closed");
        return ERROR_CODE;
    }

    // allocate the arguments array
    char **argv = malloc((argc + 1) * sizeof(char *) + len + 1);
    char *args = (char *)(argv + argc + 1);

    // fill the arguments array
    int old_i, arg_i, i;
    old_i = arg_i = 0;
    for (i = 0; string[i] != '\0'; i++) {
        if (string[i] == INTR_QUOTE) {
            in_string = !in_string;
        }

        if (IS_SPACE_CHAR(string[i]) && !in_string) {
            int tmp = quotes_less_copy(args, string + old_i, i - old_i);
            argv[arg_i++] = args;
            while (IS_SPACE_CHAR(string[i + 1]))
                i++;
            old_i = i + 1;
            args += tmp + 1;
        }
    }

    if (old_i != i) {
        len = quotes_less_copy(args, string + old_i, len - old_i);
        argv[arg_i++] = args;
    }

    argv[arg_i] = NULL;
    return argv;
}

char *args_rejoin(char **input, int to) {
    int size = 1;
    for (int i = 0; i < to; i++) {
        size += strlen(input[i]) + 3;
    }

    char *joined_input = malloc(size);
    joined_input[0] = '\0';

    size = 0;
    for (int i = 0; i < to; i++) {
        joined_input[size++] = INTR_QUOTE;
        for (int j = 0; input[i][j] != '\0'; j++) {
            joined_input[size++] = input[i][j];
        }
        joined_input[size++] = INTR_QUOTE;
        if (i != to - 1) {
            joined_input[size++] = ' ';
        }
    }
    joined_input[size] = '\0';

    return joined_input;
}

char *get_function_name(const char *line) {
    while (IS_SPACE_CHAR(*line))
        line++;
    if (*line == '\0')
        return NULL;
    int in_string = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == INTR_QUOTE) {
            in_string = !in_string;
        }

        if (IS_SPACE_CHAR(line[i]) && !in_string) {
            char *function_name = malloc(i + 1);
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
    for (int i = 0; argv[i] != NULL; i++)
        free(argv[i]);
    free(argv);
}

void free_vars(void) {
    for (int i = 0; i < MAX_VARIABLES && variables[i].name; i++) {
        if (variables[i].is_sync)
            continue;
        free(variables[i].value);
        free(variables[i].name);
    }
    free(variables);
}

void free_pseudos(void) {
    for (int i = 0; i < MAX_PSEUDOS && pseudos[i].name; i++) {
        free(pseudos[i].name);
        free(pseudos[i].value);
    }
    free(pseudos);
}

void free_functions(void) {
    for (int i = 0; i < MAX_FUNCTIONS && functions[i].name; i++) {
        free(functions[i].name);
        free(functions[i].lines);
    }
    free(functions);
}

/*****************************
 *                          *
 *  Olivine Pipe Processor  *
 *                          *
*****************************/

char *execute_line(const char *full_line);

char *pipe_processor(char **input) {
    #if PROFANBUILD || UNIXBUILD
    // get argc
    int argc;
    for (argc = 0; input[argc] != NULL; argc++);

    if (argc == 0) {
        raise_error("Pipe Processor", "Requires at least one argument");
        return ERROR_CODE;
    }

    int old_fds[2] = {
        dup(0),
        dup(1),
    };

    char *line, *tmp = NULL, *ret = NULL;
    int fd[2], fdin, start = 0;
    fdin = dup(0);
    for (int i = 0; i < argc + 1; i++) {
        if (i != argc && strcmp(input[i], "|"))
            continue;

        if (argc == i) {
            dup2(old_fds[1], 1);
            dup2(fdin, 0);
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
            while (s && (ret[s - 1] == '\n' || ret[s - 1] == '\0'))
                s--;
            ret[s] = '\0';
            break;
        }

        line = args_rejoin(input + start, i - start);
        start = i + 1;

        if (argc != i) {
            if (pipe(fd) == -1) {
                raise_error("Pipe Processor", "Pipe failed");
                ret = ERROR_CODE;
                break;
            }
            dup2(fdin, 0);
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
    dup2(old_fds[0], 0);

    // if line execution failed, print pipe content
    if (tmp == ERROR_CODE) {
        raise_error("Pipe Processor", "Command failed");
        char buffer[101];
        int n = read(fdin, buffer, 100);
        if (n > 0) {
            puts("\e[37m  --- Pipe Content ---\e[0m");
            do {
                buffer[n] = '\0';
                fputs(buffer, stdout);
            } while ((n = read(fdin, buffer, 100)) > 0);
            puts("\e[37m  --------------------\e[0m");
        }
    }

    close(fdin);

    close(old_fds[0]);
    close(old_fds[1]);

    return ret;
    #else
    UNUSED(input);
    raise_error("Pipe Processor", "Not supported in this build");
    return ERROR_CODE;
    #endif
}

/**************************
 *                       *
 *  Execution Functions  *
 *                       *
**************************/

int execute_lines(olv_line_t *lines, int line_end, char **result);

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

    char *result = malloc(1);
    int ret = execute_lines(function->lines, function->line_count, &result);

    // free variables
    del_variable_level(g_current_level--);

    if (ret == -1) {
        // function failed
        free(result);
        return ERROR_CODE;
    }

    return result;
}

char *check_subfunc(const char *line);

char *check_bin(char *name, char *line, void **function) {
    char *bin_path = get_bin_path(name);
    if (bin_path == NULL) {
        return line;
    }

    *function = if_dot;
    char *new_line = malloc(strlen(line) + strlen(bin_path) + 2);
    strcpy(new_line, ". ");
    strcat(new_line, bin_path);
    free(bin_path);

    int i, in_quote = 0;

    for (i = 0; IS_SPACE_CHAR(line[i]); i++);

    for (; line[i]; i++) {
        if (line[i] == INTR_QUOTE) {
            in_quote = !in_quote;
        }
        if (IS_SPACE_CHAR(line[i]) && !in_quote) {
            strcat(new_line, line + i);
            break;
        }
    }

    free(line);

    return new_line;
}

char *execute_line(const char *full_line) {
    // check for function and variable
    char *line = check_subfunc(full_line);
    int pipe_after = 0;

    if (line == NULL) {
        // subfunction failed
        return ERROR_CODE;
    }

    // get the function name
    int isif = 0;
    char *function_name = get_function_name(line);

    if (function_name == NULL) {
        raise_error(NULL, "Expected function name, but got empty string");
        free(line);
        return ERROR_CODE;
    }

    // get the function address
    void *function = get_function(function_name);
    if (function == NULL) {
        function = get_if_function(function_name);
        isif = 1;
    }

    char *result;

    if (function == NULL && BIN_AS_PSEUDO) {
        line = check_bin(function_name, line, &function);
    }

    if (function == NULL) {
        raise_error(NULL, "Function '%s' not found", function_name);
        result = ERROR_CODE;
    } else {
        // generate the arguments array
        char **function_args = gen_args(line);
        if (function_args == ERROR_CODE) {
            free(function_name);
            free(line);
            return ERROR_CODE;
        }

        // check if "|" is present
        for (int i = 0; function_args[i] != NULL; i++) {
            if (function_args[i][0] == '|' && function_args[i][1] == '\0') {
                pipe_after = 1;
                break;
            }
        }

        #if ENABLE_DEBUG
            fprintf(stderr, DEBUG_COLOR"   %03d EXEC:", g_fileline);
            for (int i = 0; function_args[i] != NULL; i++) {
                fprintf(stderr, " %s", function_args[i]);
            }
            fputs("\n\e[0m", stderr);
        #endif

        // execute the function
        if (pipe_after)
            result = pipe_processor(function_args);
        else if (isif)
            result = ((char *(*)(char **)) function)(function_args + 1);
        else
            result = execute_function(function, function_args + 1);

        free(function_args);
    }

    free(function_name);
    free(line);

    return result;
}

char *check_variables(char *line) {
    // check if !... is present
    int end;

    for (int start = 0; line[start];) {
        if (line[start] != INTR_VARDF || line[start + 1] == INTR_VARDF) {
            start++;
            continue;
        }

        for (end = start + 1; IS_NAME_CHAR(line[end]) || line[end] == '#'; end++);

        if (end - start == 1) {
            raise_error(NULL, "Empty variable name");
            free(line);
            return NULL;
        }

        char old = line[end];
        line[end] = '\0';


        // get the variable value
        char *var_value = get_variable(line + start + 1);

        if (var_value == NULL) {
            raise_error(NULL, "Variable '%s' not found", line + start + 1);
            line[end] = old;
            free(line);
            return NULL;
        }

        line[end] = old;

        // replace the variable with its value
        char *nline = malloc(strlen(line) - (end - start) + strlen(var_value) + 1);
        strncpy(nline, line, start);
        strcpy(nline + start, var_value);
        strcat(nline, line + end);

        free(line);

        if (start)
            start--;

        line = nline;
    }

    return line;
}

char *check_pseudos(char *line) {
    char *pseudo_value, old;
    int end;

    while (1) {
        for (end = 0; line[end] && !IS_SPACE_CHAR(line[end]); end++);

        old = line[end];
        line[end] = '\0';

        pseudo_value = get_pseudo(line);
        line[end] = old;

        if (pseudo_value == NULL)
            return line;

        char *nline = malloc(strlen(line) - end + strlen(pseudo_value) + 2); // TODO: check if +2 is needed
        strcpy(nline, pseudo_value);
        strcat(nline, line + end);
        free(line);
        line = nline;
    }
}

#if ENABLE_WILDC

char *check_wildcards(char *line) {
    // check if * is present outside of quotes

    // make a fast check before
    int start = 1;

    for (int i = 0; line[i]; i++) {
        if (line[i] == INTR_WILDC) {
            start = 0;
            break;
        }
    }

    if (start)
        return line;

    for (int i = 0; line[i]; i++) {
        if (line[i] == INTR_QUOTE && (i == 0 || line[i - 1] != '\\')) {
            start = start == -1 ? i + 1 : -1;
            continue;
        }
        if (start == -1)
            continue;
        if (line[i] == ' ')
            start = i + 1;
        if (line[i] != INTR_WILDC)
            continue;

        int end;
        for (end = i + 1; line[end] && !IS_SPACE_CHAR(line[end]); end++);

        char old = line[end];
        line[end] = '\0';

        char *wildcard = resolve_wildcard("", line + start);
        line[end] = old;

        if (wildcard == NULL) {
            for (int j = start; j < end; j++) {
                if (line[j] == INTR_WILDC)
                    line[j] = USER_WILDC;
            }
            raise_error(NULL, "Wildcard '%s' not found", line + start);
            free(line);
            return NULL;
        }

        int len = strlen(wildcard);

        char *nline = malloc(strlen(line) - (end - start) + len + 1);
        strncpy(nline, line, start);
        strcpy(nline + start, wildcard);
        strcat(nline, line + end);

        free(wildcard);
        free(line);
        line = nline;

        i = start + len - 1;
    }

    return line;
}
#endif

char *check_subfunc(const char *input) {
    // check if !( ... ) is present
    int open_parentheses, end;
    char *line = strdup(input);

    for (int start = 0; line[start];) {
        if (line[start] != INTR_VARDF || line[start + 1] != '(') {
            start++;
            continue;
        }

        open_parentheses = 1;

        for (end = start + 2; line[end]; end++) {
            switch (line[end]) {
                case '(':
                    open_parentheses++;
                    break;
                case ')':
                    open_parentheses--;
                    break;
            }
            if (!open_parentheses)
                break;
        }

        if (open_parentheses) {
            raise_error(NULL, "Missing closing parenthesis");
            free(line);
            return NULL;
        }

        char old = line[end];
        line[end] = '\0';

        // execute the subfunc
        char *subfunc_result = execute_line(line + start + 2);

        if (subfunc_result == ERROR_CODE) {
            // subfunc execution failed
            free(line);
            return NULL;
        }

        line[end] = old;

        // replace the subfunc with its result
        char *nline;

        if (subfunc_result) {
            nline = malloc(strlen(line) - (end - start) + strlen(subfunc_result) + 1);
            strncpy(nline, line, start);
            strcpy(nline + start, subfunc_result);
            strcat(nline, line + end + 1);
            free(subfunc_result);
        } else {
            nline = malloc(strlen(line) - (end - start) + 1);
            strncpy(nline, line, start);
            strcpy(nline + start, line + end + 1);
        }

        free(line);

        if (start)
            start--;

        line = nline;
    }

    line = check_variables(line);

    if (line == NULL)
        return NULL;

    #if ENABLE_WILDC
        line = check_wildcards(line);
        if (line == NULL)
            return NULL;
    #endif

    return check_pseudos(line);
}

/*************************
 *                      *
 *  Execution Functions *
 *                      *
*************************/

int check_condition(const char *condition) {
    char *verif = check_subfunc(condition);

    if (verif == NULL)
        return -1;

    int res = 1;

    if (strcmp(verif, "0") == 0 || local_strncmp_nocase(verif, "false", 5) == 0)
        res = 0;

    free(verif);
    return res;
}

int get_line_end(int line_count, olv_line_t *lines) {
    // return the line number or -1 on error

    int end_offset = 1;
    for (int i = 1; i < line_count; i++) {
        if (
            does_startwith(lines[i].str, "IF")    ||
            does_startwith(lines[i].str, "FOR")   ||
            does_startwith(lines[i].str, "WHILE") ||
            does_startwith(lines[i].str, "FUNC")  ||
            does_startwith(lines[i].str, "ELSE")
        ) {
            end_offset++;
        } else if (does_startwith(lines[i].str, "END")) {
            end_offset--;
        }

        if (end_offset)
            continue;

        if (strlen(lines[i].str) != 3) {
            raise_error_line(lines[i].fileline, "END", "Invalid statement");
            return -1;
        }

        return i;
    }

    raise_error_line(lines[line_count - 1].fileline, NULL,
            "Missing END statement for keyword line %d", lines[0].fileline + 1);

    return -1;
}

int execute_if(int line_count, olv_line_t *lines, char **result, int *cnd_state);
int execute_else(int line_count, olv_line_t *lines, char **result, int *last_if_state);
int execute_while(int line_count, olv_line_t *lines, char **result);
int execute_for(int line_count, olv_line_t *lines, char **result);
int save_function(int line_count, olv_line_t *lines);
int execute_return(const char *line, char **result);

#define execute_lines_ret(val) return (g_fileline = old_fileline, val)

int execute_lines(olv_line_t *lines, int line_end, char **result) {
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

    int old_fileline = g_fileline;

    int lastif_state = 2; // 0: false, 1: true, 2: not set
    char *res = NULL;

    for (int i = 0; i < line_end; i++) {
        g_fileline = lines[i].fileline;

        #if ENABLE_DEBUG
            fprintf(stderr, DEBUG_COLOR"=> %03d READ: ", g_fileline);
            print_internal_string(lines[i].str, stderr);
            fputs("\e[0m\n", stderr);
        #endif

        if (does_startwith(lines[i].str, "FOR")) {
            int ret = execute_for(line_end - i, lines + i, result);
            if (ret == -1) {
                // invalid FOR loop
                execute_lines_ret(-1);
            } else if (ret < -1) {
                execute_lines_ret(ret);
            }

            i += ret;
        }

        else if (does_startwith(lines[i].str, "IF")) {
            int ret = execute_if(line_end - i, lines + i, result, &lastif_state);

            if (ret == -1) {
                // invalid IF statement
                execute_lines_ret(-1);
            } else if (ret < -1) {
                execute_lines_ret(ret);
            }

            i += ret;
        }

        else if (does_startwith(lines[i].str, "ELSE")) {
            int ret = execute_else(line_end - i, lines + i, result, &lastif_state);

            if (ret == -1) {
                // invalid ELSE statement
                execute_lines_ret(-1);
            } else if (ret < -1) {
                execute_lines_ret(ret);
            }

            i += ret;
        }

        else if (does_startwith(lines[i].str, "WHILE")) {
            int ret = execute_while(line_end - i, lines + i, result);

            if (ret == -1) {
                // invalid WHILE loop
                execute_lines_ret(-1);
            } else if (ret < -1) {
                execute_lines_ret(ret);
            }

            i += ret;
        }

        else if (does_startwith(lines[i].str, "FUNC")) {
            int ret = save_function(line_end - i, lines + i);

            if (ret == -1) {
                // invalid FUNCTION declaration
                execute_lines_ret(-1);
            } else if (ret < -1) {
                execute_lines_ret(ret);
            }

            i += ret;
        }

        else if (does_startwith(lines[i].str, "END")) {
            raise_error(NULL, "Suspicious END keyword");
            execute_lines_ret(-1);
        }

        else if (does_startwith(lines[i].str, "BREAK")) {
            execute_lines_ret(-3);
        }

        else if (does_startwith(lines[i].str, "CONTINUE")) {
            execute_lines_ret(-2);
        }

        else if (does_startwith(lines[i].str, "RETURN")) {
            int ret = execute_return(lines[i].str, result);
            execute_lines_ret(ret);
        }

        else {
            res = execute_line(lines[i].str);

            if (res == ERROR_CODE) {
                if (STOP_ON_ERROR)
                    execute_lines_ret(-1);
            } else if (res) {
                for (int i = 0; res[i]; i++) {
                    if (res[i] == INTR_QUOTE)
                        res[i] = USER_QUOTE;
                }
                if (res[0])
                    puts(res);
                free(res);
                res = NULL;
            }
        }

        if (res != ERROR_CODE) {
            strcpy(g_exit_code, "0");
        }
    }
    execute_lines_ret(0);
}

int execute_return(const char *line, char **result) {
    if (strlen(line) < 7 || result == NULL) {
        return -4;
    }

    char *res = check_subfunc(line + 7);

    if (res == NULL) {
        // invalid RETURN statement
        return -1;
    }

    free(*result);
    *result = res;

    return -4;
}

int execute_for(int line_count, olv_line_t *lines, char **result) {
    char *for_line = check_subfunc(lines[0].str);

    if (for_line == NULL) {
        return -1;
    }

    if (strlen(for_line) < 5) {
        raise_error("FOR", "No variable name provided");
        free(for_line);

        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == -1) {
        free(for_line);
        return -1;
    }

    int index;
    for (index = 4; for_line[index] && !IS_SPACE_CHAR(for_line[index]); index++);
    for_line[index++] = '\0';

    while (IS_SPACE_CHAR(for_line[index]))
        index++;

    if (for_line[index] == '\0') {
        free(for_line);
        return line_end;
    }

    char *var_name = for_line + 4;

    int var_exist_before = get_variable_index(var_name, 0) != -1;
    int in_string = 0;
    int var_len = 0;

    int res;
    // execute the loop
    while (for_line[index]) {
        // skip spaces
        while (IS_SPACE_CHAR(for_line[index]))
            index++;
        if (for_line[index] == '\0')
            break;

        // set the the variable
        for (var_len = 0; for_line[index + var_len] != '\0'; var_len++) {
            if (for_line[index + var_len] == INTR_QUOTE) {
                in_string = !in_string;
            } else if (!in_string && IS_SPACE_CHAR(for_line[index + var_len])) {
                break;
            }
        }

        set_variable_withlen(var_name, for_line + index, var_len, 0);

        index += var_len;

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

    free(for_line);

    return line_end;
}

int execute_if(int line_count, olv_line_t *lines, char **result, int *cnd_state) {
    const char *condition = lines[0].str + 3;
    int tmp;

    if (condition[0] == '\0') {
        raise_error("IF", "No condition provided");
        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == -1)
        return -1;

    tmp = check_condition(condition);

    if (tmp == -1)
        return -1;

    *cnd_state = tmp;

    if (tmp) {
        tmp = execute_lines(lines + 1, line_end - 1, result);
        if (tmp < 0) line_end = tmp;
    }

    return line_end;
}

int execute_else(int line_count, olv_line_t *lines, char **result, int *last_if_state) {
    const char *else_line = lines[0].str;

    if (*last_if_state == 2) {   // not set
        raise_error("ELSE", "No IF statement before");
        return -1;
    }

    if (else_line[4] != '\0') {
        raise_error("ELSE", "No condition expected");
        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == -1) {
        return -1;
    }

    if (!(*last_if_state)) {
        int ret = execute_lines(lines + 1, line_end - 1, result);
        if (ret < 0) line_end = ret;
    }

    *last_if_state = !(*last_if_state);

    return line_end;
}

int execute_while(int line_count, olv_line_t *lines, char **result) {
    const char *condition = lines[0].str + 6;

    // check condition length
    if (strlen(condition) == 0) {
        raise_error("WHILE", "No condition provided");
        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == -1)
        return -1;

    // execute while loop
    while (1) {
        int verif = check_condition(condition);

        // invalid condition for WHILE loop
        if (verif == -1)
            return -1;

        if (!verif)
            break;

        int ret = execute_lines(lines + 1, line_end - 1, result);
        if (ret == -3) {
            break;
        } if (ret < 0 && ret != -2) {
            return ret;
        }
    }

    return line_end;
}

int save_function(int line_count, olv_line_t *lines) {
    const char *func_line = lines[0].str + 5;

    if (func_line[0] == '\0') {
        raise_error(NULL, "Missing function name");
        return -1;
    }

    int line_end = get_line_end(line_count, lines);

    if (line_end == -1) {
        return -1;
    }

    if (!is_valid_name(func_line)) {
        raise_error(NULL, "Invalid function name '%s'", func_line);
        return -1;
    }

    return set_function(func_line, lines + 1, line_end - 1) ? -1 : line_end;
}

void execute_program(const char *program) {
    olv_line_t *lines;
    int len;

    lines = lexe_program(program, 1, &len);
    execute_lines(lines, len, NULL);

    free(lines);
}

int does_syntax_fail(const char *program) {
    olv_line_t *lines = lexe_program(program, 1, NULL);

    // check if all 'IF', 'WHILE', 'FOR' and 'FUNC' have a matching 'END'
    int open = 0;
    for (int i = 0; lines[i].str != NULL; i++) {
        if (does_startwith(lines[i].str, "IF")    ||
            does_startwith(lines[i].str, "WHILE") ||
            does_startwith(lines[i].str, "FOR")   ||
            does_startwith(lines[i].str, "FUNC")  ||
            does_startwith(lines[i].str, "ELSE")
        ) open++;
        else if (does_startwith(lines[i].str, "END"))
            open--;
    }
    free(lines);

    return open > 0;
}

/***********************
 *                    *
 *  Lexing Functions  *
 *                    *
***********************/

olv_line_t *lexe_program(const char *program, int make_internal, int *len) {
    int program_len, tmp_index, line_index = 1;

    if (len != NULL)
        *len = 0;

    for (int i = 0; program[i] != '\0'; i++) {
        if (program[i] == '\n' || program[i] == ';') {
            line_index++;
        }
    }

    tmp_index = (line_index + 1) * sizeof(olv_line_t);
    line_index = 0;

    program_len = strlen(program) + 1;
    char *tmp = malloc(program_len);

    olv_line_t *lines = calloc(tmp_index + (program_len), 1);

    char *line_ptr = (char *) lines + tmp_index;
    int fileline, in_quote, i;

    fileline = tmp_index = in_quote = i = 0;

    if (strncmp(program, "#!", 2) == 0) {
        for (; program[i] && program[i] != '\n'; i++);
        fileline++;
    }

    while (IS_SPACE_CHAR(program[i]))
        i++;

    for (; program[i]; i++) {
        if (program[i] == '\n' && in_quote) {
            raise_error_line(fileline, NULL, "Missing closing quote");
            lines[0].str = NULL;
            free(tmp);
            return lines;
        }

        if (program[i] == '\n' || (program[i] == ';' && !in_quote)) {
            while (tmp_index > 0 && IS_SPACE_CHAR(tmp[tmp_index - 1]))
                tmp_index--;

            if (tmp_index) {
                tmp[tmp_index++] = '\0';

                lines[line_index].fileline = fileline + 1;
                lines[line_index].str = line_ptr;

                strcpy(line_ptr, tmp);

                line_ptr += tmp_index;
                tmp_index = 0;

                line_index++;
            }

            if (program[i] == '\n') {
                fileline++;
            }

            while (IS_SPACE_CHAR(program[i + 1])) {
                if (program[++i] == '\n')
                    fileline++;
            }
            continue;
        }

        // remove comments
        if (!in_quote && program[i] == '/' && program[i + 1] == '/') {
            while (
                program[i + 1] != '\0' &&
                program[i + 1] != '\n' &&
                program[i + 1] != ';'
            ) i++;
            continue;
        }

        if (!make_internal) {
            tmp[tmp_index] = program[i];
            goto lexe_end;
        }

        if (program[i] == USER_QUOTE) {
            in_quote = !in_quote;
            tmp[tmp_index++] = INTR_QUOTE;
            continue;
        }

        if (program[i] == USER_VARDF) {
            tmp[tmp_index++] = INTR_VARDF;
            continue;
        }

        #if ENABLE_WILDC
        if (program[i] == USER_WILDC && !in_quote) {
            tmp[tmp_index++] = INTR_WILDC;
            continue;
        }
        #endif

        // interpret double backslashes
        if (program[i] != '\\' || !make_internal) {
            tmp[tmp_index] = program[i];
            goto lexe_end;
        }

        switch (program[++i]) {
            case 'n':
                tmp[tmp_index] = '\n';
                break;
            case 't':
                tmp[tmp_index] = '\t';
                break;
            case 'r':
                tmp[tmp_index] = '\r';
                break;
            case 'a':
                tmp[tmp_index] = '\a';
                break;
            case 'e':
                tmp[tmp_index] = '\e';
                break;
            case '\\':
                tmp[tmp_index] = '\\';
                break;
            case USER_WILDC:
                tmp[tmp_index] = USER_WILDC;
                break;
            case USER_QUOTE:
                tmp[tmp_index] = USER_QUOTE;
                break;
            case USER_VARDF:
                tmp[tmp_index] = USER_VARDF;
                break;
            case ';':
                tmp[tmp_index] = ';';
                break;
            default:
                if (program[i] == '\0')
                    raise_error_line(fileline, NULL, "Backslash at end of line");
                else
                    raise_error_line(fileline, NULL, "Invalid escape sequence '\\%c'", program[i]);
                lines[0].str = NULL;
                free(tmp);
                return lines;
        }

        lexe_end:
        if (!in_quote && IS_SPACE_CHAR(tmp[tmp_index])) {
            if (tmp_index && IS_SPACE_CHAR(tmp[tmp_index - 1]))
                continue;
            tmp[tmp_index] = ' ';
        }

        tmp_index++;
    }

    if (tmp_index != 0) {
        // remove trailing spaces
        while (tmp_index > 0 && IS_SPACE_CHAR(tmp[tmp_index - 1]))
            tmp_index--;
        tmp[tmp_index++] = '\0';

        lines[line_index].fileline = fileline + 1;
        lines[line_index++].str = line_ptr;

        strcpy(line_ptr, tmp);
    }

    free(tmp);

    if (len != NULL)
        *len = line_index;

    return lines;
}

/***********************
 *                    *
 *  Program Printing  *
 *                    *
***********************/

char *get_func_color(const char *str) {
    // keywords: purple
    for (int i = 0; keywords[i] != NULL; i++) {
        if (local_strncmp_nocase(str, keywords[i], -1) == 0) {
            return "95";
        }
    }

    // pseudos: dark blue
    if (pseudos && get_pseudo(str) != NULL) {
        return "44";
    }

    // internal functions: cyan
    if (get_if_function(str) != NULL) {
        return "96";
    }

    if (functions == NULL) {
        return "94";
    }

    // functions: dark cyan
    if (get_function(str) != NULL) {
        return "36";
    }

    // bin: blue
    if (in_bin_names(str)) {
        return "94";
    }

    // unknown functions: dark red
    return "31";
}

#define olv_print_ivfc(str, is_var) do {   \
        if (!is_var) fputs(tmp, stdout);   \
        else printf("\e[93m%s\e[0m", tmp); \
    } while (0)

void olv_print(const char *str, int len) {
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

    while (str[0] == ' ' && len) {
        putchar(' ');
        str++;
        len--;
    }

    if (str[0] == '/' && str[1] == '/') {
        fputs("\e[32m", stdout);
        for (; str[i] != ';' && i < len; i++) {
            if (i == len) return;
            putchar(str[i]);
        }
        fputs("\e[0m", stdout);
        olv_print(str + i, len - i);
        return;
    }

    char *tmp = malloc(len + 1);
    while (
        !(
            str[i] == USER_VARDF ||
            str[i] == ' '        ||
            str[i] == ';'
        ) && i != len
    ) i++;

    memcpy(tmp, str, i);
    tmp[i] = '\0';
    printf("\e[%sm%s\e[0m", get_func_color(tmp), tmp);

    int from = i;
    int in_quote = 0;
    for (; i < len; i++) {
        if (!in_quote && i < len - 1 && str[i] == '/' && str[i+1] == '/') {
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                olv_print_ivfc(tmp, is_var);
            }
            olv_print(str + i, len - i);
            free(tmp);
            return;
        }

        if (str[i] == USER_QUOTE && (i == 0 || str[i - 1] != '\\')) {
            in_quote = !in_quote;
        }

        if (str[i] == USER_VARDF && str[i + 1] == '(' && (i == 0 || str[i - 1] != '\\')) {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                olv_print_ivfc(tmp, is_var);
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

            printf("\e[9%cm!(\e[0m", (j == len) ? '1' : '2');
            olv_print(str + i + 2, j - i - 2);

            if (j != len) {
                fputs("\e[92m)\e[0m", stdout);
            }

            i = j;
            from = i + 1;
        }

        else if (!in_quote && str[i] == ';' && (i == 0 || str[i - 1] != '\\')) {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                olv_print_ivfc(tmp, is_var);
                from = i;
            }
            fputs("\e[37m;\e[0m", stdout);
            olv_print(str + i + 1, len - i - 1);
            free(tmp);
            return;
        }

        else if (str[i] == '|') {
            if (!(i && (str[i + 1] == str[i - 1] && (str[i + 1] == ' ' || str[i + 1] == USER_QUOTE))) &&
                !(i == len - 1 && i && str[i - 1] == ' ')
            ) continue;

            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                olv_print_ivfc(tmp, is_var);
                from = i;
            }

            fputs("\e[37m|\e[0m", stdout);
            if (str[i + 1] == USER_QUOTE) {
                putchar(USER_QUOTE);
                i++;
            }
            olv_print(str + i + 1, len - i - 1);
            free(tmp);
            return;
        }

        // variable
        else if (str[i] == USER_VARDF && (i == 0 || str[i - 1] != '\\')) {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                olv_print_ivfc(tmp, is_var);
                from = i;
            }
            is_var = 1;
        }

        else if (!IS_NAME_CHAR(str[i]) && str[i] != '#') {
            // print from from to i
            if (from != i) {
                memcpy(tmp, str + from, i - from);
                tmp[i - from] = '\0';
                olv_print_ivfc(tmp, is_var);
                from = i;
            }
            is_var = 0;
        }
    }

    if (from != i) {
        memcpy(tmp, str + from, i - from);
        tmp[i - from] = '\0';
        olv_print_ivfc(tmp, is_var);
    }

    free(tmp);
}

/********************
 *                 *
 *  prompt render  *
 *                 *
********************/

#if USE_READLINE

char *render_prompt(char *output, int output_size) {
    int output_i = 0;
    for (int i = 0; g_prompt[i] != '\0'; i++) {
        if (output_i >= output_size - 1)
            break;
        if (g_prompt[i] != '$') {
            output[output_i++] = g_prompt[i];
            continue;
        }
        switch (g_prompt[i + 1]) {
            case 'v':
                for (int j = 0; OLV_VERSION[j] != '\0' && output_i < output_size - 1; j++)
                    output[output_i++] = OLV_VERSION[j];
                break;
            case 'd':
                for (int j = 0; g_current_directory[j] != '\0' && output_i < output_size - 1; j++)
                    output[output_i++] = g_current_directory[j];
                break;
            case '(':
                if (g_exit_code[0] != '0')
                    break;
                for (; g_prompt[i] != ')'; i++);
                i--;
                break;
            case '{':
                if (g_exit_code[0] == '0')
                    break;
                for (; g_prompt[i] != '}'; i++);
                i--;
                break;
            case ')':
                break;
            case '}':
                break;
            default:
                output[output_i++] = g_prompt[i];
                break;
        }
        i++;
    }
    output[output_i] = '\0';
    return output;
}

#else

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
                if (g_exit_code[0] != '0')
                    break;
                for (; g_prompt[i] != ')'; i++);
                i--;
                break;
            case '{':
                if (g_exit_code[0] == '0')
                    break;
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

#endif

/*********************
 *                  *
 *  profanOS input  *
 *                  *
*********************/

#if PROFANBUILD

// input() setings
#define SLEEP_T 11
#define FIRST_L 12

#define SC_MAX 57

int add_to_suggest(char **suggests, int count, char *str) {
    if (count < MAX_SUGGESTS) {
        suggests[count] = strdup(str);
        count++;
        suggests[count] = NULL;
        return count;
    }
    return count;
}

char *olv_autocomplete(const char *str, int len, char **other, int *dec_ptr) {
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
        if (str[j] == USER_QUOTE)
            is_var = !is_var;
        if (str[j] == USER_VARDF && str[j + 1] == '(') {
            dec = j + 2;
            i = dec;
            j++;
        } else if (str[j] == ';' && !is_var) {
            dec = j + 1;
            i = dec;
        }
    }

    is_var = 0;

    while (str[i] == ' ' && i != len) {
        dec++;
        i++;
    }

    // check if we are in the middle of a variable
    int in_var = 0;
    for (int j = i; j < len; j++) {
        if (str[j] == USER_VARDF) {
            in_var = j + 1;
        } else if (str[j] == ' ' || str[j] == USER_QUOTE) {
            in_var = 0;
        }
    }

    if (in_var) i++;

    while (
        !(
            str[i] == USER_VARDF ||
            str[i] == ' '        ||
            str[i] == USER_QUOTE
        ) && i != len
    ) i++;

    if (i - dec == 0) {
        return NULL;
    }

    // path
    if (i < len && !in_var) {
        // ls the current directory
        char *path = malloc(MAX_PATH_SIZE);
        char *inp_end = malloc(MAX_PATH_SIZE);

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

        char *tmp = profan_join_path(g_current_directory, inp_end);
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
        uint32_t dir = fu_path_to_sid(SID_ROOT, path);
        free(path);

        if (IS_SID_NULL(dir) || !fu_is_dir(dir)) {
            free(inp_end);
            return NULL;
        }

        // get the directory content
        char **names;
        uint32_t *out_ids;

        int elm_count = fu_get_dir_content(dir, &out_ids, &names);

        for (int j = 0; j < elm_count; j++) {
            if (names[j][0] == '.' && inp_end[0] != '.') continue;
            if (strncmp(names[j], inp_end, dec) == 0) {
                tmp = malloc(strlen(names[j]) + 2);
                strcpy(tmp, names[j]);
                if (fu_is_dir(out_ids[j]))
                    strcat(tmp, "/");
                suggest = add_to_suggest(other, suggest, tmp);
                free(tmp);
            }
        }

        for (int j = 0; j < elm_count; j++)
            profan_kfree(names[j]);
        profan_kfree(out_ids);
        profan_kfree(names);
        free(inp_end);

        if (suggest == 1) {
            ret = strdup(other[0] + dec);
            free(other[0]);
            return ret;
        }

        return NULL;
    }

    tmp = malloc(len + 1);

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

    int is_upper;
    char *lower;

    // keywords
    for (int j = 0; keywords[j] != NULL; j++) {
        if (local_strncmp_nocase(tmp, keywords[j], i - dec) == 0) {
            is_upper = 1;
            for (int k = 0; k < i - dec; k++) {
                if (tmp[k] >= 'a' && tmp[k] <= 'z') {
                    is_upper = 0;
                    break;
                }
            }
            if (is_upper) {
                suggest = add_to_suggest(other, suggest, keywords[j]);
                continue;
            }
            lower = calloc(strlen(keywords[j]) + 1, sizeof(char));
            for (int k = 0; keywords[j][k] != '\0'; k++) {
                if (k < i - dec)
                    lower[k] = tmp[k];
                else
                    lower[k] = LOWERCASE(keywords[j][k]);
            }
            suggest = add_to_suggest(other, suggest, lower);
            free(lower);
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

    // bin
    #if BIN_AS_PSEUDO
    if (bin_names) {
        for (int j = 0; bin_names[j] != NULL; j++) {
            if (strncmp(tmp, bin_names[j], i - dec) == 0) {
                suggest = add_to_suggest(other, suggest, bin_names[j]);
            }
        }
    }
    #endif

    free(tmp);

    if (suggest == 1) {
        ret = strdup(other[0] + i - dec);
        free(other[0]);
    }

    return ret;
}

int input_local_profan(char *buffer, int size, char **history, int history_end, int buffer_index) {
    // return -1 if the input is valid, else return the cursor position

    #if USE_ENVVARS
    char *term = getenv("TERM");
    if (term && strcmp(term, "/dev/panda") && strcmp(term, "/dev/kterm")) {
        if (fgets(buffer, size, stdin))
            return -1;
        puts("");
        return -2;
    }
    #endif

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
    while (sc != KB_ENTER) {
        usleep(SLEEP_T * 1000);
        sc = syscall_sc_get();

        if (sc == KB_RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        key_ticks = (sc != last_sc) ? 0 : key_ticks + 1;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            continue;
        }

        if (sc == KB_LSHIFT || sc == KB_RSHIFT) {
            shift = 1;
            continue;
        }

        if (sc == KB_LSHIFT + 128 || sc == KB_RSHIFT + 128) {
            shift = 0;
            continue;
        }

        if (sc == KB_LEFT) {
            if (!buffer_index) continue;
            buffer_index--;
        }

        else if (sc == KB_RIGHT) {
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
        }

        else if (sc == KB_BACK) {
            if (!buffer_index) continue;
            buffer_index--;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == KB_DEL) {
            if (buffer_index == buffer_actual_size) continue;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == KB_TOP) {
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

        else if (sc == KB_BOT) {
            // read from history
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

        else if (sc == KB_TAB) {
            int dec;
            char *suggestion = olv_autocomplete(buffer, buffer_index, other_suggests, &dec);
            if (suggestion == NULL && other_suggests[0] != NULL) {
                // print other suggestions
                char *common_beginning = strdup(other_suggests[0] + dec);
                putchar('\n');

                for (int i = 0; other_suggests[i] != NULL; i++) {
                    fputs(other_suggests[i], stdout);
                    if (other_suggests[i + 1] != NULL) {
                        fputs("   ", stdout);
                    }
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
            if (buffer_actual_size + suggestion_len >= size)
                continue;
            for (int i = buffer_actual_size; i >= buffer_index; i--)
                buffer[i + suggestion_len] = buffer[i];
            for (int i = 0; i < suggestion_len; i++)
                buffer[buffer_index + i] = suggestion[i];
            buffer_actual_size += suggestion_len;
            buffer_index += suggestion_len;
            free(suggestion);
        }

        else if (sc <= SC_MAX) {
            if (size < buffer_actual_size + 2 ||
                    profan_kb_get_char(sc, shift) == '\0')
                continue;
            for (int i = buffer_actual_size; i > buffer_index; i--)
                buffer[i] = buffer[i - 1];
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
}

/*********************
 *                  *
 *  readline input  *
 *                  *
*********************/

#elif USE_READLINE

int input_local_readline(char *buffer, int size, const char *prompt) {
    static char *last_line = NULL;

    char *line = readline(prompt);
    if (line == NULL) {
        return -2;
    }

    if (line[0] != '\0' && (last_line == NULL || strcmp(line, last_line))) {
        add_history(line);
    }

    free(last_line);
    last_line = line;

    strncpy(buffer, line, size);

    return -1;
}

#endif

/*********************
 *                  *
 *  standard input  *
 *                  *
*********************/

int input_local_stdio(char *buffer, int size) {

    if (fgets(buffer, size, stdin))
        return -1;

    // end of file
    puts("");

    return -2;
}

/***************************
 *                        *
 *  Shell/File functions  *
 *                        *
***************************/

void start_shell(void) {
    // use execute_program() to create a shell
    char *line = malloc(INPUT_SIZE + 1);

    #if PROFANBUILD
    char **history = calloc(HISTORY_SIZE, sizeof(char *));
    int history_index = 0;
    #endif

    int len, is_user, cursor_pos = -1;

    #if UNIXBUILD || PROFANBUILD
    is_user = isatty(STDIN_FILENO);
    #else
    is_user = 1;
    #endif

    while (1) {
        line[0] = '\0';

        if (is_user) {
            #if PROFANBUILD
            do {
                display_prompt();
                cursor_pos = input_local_profan(line, INPUT_SIZE, history, history_index, cursor_pos);
            } while (cursor_pos >= 0);
            #elif USE_READLINE
            cursor_pos = input_local_readline(line, INPUT_SIZE, render_prompt(line, INPUT_SIZE));
            #else
            display_prompt();
            cursor_pos = input_local_stdio(line, INPUT_SIZE);
            #endif
        } else {
            if (!fgets(line, INPUT_SIZE, stdin)) {
                break;
            }
        }

        while (is_user && cursor_pos != -2 && does_syntax_fail(line)) {
            // multiline program
            strcat(line, ";");
            len = strlen(line);
            line[len] = '\0';
            #if PROFANBUILD
            do {
                fputs(OTHER_PROMPT, stdout);
                fflush(stdout);
                cursor_pos = input_local_profan(line + len, INPUT_SIZE - len, history, history_index, cursor_pos);
            } while(cursor_pos >= 0);
            #elif USE_READLINE
            cursor_pos = input_local_readline(line + len, INPUT_SIZE - len, OTHER_PROMPT);
            #else
            fputs(OTHER_PROMPT, stdout);
            fflush(stdout);
            cursor_pos = input_local_stdio(line + len, INPUT_SIZE - len);
            #endif
        }

        if (cursor_pos == -2 || strcmp(line, "shellexit") == 0) {
            puts("Exiting olivine shell, bye!");
            break;
        }

        #if PROFANBUILD
        // add to history if not empty and not the same as the last command
        if (line[0] && (history[history_index] == NULL || strcmp(line, history[history_index]))) {
            history_index = (history_index + 1) % HISTORY_SIZE;

            if (history[history_index] != NULL) {
                free(history[history_index]);
            }

            history[history_index] = strdup(line);
        }
        #endif

        execute_program(line);
    }

    #if PROFANBUILD
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }
    free(history);
    #endif

    #if USE_READLINE
    rl_clear_history();
    #endif

    free(line);
}

int execute_file(const char *file, char **args) {
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        raise_error(NULL, "File '%s' does not exist", file);
        return 1;
    }

    char *line = malloc(INPUT_SIZE + 1);
    char *program = malloc(1);
    program[0] = '\0';

    while (fgets(line, INPUT_SIZE, f) != NULL) {
        // realloc program
        int len = strlen(line);
        program = realloc(program, strlen(program) + len + 1);
        strcat(program, line);
    }

    fclose(f);
    free(line);

    g_current_level++;
    if (set_variable("filename", file)) {
        g_current_level--;
        free(program);
        return 1;
    }

    int argc;
    char tmp[4];
    for (argc = 0; args[argc] != NULL; argc++) {
        local_itoa(argc, tmp);
        if (!set_variable(tmp, args[argc]))
            continue;
        g_current_level--;
        return 1;
    }

    local_itoa(argc, tmp);
    if (set_variable("#", tmp)) {
        g_current_level--;
        return 1;
    }

    execute_program(program);

    // free variables
    del_variable_level(g_current_level--);

    free(program);

    return 0;
}

void print_file_highlighted(const char *file) {
    FILE *f = fopen(file, "r");
    if (f == NULL) {
        raise_error(NULL, "File '%s' does not exist", file);
        return;
    }

    functions = NULL;
    pseudos = NULL;

    char *line = malloc(INPUT_SIZE + 1);
    char *program = malloc(1);
    program[0] = '\0';

    while (fgets(line, INPUT_SIZE, f)) {
        // realloc program
        int len = strlen(line);
        program = realloc(program, strlen(program) + len + 1);
        strcat(program, line);
    }

    fclose(f);
    free(line);

    int tmp, indent = 0;

    olv_line_t *lines = lexe_program(program, 0, NULL);
    for (int i = 0; lines[i].str; i++) {
        const char *l = lines[i].str;
        if ((tmp = does_startwith(l, "END")))
            indent--;

        for (int j = 0; j < indent * 4; j++)
            putchar(' ');

        olv_print(l, strlen(l));
        putchar('\n');
        if (tmp && !indent && lines[i + 1].str) {
            putchar('\n');
            continue;
        }

        if (does_startwith(l, "IF")    ||
            does_startwith(l, "WHILE") ||
            does_startwith(l, "FOR")   ||
            does_startwith(l, "FUNC")  ||
            does_startwith(l, "ELSE")
        ) indent++;
    }
    fputs("\e[0m", stdout);

    free(program);
    free(lines);
}

/************************
 *                     *
 *   Argument Parser   *
 *                     *
************************/

typedef struct {
    char help;
    char version;
    char no_init;
    char inter;
    char print;
    int arg_offset;

    char *file;
    char *command;
} olivine_args_t;

int show_help(int full, const char *name) {
    fprintf(full ? stdout : stderr, "Usage: %s [options] [file] [arg1 arg2 ...]\n", name);
    if (!full) {
        fprintf(stderr, "Try '%s --help' for more information.\n", name);
        return 1;
    }
    puts("Options:\n"
        "  -c, --command  execute argument as code line\n"
        "  -i, --inter    start a shell after executing\n"
        "  -h, --help     show this help message and exit\n"
        "  -n, --no-init  don't execute the init program\n"
        "  -p, --print    show file with syntax highlighting\n"
        "  -v, --version  display program's version number\n"

        "Without file, the program will start a shell.\n"
        "Use 'exec' to execute a file from the shell instead\n"
        "of relaunching a new instance of olivine."
    );
    return 0;
}

void show_version(void) {
    printf(
        "Olivine %s, %s, %s%s%s%s%s\n",
        OLV_VERSION,
        PROFANBUILD   ? "profanOS" : UNIXBUILD ? "Unix" : "Default",
        USE_READLINE  ? "R" : "r",
        BIN_AS_PSEUDO ? "B" : "b",
        USE_ENVVARS   ? "E" : "e",
        STOP_ON_ERROR ? "S" : "s",
        ENABLE_WILDC  ? "W" : "w"
    );
}

olivine_args_t *parse_args(int argc, char **argv) {
    olivine_args_t *args = calloc(1, sizeof(olivine_args_t));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0
            || strcmp(argv[i], "--help") == 0
        ) args->help = 2;
        else if (strcmp(argv[i], "-v") == 0
            || strcmp(argv[i], "--version") == 0
        ) args->version = 1;
        else if (strcmp(argv[i], "-p") == 0
            || strcmp(argv[i], "--print") == 0
        ) args->print = 1;
        else if (strcmp(argv[i], "-n") == 0
            || strcmp(argv[i], "--no-init") == 0
        ) args->no_init = 1;
        else if (strcmp(argv[i], "-i") == 0
            || strcmp(argv[i], "--inter") == 0
        ) args->inter = 1;
        else if (strcmp(argv[i], "-c") == 0
            || strcmp(argv[i], "--command") == 0
        ) {
            if (i + 1 == argc) {
                fputs("Error: no command given", stderr);
                args->help = 1;
                return args;
            }
            args->command = argv[i + 1];
            i++;
        } else if (argv[i][0] != '-') {
            args->file = argv[i];
            args->arg_offset = i + 1;
            break;
        } else {
            fprintf(stderr, "Error: invalid option -- '%s'\n", argv[i]);
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

void olv_init_globals(void) {
    g_current_level = 0;
    g_fileline = -1;

    g_current_directory = malloc(MAX_PATH_SIZE + 1);

    #if UNIXBUILD || PROFANBUILD
    if (getcwd(g_current_directory, MAX_PATH_SIZE) == NULL)
    #endif
    strcpy(g_current_directory, "/");

    strcpy(g_exit_code, "0");

    g_prompt = malloc(PROMPT_SIZE + 1);
    strcpy(g_prompt, DEFAULT_PROMPT);

    variables = calloc(MAX_VARIABLES, sizeof(variable_t));
    pseudos   = calloc(MAX_PSEUDOS,   sizeof(pseudo_t));
    functions = calloc(MAX_FUNCTIONS, sizeof(function_t));
    bin_names = load_bin_names();

    set_variable_global("version", OLV_VERSION);
    set_variable_global("profan", PROFANBUILD ? "1" : "0");
    set_variable("spi", "0");

    set_sync_variable("path", g_current_directory, MAX_PATH_SIZE);
    set_sync_variable("exit", g_exit_code, 4);
    set_sync_variable("prompt", g_prompt, PROMPT_SIZE);
}

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

    if (args->print) {
        if (args->file == NULL) {
            puts("Error: no file given");
            free(args);
            return 1;
        }
        print_file_highlighted(args->file);
        free(args);
        return 0;
    }

    olv_init_globals();

    // init pseudo commands
    if (!args->no_init) {
        execute_program(init_prog);
    }

    if (args->file != NULL && execute_file(args->file, argv + args->arg_offset)) {
        ret_val = 1;
    }

    if (args->command != NULL) {
        execute_program(args->command);
    }

    if (args->inter || (args->file == NULL && args->command == NULL)) {
        start_shell();
    }

    if (local_atoi(g_exit_code, &ret_val))
        ret_val = 1;

    free_functions();
    free_pseudos();
    free_vars();

    free(bin_names);

    free(g_current_directory);
    free(g_prompt);
    free(args);

    return ret_val;
}
