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
    VM_image image = load_vm_image("vm_image.sav");
    
    /* Update the VM state */
    add_memory(image.memory_size, image.guest_memory_physical_base);

    restore_from_image(&image, get_vcpufd());

    /* Launch the VM */
    continue_vm();
    return 0;
}
