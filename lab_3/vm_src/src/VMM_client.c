#include "../../vm_manager/manager.h"
#include "../../load_manager/loader.h"
#include "../../memory_manager/memory.h"
#include "../../vm_migration/migration.h"

#define VM_MEMORY_SIZE  0xF000
#define STACK_ADDRESS   0xE000
#define STACK_SIZE      0x3000

int main(int argc, char *argv[])
{

    if (argc < 3) {
        printf("No destination VMM has been set. Execution locally using the VM image file only %d\n", argc);
    } else {
        set_dest_VMM(atoi(argv[1]), argv[2]);
    }
    
    /* creation de la machine virtuelle: RAM et vCPUs */
    create_vm();
    create_bootstrap();
    add_memory(VM_MEMORY_SIZE, 0);

    create_stack(STACK_ADDRESS, STACK_SIZE);

    /* Chargement du code du bootstrap */
    uint64_t boot_entry_point = load_file("bin/boot");

    /* Chargement du code de l'application */
    uint64_t app_entry_point = load_file("bin/app_img");

    launch_vm(boot_entry_point, app_entry_point, STACK_ADDRESS);
    
    return 0;
}
