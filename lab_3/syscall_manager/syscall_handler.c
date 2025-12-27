#include "stdio.h"
#include "unistd.h"
#include <stddef.h>
#define _GNU_SOURCE
#include "syscall_handler.h"
#include "../vm_migration/migration.h"

guest_file open_files[MAX_OPEN_FILES];
uint8_t n_open_files;
static int restored_from_snapshot = 0;

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
                    if (open_files[i].offset == 0) { // If 1st write 
                        open_files[i].start_addr = arg2;
                    }
                    open_files[i].offset += regs.rax;
                    break;
                }
            }
        }
        printf("VM_APP - WRITE %d %llx %lld - return %lld\n", write_fd, arg2, arg3, regs.rax);
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
            open_files[n_open_files].start_addr = 0;
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
        printf("VM_APP - CLOSE %d - return %lld\n", close_fd, regs.rax);
        break;
    case 60: /* exit */
        printf("VM_APP - EXIT %lld\n", arg1);
        exit(arg1);
        break;
    case 158: /* arch_prctl - USELESS HERE */
        break;
    case 218: /* set_tid_address - USELESS HERE */
        break;
    case 231: /* exit_group - USELESS HERE */
        break;
    case 400: /* SAVE */
        printf("VM_APP - SAVE requested\n");
        if (restored_from_snapshot)
        {
            printf("VM_APP - already restored from snapshot, ignoring SAVE\n");
            restored_from_snapshot = 0;
            break;
        }
        save_vm_image("vm_image.sav", vcpufd);
        exit(0);
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

void dump_msrs(VM_image *image, int vcpufd)
{
    static uint32_t wanted_msrs[] = {
        0xC0000080, /* EFER */
        0xC0000081, /* STAR */
        0xC0000082, /* LSTAR */
    };

    image->msrs.nmsrs = sizeof(wanted_msrs) / sizeof(wanted_msrs[0]);

    for (uint32_t i = 0; i < image->msrs.nmsrs; i++)
    {
        image->msrs.entries[i].index = wanted_msrs[i];
    }

    struct kvm_msrs *msrs =
        alloca(sizeof(struct kvm_msrs) +
               image->msrs.nmsrs * sizeof(struct kvm_msr_entry));

    msrs->nmsrs = image->msrs.nmsrs;
    memcpy(msrs->entries, image->msrs.entries,
           image->msrs.nmsrs * sizeof(struct kvm_msr_entry));

    ioctl(vcpufd, KVM_GET_MSRS, msrs);

    memcpy(image->msrs.entries, msrs->entries,
           image->msrs.nmsrs * sizeof(struct kvm_msr_entry));
}

void dump_fpu(VM_image *image, int vcpufd)
{
    ioctl(vcpufd, KVM_GET_FPU, &image->fpu);
}

void save_vm_image(const char *filename, int vcpufd)
{
    VM_image image = {0};

    dump_registers(&image, vcpufd);
    dump_sregisters(&image, vcpufd);
    dump_msrs(&image, vcpufd);
    dump_fpu(&image, vcpufd);
    dump_memory(&image);
    dump_open_files(&image);

    FILE *f = fopen(filename, "wb");
    fwrite(&image, sizeof(image), 1, f);
    fclose(f);

    print_image_info(&image);

    send_vm_image(&image);
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

void restore_msrs(VM_image *image, int vcpufd)
{
    struct kvm_msrs *msrs =
        alloca(sizeof(*msrs) +
               image->msrs.nmsrs * sizeof(struct kvm_msr_entry));

    msrs->nmsrs = image->msrs.nmsrs;
    memcpy(msrs->entries, image->msrs.entries,
           image->msrs.nmsrs * sizeof(struct kvm_msr_entry));

    ioctl(vcpufd, KVM_SET_MSRS, msrs);
}

void restore_fpu(VM_image *image, int vcpufd)
{
    ioctl(vcpufd, KVM_SET_FPU, &image->fpu);
}

void restore_from_image(VM_image *image, int vcpufd)
{
    /* 1. Memory */
    memcpy(get_memory(), image->guest_memory, image->memory_size);

    /* 3. Special registers */
    ioctl(vcpufd, KVM_SET_SREGS, &image->sregisters);

    /* 4. MSRs */
    restore_msrs(image, vcpufd);

    /* 5. FPU */
    restore_fpu(image, vcpufd);

    /* 7. General registers */
    ioctl(vcpufd, KVM_SET_REGS, &image->registers);

    /* 8. Files */
    uint8_t *mem = get_memory();
    n_open_files = image->n_open_files;
    for (int i = 0; i < n_open_files; i++)
    {
        open_files[i] = image->open_files[i];
        open_files[i].host_fd = open(open_files[i].path, open_files[i].flags);

        char *buff = &mem[open_files[i].start_addr];
        write(open_files[i].host_fd, buff, open_files[i].offset);
    }

    /* Mark that subsequent exits come from a restored VM */
    restored_from_snapshot = 1;
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
    printf("SRegisters:\n");
    printf("CR0: %llx\n", image->sregisters.cr0);
    printf("CR3: %llx\n", image->sregisters.cr3);
    printf("CR4: %llx\n", image->sregisters.cr4);
    printf("MSRs:\n");
    printf("Number of MSRs: %d\n", image->msrs.nmsrs); 
    printf("EFER: %llx\n", image->msrs.entries[0]);
    printf("STAR: %llx\n", image->msrs.entries[1]);
    printf("LSTAR: %llx\n", image->msrs.entries[2]);
    printf("Memory Size: %zd\n", image->memory_size);
    printf("Guest Memory Physical Base: %zd\n", image->guest_memory_physical_base);
    printf("Number of Open Files: %d\n", image->n_open_files);
    for (int i = 0; i < image->n_open_files; i++)
    {
        printf("Open File %d (%s): Guest FD: %d, Host FD: %d, Starting address: %d, Offset: %zd\n",
               i, image->open_files[i].path,
               image->open_files[i].guest_fd,
               image->open_files[i].host_fd,
               image->open_files[i].start_addr,
               image->open_files[i].offset);
    }

    // Dump full memory in mem2.txt
    FILE *memf = fopen("mem2.txt", "w");
    for (int i = 0; i < image->memory_size; i++)
    {
        fprintf(memf, "%02x ", image->guest_memory[i]);
        if ((i + 1) % 16 == 0)
            fprintf(memf, "\n");
    }
    fclose(memf);

    // Dump upcoming instructions at RIP
    uint64_t rip = image->registers.rip;
    int nbytes = 32; // number of bytes to dump
    if (rip + nbytes > (uint64_t)image->memory_size)
        nbytes = image->memory_size - rip;

    printf("Upcoming instructions at RIP=0x%lx:\n", rip);
    for (int i = 0; i < nbytes; i++)
    {
        printf("%02x ", image->guest_memory[rip + i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    if (nbytes % 16 != 0)
        printf("\n");

    printf("==========================\n");
}
