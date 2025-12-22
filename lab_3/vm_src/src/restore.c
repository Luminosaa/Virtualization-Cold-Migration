#include "../../vm_manager/manager.h"
#include "../../load_manager/loader.h"
#include "../../utils/util.h"
#include "../../memory_manager/memory.h"

#define VM_MEMORY_SIZE  0xF000
#define STACK_ADDRESS   0xE000
#define STACK_SIZE      0x3000

/* TODO */
int main(int argc, char *argv)
{
    /* Crate a blank VM */
    create_vm();
    create_bootstrap();
    
    /* Read the VM image file */
    load_vm_image("vm_image.sav", /* vcpufd */ 0);
    /* Update the VM state */

    /* Launch the VM */
    
    return 0;
}
