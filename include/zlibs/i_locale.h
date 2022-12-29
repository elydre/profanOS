#ifndef LOCALE_ID
#define LOCALE_ID 1007

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)

#define locale_init() (void(*))(int) get_func_addr(LOCALE_ID, 2)
#define locale_reload() (void(*))(int) get_func_addr(LOCALE_ID, 3)
#define locale_get_scancode_from_char(input_character) (int(*)(char)) get_func_addr(LOCALE_ID, 4 /* tbd */)
#define locale_get_char_from_scancode(scancode) (char(*)(int)) get_func_addr(LOCALE_ID, 5 /* tbd */)

#endif