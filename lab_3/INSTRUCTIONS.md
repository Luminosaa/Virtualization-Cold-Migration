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
There are 2 types of migrations: cold migration and live migration.
The cold migration consists of stopping the target VM, moving it to the target physical machine and finally resuming its execution. Client services deployed in a VM are hold during the migration operation.
The live migration is a more advanced mechanism. During the migration, VM is still running on the source machine. 
In this lab we only consider the cold migration.

## Cold Migration Operation
The cold migration is composed of 2 fundamental operations SAVE and RESTORE.
* SAVE means stopping the VM and dump its current state in a VM image.
    * Pause all the VM vCPUs.
    * Dump the **guest pysical memory**, **vCPU registers** (privileged and unprivileged), **devices** (file descriptors), **pending IO requests**,...
    *  Save the result in a file with a defined format on a persistent storage (i.e. disk).
* RESTORE means resuming the VM execution from its saved image.
    * Create a blank VM.
    * Update the blank VM state with the save image file.
    * Run the vCPUs.


# Instructions

## Step 1: VM state and image file
The VM state represents the current activity of the VM (vCPU registers, physical memory, device state, IO pending request,... ).
The VM image is a format (file) that corresponds to the state of the VM at a given time.

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
./bin/main
OPEN ./ay_caramba 32834 511 - return 8
WRITE 8 a07e 16 - return 16
SAVE OPERATION
$ 
```


## Step 3: RESTORE operation
For this step you are supposed to create a main file (i.e. restore_main.c, similar to the original one) that:
* Create a blank VM.
* Read the content of the VM image file and update the VM components.
* Launch the VM (run the vCPU).

You should have this type of output:
```bash 
$ cd vm_src/
$ make restore
RESTORE OPERATION
WRITE 8 a073 12 - return 12
CLOSE 8 - return 0
EXIT 0
$ 
```


### Bonus
In this bonus part, we ask you to implement a realistic migration.
Instead of storing the VM image on a disk, you have to transfer the VM state from a client VMM to a server VMM.

