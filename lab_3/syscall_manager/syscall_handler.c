#define _GNU_SOURCE
#include "syscall_handler.h"

guest_file open_files[MAX_OPEN_FILES];
uint8_t n_open_files;
int syscall_handler(uint8_t *memory, int vcpufd)
{
    struct kvm_regs regs;
    int ret = -1;

    /* Read the vCPU registers */
    ioctl(vcpufd, KVM_GET_REGS, &regs);

    /* Read the system call arguments */
    __u64 arg1 = regs.rdi;
    __u64 arg2 = regs.rsi;
    __u64 arg3 = regs.rdx;
    __u64 arg4 = regs.r10;
    __u64 arg5 = regs.r8;
    __u64 arg6 = regs.r9;

    /** Get the guest physical memory for translation.
     * NB: for simplicity, we implemented an identity guest page table,
     * it means that a X guest virtual page is mapped on the X guest physical page.
     */

    uint8_t *mem = get_memory();
    // printf("Syscall: %lld %lld %llx %lld\n", regs.rax, arg1, arg2, arg3);
    /* The system call number is stored in the RAX register*/
    switch (regs.rax)
    {
    case 1: /* write */
        /**
         * The 2nd argument is a pointer to a string.
         * This pointer corresponds to a guest virtual address (GVA).
         * From the VMM, we have to translate the GVA to host virtual address (HVA).
         * For a given GVA X, the corresponding HVA is MEM+GVA
         * (MEM is the address of the mmap performed by the VMM to allocate guest physical memory).
         */
        char *buff = &mem[arg2];
        int write_fd = get_host_fd_from_guest_fd(arg1);
        if (write_fd < 0)
        {
            regs.rax = -1;
            break;
        }
        regs.rax = write(write_fd, buff, arg3);
        if (regs.rax >= 0)
        {
            // Update the offset in the open_files table
            for (int i = 0; i < n_open_files; i++)
            {
                if (open_files[i].guest_fd == write_fd)
                {
                    open_files[i].offset += regs.rax;
                    break;
                }
            }
        }

        printf("VM_APP - WRITE %lld %llx %lld - return %lld\n", write_fd, arg2, arg3, regs.rax);
        break;
    case 2: /* open */
        /* The same operation is applied to obtain the HVA of the path */
        if (n_open_files >= MAX_OPEN_FILES) // too many open files
        {
            regs.rax = -1;
            break;
        }
        char *path = &mem[arg1];
        regs.rax = open(path, arg2, arg3);
        if (regs.rax >= 0)
        {
            open_files[n_open_files].guest_fd = regs.rax;
            open_files[n_open_files].host_fd = regs.rax;
            open_files[n_open_files].flags = arg2;
            open_files[n_open_files].offset = 0;
            strncpy(open_files[n_open_files].path, path, MAX_PATH_LEN);
            n_open_files++;
        }
        printf("VM_APP - OPEN %s %lld %lld - return %lld\n", path, arg2, arg3, regs.rax);
        break;
    case 3: /* close */
        int close_fd = get_host_fd_from_guest_fd(arg1);
        if (close_fd < 0)
        {
            regs.rax = -1;
            break;
        }
        regs.rax = close(close_fd);
        if (regs.rax == 0)
        {
            // remove from open_files by swapping with the last entry
            for (int i = 0; i < n_open_files; i++)
            {
                if (open_files[i].guest_fd == close_fd)
                {
                    open_files[i] = open_files[n_open_files - 1];
                    n_open_files--;
                    break;
                }
            }
        }
        printf("VM_APP - CLOSE %lld - return %lld\n", close_fd, regs.rax);
        break;
    case 60: /* exit */
        printf("VM_APP - EXIT %lld\n", arg1);
        if (arg1 == SAVE_EXIT_CODE)
        {
            save_vm_image("vm_image.sav", vcpufd);
            arg1 = 0;
        }
        exit(arg1);
        break;
    case 158: /* arch_prctl - USELESS HERE */
        break;
    case 218: /* set_tid_address - USELESS HERE */
        break;
    case 231: /* exit_group - USELESS HERE */
        break;
    default:
        printf("VM_APP - undefind syscall %lld\n", regs.rax);
        break;
    }

    /* Update the vCPU register for the system calls return values */
    ioctl(vcpufd, KVM_SET_REGS, &regs);
    return 1;
}

int get_host_fd_from_guest_fd(int guest_fd)
{
    if (guest_fd == 0)
        return 0; // stdin
    if (guest_fd == 1)
        return 1; // stdout
    if (guest_fd == 2)
        return 2; // stderr
    for (int i = 0; i < n_open_files; i++)
    {
        if (open_files[i].guest_fd == guest_fd)
        {
            return open_files[i].host_fd;
        }
    }
    return -1;
}

void dump_registers(VM_image *image, int vcpufd)
{
    struct kvm_regs regs;
    ioctl(vcpufd, KVM_GET_REGS, &regs);
    // Skip the current
    image->registers = regs;
}

void dump_sregisters(VM_image *image, int vcpufd)
{
    struct kvm_sregs sregs;
    ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    image->sregisters = sregs;
}

void dump_memory(VM_image *image)
{
    uint8_t *memory = get_memory();
    image->memory_size = get_memory_size();
    image->guest_memory_physical_base = get_memory_start();
    memcpy(image->guest_memory, memory, image->memory_size);
}

void dump_open_files(VM_image *image)
{
    image->n_open_files = n_open_files;
    for (int i = 0; i < n_open_files; i++)
    {
        image->open_files[i] = open_files[i];
    }
}

void save_vm_image(const char *filename, int vcpufd)
{
    VM_image image;

    dump_registers(&image, vcpufd);
    dump_sregisters(&image, vcpufd);
    dump_memory(&image);
    dump_open_files(&image);

    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        perror("fopen");
        return;
    }
    fwrite(&image, sizeof(VM_image), 1, f);
    fclose(f);

    print_image_info(&image);
}

VM_image load_vm_image(const char *filename)
{
    VM_image image;

    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        perror("fopen");
        return image;
    }
    fread(&image, sizeof(VM_image), 1, f);
    fclose(f);
    print_image_info(&image);
    return image;
}

void restore_from_image(VM_image *image, int vcpufd)
{
    /* Restore registers */
    ioctl(vcpufd, KVM_SET_REGS, &image->registers);
    ioctl(vcpufd, KVM_SET_SREGS, &image->sregisters);
    printf("Restored registers and sregisters.\n");
    /* Restore memory */
    uint8_t *memory = get_memory();
    memcpy(memory, image->guest_memory, image->memory_size);
    printf("Restored memory.\n");
    /* Restore open files */
    n_open_files = image->n_open_files;
    for (int i = 0; i < n_open_files; i++)
    {
        open_files[i] = image->open_files[i];
        open_files[i].host_fd = open(open_files[i].path, open_files[i].flags);
        // set the current offset
        if (open_files[i].host_fd >= 0)
        {
            lseek(open_files[i].host_fd, open_files[i].offset, SEEK_SET);
        }
    }
    printf("Restored %d open files.\n", n_open_files);
}

void print_image_info(VM_image *image)
{
    printf("===== VM Image Info: ======\n");
    printf("Registers:\n");
    printf("RAX: %llx\n", image->registers.rax);
    printf("RBX: %llx\n", image->registers.rbx);
    printf("RCX: %llx\n", image->registers.rcx);
    printf("RDX: %llx\n", image->registers.rdx);
    printf("RSI: %llx\n", image->registers.rsi);
    printf("RDI: %llx\n", image->registers.rdi);
    printf("RSP: %llx\n", image->registers.rsp);
    printf("RBP: %llx\n", image->registers.rbp);
    printf("RIP: %llx\n", image->registers.rip);
    printf("RFLAGS: %llx\n", image->registers.rflags);
    printf("Memory Size: %lld\n", image->memory_size);
    printf("Guest Memory Physical Base: %llx\n", image->guest_memory_physical_base);
    printf("Number of Open Files: %d\n", image->n_open_files);
    for (int i = 0; i < image->n_open_files; i++)
    {
        printf("Open File %d (%s): Guest FD: %d, Host FD: %d, Offset: %d\n", i, image->open_files[i].path, image->open_files[i].guest_fd, image->open_files[i].host_fd, image->open_files[i].offset);
    }
    printf("==========================\n");
}
