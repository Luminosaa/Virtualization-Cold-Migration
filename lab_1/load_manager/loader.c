#include "loader.h"
/***
 * TODO
 * This Function Loads The Binary Code In The Guest Physical Memory.
 */
int load_vm_code(const uint8_t *code)
{
    uint8_t *mem = get_memory();
    memcpy(&mem[0x1000], code, 12);
    return 0;
}