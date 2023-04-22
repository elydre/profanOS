#ifndef OCMLIB_ID
#define OCMLIB_ID 1005

#include <type.h>

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

#define ocm_write ((uint32_t (*)(int, uint8_t)) get_func_addr(OCMLIB_ID, 2))
#define ocm_read ((uint8_t (*)(int, uint32_t)) get_func_addr(OCMLIB_ID, 3))
#define ocm_get_len ((uint32_t (*)(int)) get_func_addr(OCMLIB_ID, 4))
#define ocm_get_last_update ((uint32_t (*)(int)) get_func_addr(OCMLIB_ID, 5))
#define ocm_get_wpos ((uint32_t (*)(int)) get_func_addr(OCMLIB_ID, 6))
#define ocm_set_wpos ((uint32_t (*)(int, uint32_t)) get_func_addr(OCMLIB_ID, 7))

#endif
