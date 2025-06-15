#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>


char **split(const char *str, char c) {
	char **result = NULL;
	size_t count = 0;
	char *tmp = strdup(str);
	char *last_comma = NULL;

	char *token = strtok(tmp, &c);
	while (token) {
		count++;
		last_comma = token;
		token = strtok(NULL, &c);
	}

	free(tmp);

	result = malloc(sizeof(char *) * (count + 1));
	if (!result)
		return NULL;
	size_t index = 0;
	tmp = strdup(str);
	token = strtok(tmp, &c);
	while (token) {
		result[index++] = strdup(token);
		token = strtok(NULL, &c);
	}
	result[index] = NULL;

	free(tmp);
	return result;
}


void get_host_by_name(const char *name, uint8_t *ip) {
	char **splited = split(name, '.');


}