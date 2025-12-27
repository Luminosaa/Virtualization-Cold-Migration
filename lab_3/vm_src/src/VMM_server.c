#include "../../vm_manager/manager.h"
#include "../../memory_manager/memory.h"
#include "../../vm_migration/migration.h"

#define VM_MEMORY_SIZE 0xF000
#define STACK_ADDRESS 0xE000
#define STACK_SIZE 0x3000


int main(int argc, char *argv[])
{
    int port = 4040;

    int client_fd = server_connection(port);

    VM_image image = receive_vm_image(client_fd);

    print_image_info(&image);

    /* Create a blank VM */
    create_vm();
    create_bootstrap();                  // sets up sregs, GDT
    add_memory(image.memory_size, image.guest_memory_physical_base);
    create_stack(STACK_ADDRESS, STACK_SIZE);

    restore_from_image(&image, get_vcpufd());
    continue_vm();
    
    return 0;
}




