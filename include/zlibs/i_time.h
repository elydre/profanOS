#ifndef TIME_ID
#define TIME_ID 1004

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
int is_leap_year(int year);
int time_calc_unix(i_time_t *time);
int time_gen_unix();
void time_add(i_time_t *time, int seconde);
void ms_sleep(uint32_t ms);
*/

#define is_leap_year ((int (*)(int)) get_func_addr(TIME_ID, 2))
#define time_calc_unix ((int (*)(i_time_t *)) get_func_addr(TIME_ID, 3))
#define time_gen_unix ((int (*)()) get_func_addr(TIME_ID, 4))
#define time_add ((void (*)(i_time_t *, int)) get_func_addr(TIME_ID, 5))
#define ms_sleep ((void (*)(uint32_t)) get_func_addr(TIME_ID, 6))
#define ms_sleep_perfect ((void (*)(uint32_t)) get_func_addr(TIME_ID, 7))

#endif
