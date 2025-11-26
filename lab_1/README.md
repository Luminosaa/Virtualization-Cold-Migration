# ScratchVM: Lab 1

The objective of this first lab is to be able to run a binary code inside a micro virtual machine.
Our virtual machine contains 1 vCPU and low amount of physical memory. We do not consider devices (disk, network, peripherals, ...).

## Step 0: Taking control of KVM 
* Browsing the [KVM API documentation](https://www.kernel.org/doc/Documentation/virt/kvm/api.txt)
* Identifying the needed ioctls

## Step 1: Creating a simple virtual machine
- Allocating the guest physical memory
- Creating 1 vCPU

## Step 2: Loading the binary code
- Copying the code in the guest physical memory area

## Step 3: Running the virtual machine
- Setting up vCPU registers
- Running the vCPU
