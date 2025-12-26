#ifndef __SYSCALL__
#define __SYSCALL__
#pragma once
#include "../memory_manager/memory.h"
#include "../vm_manager/manager.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/random.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/sendfile.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <stdint.h>
#include <stdint.h>
#include <linux/kvm.h>
#include <asm/prctl.h>   /* Definition of ARCH_* constants */
#include <sys/syscall.h> /* Definition of SYS_* constants */
#include <unistd.h>
#define SYS_RET regs->rax
#define SYS_NUM regs->rax
#define ARG1 regs->rdi
#define ARG2 regs->rsi
#define ARG3 regs->rdx
#define ARG4 regs->r10
#define ARG5 regs->r8 // changed
#define ARG6 regs->r9

#include <linux/kvm.h>
#include <stdint.h>
#define SAVE_EXIT_CODE 255
#define MAX_PATH_LEN 256
#define MAX_OPEN_FILES 64
#define MAX_MEMORY_SIZE 0xF0000
#define MAX_MSR_ENTRIES 32

typedef struct
{
    int guest_fd;
    int host_fd;
    int flags;
    uint32_t start_addr;
    uint64_t offset;
    char path[MAX_PATH_LEN];
} guest_file;

typedef struct
{
    /* CPU core state */
    struct kvm_regs registers;
    struct kvm_sregs sregisters;

    /* Model-specific registers */
    struct {
        uint32_t nmsrs;
        struct kvm_msr_entry entries[MAX_MSR_ENTRIES];
    } msrs;

    /* FPU / SIMD state (SSE) */
    struct kvm_fpu fpu;

    /* Memory */
    int64_t memory_size;
    int64_t guest_memory_physical_base;
    uint8_t guest_memory[MAX_MEMORY_SIZE];

    /* Files */
    uint8_t n_open_files;
    guest_file open_files[MAX_OPEN_FILES];

} VM_image;


int syscall_handler(uint8_t *memory, int vcpufd);

int get_host_fd_from_guest_fd(int guest_fd);

void save_vm_image(const char *filename, int vcpufd);

VM_image load_vm_image(const char *filename);

void restore_from_image(VM_image *image, int vcpufd);

void print_image_info(VM_image *image);
#endif
