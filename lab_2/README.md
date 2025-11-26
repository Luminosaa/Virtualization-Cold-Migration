# ScratchVM: Lab 2

The objective of this first lab is to be able to run a simple Linux C application inside a micro virtual machine.
Our virtual machine contains 1 vCPU and low amount of physical memory. We do not consider devices (disk, network, peripherals, ...).

## Step 1: Understanding VM application execution flow 
- Deploy and run the virtual machine and describe what you observe (the output).
- Identify and try to explain the VM application execution flow. (Check the files [boot.asm](./vm_src/src/boot.asm) and [manager.c](./vm_manager/manager.c))

## Step 2: Handling VM exit
- Identify and detail the system calls operations. ([Linux System Call Table](https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/))
- Handle the system calls performed by the VM application.


## Step 3: Reverse Engineering
- Try to guess the source code of the VM application.

