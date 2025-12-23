## Step 1: VM state and image file
* List all the VM components (vCPU registers, physical memory, device state, IO pending requests,... ) that can be part of the deployed VM state.

vCPU registers (kvm_regs and kvm_sregs)
Memory size and guest physical base address
Raw dump of memory
Number of open files
For every open file: 
    guest fd; file path; open flags; file offset (to resume writes)

Image format: 
```C
#define MAX_PATH_LEN 256
#define MAX_OPEN_FILES 64
#define MAX_MEMORY_SIZE 0xF0000
#define MAX_MSR_ENTRIES 32
#define MAX_CPUID_ENTRIES  64

typedef struct
{
    int guest_fd;
    int host_fd;
    int flags;
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

    /* FPU / SIMD state */
    struct kvm_xsave xsave;

    /* Pending interrupts / exceptions */
    struct kvm_vcpu_events vcpu_events;

    /* Debug registers */
    struct kvm_debugregs debugregs;

    /* CPUID configuration */
    struct {
        uint32_t nent;
        struct kvm_cpuid_entry2 entries[MAX_CPUID_ENTRIES];
    } cpuid;

    /* Local APIC */
    struct kvm_lapic_state lapic;

    /* Memory */
    int64_t memory_size;
    int64_t guest_memory_physical_base;
    uint8_t guest_memory[MAX_MEMORY_SIZE];

    /* Files */
    uint8_t n_open_files;
    guest_file open_files[MAX_OPEN_FILES];

} VM_image;
```


There must be no pointer in the data struct to easily read in / out using. 

Currently, guest file descriptor (fd) are equal to the host fd. But Linux doesn't ensure that we get the same fd at the next start. 
So we need to track physical and virtual fd, in case that they didn't remain the same. 
To keep it simple, and avoid creating even more data structure, we add `int host_fd;` in the `guest_file` structure. 
