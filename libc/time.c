#include <driver/rtc.h>
#include <cpu/timer.h>
#include <filesystem.h>
#include <string.h>
#include <system.h>
#include <mem.h>
#include <iolib.h>
#include <time.h>

static int boot_time;

int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int time_calc_unix(time_t *time) {
    int seconde_in_year = 31536000;
    int seconde_in_leap_year = 31622400;
    int seconde_in_month[12] = {0, 2678400, 5097600, 7776000, 10368000, 13046400, 15638400, 18316800, 20995200, 23587200, 26265600, 28857600};
    int seconde_in_day = 86400;
    int seconde_in_hour = 3600;
    int seconde_in_minute = 60;

    int start_year = 1970;
    int century = 2000;
    int unix_time = 0;

    for (int i = start_year; i < time->full[5] + century; i++) {
        unix_time += is_leap_year(i) ? seconde_in_leap_year : seconde_in_year;
    }

    unix_time += seconde_in_month[time->full[4] - 1];
    unix_time += is_leap_year(time->full[5] + century) ? seconde_in_year : 0;
    unix_time += (time->full[3] - 1) * seconde_in_day;
    unix_time += time->full[2] * seconde_in_hour;
    unix_time += time->full[1] * seconde_in_minute;
    unix_time += time->full[0];
    return unix_time;
}

int time_gen_unix() {
    time_t time;
    time_get(&time);
    // TODO : faire un parseur de settings + save proprement
    char *path = "/user/settings";
    uint32_t *settings = fs_declare_read_array(path);
    fs_read_file(path, settings);
    char * char_content = fs_declare_read_array(path);
    int char_count;
    for (char_count = 0; settings[char_count] != (uint32_t) -1; char_count++)
        char_content[char_count] = (char) settings[char_count];
    char_content[char_count] = '\0';
    int decalage = 0;
    char nb[3];
    nb[0] = char_content[1]; nb[1] = char_content[2]; nb[2] = '\0';
    decalage = ascii_to_int(nb);
    if (char_content[0] == '-') decalage = -decalage; 
    // TODO : ajouter le décalage a l'heure correctement
    free(char_content);
    free(settings);
    return time_calc_unix(&time);
}

void sleep(int seconds) {
    int start_time = time_gen_unix();
    while (time_gen_unix() - start_time < seconds);
}

void ms_sleep(uint32_t ms) {
    uint32_t start_tick = timer_get_tick();
    while (timer_get_tick() < start_tick + ms / 10) {
        do_nothing();
    }
}

void time_gen_boot() {
    boot_time = time_gen_unix();
}

int time_get_boot() {
    return boot_time;
}
