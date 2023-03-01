#include <syscall.h>
#include <i_iolib.h>
#include <i_string.h>
#include <type.h>
#include <stdlib.h>
#define TIME_C
#include <time.h>

int       daylight;
long int  timezone;
char     **tzname;

int main() {
    c_kprint("Init of the time lib !\n");
}

char *asctime(const tm_t *a) {
    fsprint("Asctime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

char *asctime_r(const tm_t *a, char *b) {
    fsprint("Asctime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

clock_t clock(void) {
    fsprint("Clock is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int clock_getres(clockid_t a, timespec_t *n) {
    fsprint("Clock_getres is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int clock_gettime(clockid_t a, timespec_t *b) {
    fsprint("Clock_gettime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int clock_settime(clockid_t a, const timespec_t *b) {
    fsprint("Clock_settime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *ctime(const time_t *a) {
    fsprint("Ctime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

char *ctime_r(const time_t *a, char *b) {
    fsprint("Ctime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

double difftime(time_t a, time_t b) {
    fsprint("Difftime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

tm_t *getdate(const char *a) {
    fsprint("Getdate is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *gmtime(const time_t *a) {
    fsprint("Gmtime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *gmtime_r(const time_t *a, tm_t *b) {
    fsprint("Gmtime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *localtime(const time_t *a) {
    fsprint("Localtime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

tm_t *localtime_r(const time_t *a, tm_t *b) {
    fsprint("Localtime_r is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

time_t mktime(tm_t *a) {
    fsprint("Mktime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int nanosleep(const timespec_t *a, timespec_t *b) {
    fsprint("Nanosleep is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

size_t strftime(char *a, size_t b, const char *c, const tm_t *d) {
    fsprint("Strftime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

char *strptime(const char *a, const char *b, tm_t *c) {
    fsprint("Strptime is not implemented yet, WHY DO YOU USE IT ?\n");
    return NULL;
}

time_t time(time_t *a) {
    fsprint("Time is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_create(clockid_t a, sigevent_t *b, timer_t c) {
    fsprint("Timer_create is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_delete(timer_t a) {
    fsprint("Timer_delete is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_gettime(timer_t a, itimerspec_t *b) {
    fsprint("Timer_gettime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_getoverrun(timer_t a) {
    fsprint("Timer_getoverrun is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

int timer_settime(timer_t a, int b, const itimerspec_t * c, itimerspec_t *d) {
    fsprint("Timer_settime is not implemented yet, WHY DO YOU USE IT ?\n");
    return 0;
}

void tzset(void) {
    fsprint("Tzset is not implemented yet, WHY DO YOU USE IT ?\n");
}

