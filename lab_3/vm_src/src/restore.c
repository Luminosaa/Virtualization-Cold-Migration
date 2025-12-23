#include "../../vm_manager/manager.h"
#include "../../load_manager/loader.h"
#include "../../utils/util.h"
#include "../../memory_manager/memory.h"

#define VM_MEMORY_SIZE 0xF000
#define STACK_ADDRESS 0xE000
#define STACK_SIZE 0x3000

/* TODO */
int main(int argc, char *argv[])
{
    VM_image image = load_vm_image("vm_image.sav");

    /* Create a blank VM */
    create_vm();
    create_bootstrap();                  // sets up sregs, GDT
    add_memory(image.memory_size, image.guest_memory_physical_base);
    create_stack(STACK_ADDRESS, STACK_SIZE);

    restore_from_image(&image, get_vcpufd());
    continue_vm();
    return 0;
}

