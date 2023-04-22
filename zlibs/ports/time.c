#include <syscall.h>
#include <stdio.h>
#include <type.h>

#define TIME_C
#include <time.h>

int       daylight;
long int  timezone;
char     **tzname;

int main() {
    puts("Init of the time lib !\n");
    return 0;
}

char *asctime(const tm_t *a) {
    puts("Asctime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

char *asctime_r(const tm_t *a, char *b) {
    puts("Asctime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

clock_t clock(void) {
    puts("Clock is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int clock_getres(clockid_t a, timespec_t *n) {
    puts("Clock_getres is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int clock_gettime(clockid_t a, timespec_t *b) {
    puts("Clock_gettime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int clock_settime(clockid_t a, const timespec_t *b) {
    puts("Clock_settime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *ctime(const time_t *a) {
    puts("Ctime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

char *ctime_r(const time_t *a, char *b) {
    puts("Ctime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

double difftime(time_t a, time_t b) {
    puts("Difftime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

tm_t *getdate(const char *a) {
    puts("Getdate is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *gmtime(const time_t *a) {
    puts("Gmtime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *gmtime_r(const time_t *a, tm_t *b) {
    puts("Gmtime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *localtime(const time_t *a) {
    puts("Localtime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *localtime_r(const time_t *a, tm_t *b) {
    puts("Localtime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

time_t mktime(tm_t *a) {
    puts("Mktime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int nanosleep(const timespec_t *a, timespec_t *b) {
    puts("Nanosleep is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

size_t strftime(char *a, size_t b, const char *c, const tm_t *d) {
    puts("Strftime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *strptime(const char *a, const char *b, tm_t *c) {
    c_serial_print(SERIAL_PORT_A, "Strptime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

time_t time(time_t *a) {
    puts("Time is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_create(clockid_t a, sigevent_t *b, timer_t c) {
    puts("Timer_create is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_delete(timer_t a) {
    puts("Timer_delete is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_gettime(timer_t a, itimerspec_t *b) {
    puts("Timer_gettime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_getoverrun(timer_t a) {
    puts("Timer_getoverrun is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_settime(timer_t a, int b, const itimerspec_t * c, itimerspec_t *d) {
    puts("Timer_settime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

void tzset(void) {
    puts("Tzset is not implemented yet, WHY DO YOU USE IT ?\n");
}

