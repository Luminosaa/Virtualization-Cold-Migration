# ScratchVM: Lab 3 -- VM Migration

The objective of this final lab is to migrate a micro virtual machine that runs a simple Linux C application.
This virtual machine is composed of 1 vCPU and low amount of physical memory. We do not consider devices (disk, network, peripherals, ...).

# Background
VM migration is a mechanism performed by the hypervisor that consists of moving a virtual machine from a physical machine to an other.
Thhis mechanism is usually used in the CLOUD for:
* maintenance -- when a server crashes, you have to migrate the VMs deployed on it.
* performance -- VMs that communicate with each other can be gather on the same physical machin to reduce communication latencies 
* power saving and consolidation -- by gathering VMs on the same physical machine, you can maximize the efficiency and turn off the empty physical machines.

## VM Migration
There are 2 types of migrations: **cold** migration and **live** migration.
The cold migration consists of **stopping the target VM**, **moving it** to the target physical machine and finally **resuming its execution**. Applications deployed in a VM are hold during the migration operation.
The live migration is a more advanced mechanism. During the migration, VM is **still running** on the source physical machine so the latency of the migration is shorter than the cold migration. 
In this lab we **do not consider** live migration.

## Cold Migration Operation
The cold migration is composed of 2 fundamental operations SAVE and RESTORE.
* SAVE means stopping the VM and dump its current state in a VM image.
    * Pause all the VM **vCPUs**.
    * Dump the **guest pysical memory**, **vCPU registers** (privileged and unprivileged), **devices** (file descriptors), **pending IO requests**,...
    *  **Send** (network) the dumped informations to the VMM on the target physical machine.
* RESTORE means resuming the VM execution from its saved image.
    * Create a **blank VM**.
    * **Update** the blank VM state with the saved **image file**.
    * Run the vCPUs.


# Instructions
Your objective is to implement a **simplier version of cold migration**.
For the SAVE operation, instead of sending the VM dumped information to the target physical machine, the VMM just **stores the informations in a VM image file on a persistant storage (disk)**.
For the RESTORE operation, the VMM does not wait for VM information through the network. Instead, it will **create and deploy a new VM instance** with the information stored in the VM image file.


## Step 1: VM state and image file
The VM state represents the current activity of the VM components (vCPU registers, physical memory, device state, IO pending request,... ).
The VM image is a format (file) that corresponds to the current state of the VM.

* List all the VM components (vCPU registers, physical memory, device state, IO pending requests,... ) that can be part of the deployed VM state.
* Define your VM image format justify your choice.
* Implementing a dump () function for each component (vCPU registers, physical memory, device state, IO pending requests,... ).

## Step 2: SAVE operation
The VM application explicitely request a SAVE operation ([app.c:21](./vm_src/src/app.c)).
* Implement the **SAVE()** function that triggers a VM trap to stop the vCPU.
* Catch and handle this VM trap by performing the VM state dump.
* Save the VM state dump on a persistent device (disk,...) and exit the VMM.

You shoule have this type of output:
```bash
$ cd vm_src/
$ make save
./bin/save
OPEN ./ay_caramba 32834 511 - return 8
WRITE 8 a07e 16 - return 16
SAVE OPERATION
$ 
```
The VMM exits after the performing the SAVE operation.

## Step 3: RESTORE operation
For this step you are supposed to create a main file (i.e. restore_main.c, similar to the original one) that:
* Create a blank VM.
* Read the content of the VM image file and update the VM components.
* Launch the VM (run the vCPU).

You should have this type of output:
```bash 
$ cd vm_src/
$ make restore
./bin/restore
RESTORE OPERATION
WRITE 8 a073 12 - return 12
CLOSE 8 - return 0
EXIT 0
$ 
```


# Bonus
In this bonus part, we ask you to implement a realistic migration.
You will run 2 VMMs: VMM_client and VMM_server.
The first one deploys and run the provided VM.
During the SAVE operation, the VMM_client sends the VM state to the VMM_server. This latter creates a blank VM and updates its state with the received information. It then resumes the execution of the VM.

You should have this type of output:

**VMM CLIENT TERMINAL**
```bash
VMM_CLIENT: VM_APP - OPEN ./ay_caramba 32834 511 - return 8
VMM_CLIENT: VM_APP - WRITE 8 a07e 16 - return 16
VMM_CLIENT: VM_APP - SAVE OPERATION
VMM_CLIENT: VM_APP - VM STATE SENT
VMM_CLIENT: EXITING
```

**VMM CLIENT TERMINAL**
```bash
VMM_SERVER: VM_APP - RESTORE OPERATION
VMM_SERVER: VM_APP - WRITE 8 a073 12 - return 12
VMM_SERVER: VM_APP - CLOSE 8 - return 0
VMM_SERVER: VM_APP - EXIT 0
```