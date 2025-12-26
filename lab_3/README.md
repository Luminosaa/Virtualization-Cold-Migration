# VM Cold migration

## Hierarchy

Below you can find the folders / files that we added / modified to complete the lab.

- ***syscall_manager:*** There is the functions to save / restore from an vm image. Furthermore, we have the vm image format.
- ***vm_migration:*** You can find the files that have the network functions.
- ***vm_src/src:*** You will see the main programs to launch save / restore operations.
    - *save.c:* The basic save to a file locally (up to step 3).
    - *restore.c:* The basic restore from a fil locally (up to step 3).
    - *VMM_client.c:* The client VMM to save the VM and sending it with network.
    - *VMM_server.c:* The server VMM to restore the VM by restoring it with network.

## Launch

Before starting,  you should be inside the following directory: `vm_src/`

### Save operation

The save operation have for `%rax` (syscall number) **400**.

``` bash
make save
```

#### Expected output

The output on the terminal of the command should look like:
``` bash
VM_APP - OPEN ./ay_caramba.txt 32834 511 - return 8
VM_APP - WRITE 8 a349 16 - return 16
VM_APP - SAVE requested
===== VM Image Info: ======
Registers:
RAX: 190
RBX: 0
RCX: 9979
RDX: 99bd
RSI: 10
RDI: a349
RSP: ddd8
RBP: ddd8
RIP: 80a1
RFLAGS: 212
SRegisters:
CR0: e0000011
CR3: 1000
CR4: 620
MSRs:
Number of MSRs: 3
EFER: c0000080
STAR: c0000081
LSTAR: c0000082
Memory Size: 61440
Guest Memory Physical Base: 0
Number of Open Files: 1
Open File 0 (./ay_caramba.txt): Guest FD: 8, Host FD: 8, Starting address: 41801, Offset: 16
Upcoming instructions at RIP=0x80a1:
48 0f 07 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 ff ff 00 00 00 9b cf 00 ff
==========================
```

You should also find in the `vm_src/` directory files called:
- `vm_image.sav` corresponding to the saved vm image 
- `mem2.txt` that is the dump of the full memory.
- `ay_caramba.txt` that is the output of the program. 
  - You shoud see as output in the file, after the save:
  ``` text 
  Hello World !!!
  ```

### Restore operation

``` bash
make restore
```

#### Expected output

The output on the terminal of the command should look like:
``` bash
===== VM Image Info: ======
Registers:
RAX: 190
RBX: 0
RCX: 9979
RDX: 99bd
RSI: 10
RDI: a349
RSP: ddd8
RBP: ddd8
RIP: 80a1
RFLAGS: 212
SRegisters:
CR0: e0000011
CR3: 1000
CR4: 620
MSRs:
Number of MSRs: 3
EFER: c0000080
STAR: c0000081
LSTAR: c0000082
Memory Size: 61440
Guest Memory Physical Base: 0
Number of Open Files: 1
Open File 0 (./ay_caramba.txt): Guest FD: 8, Host FD: 8, Starting address: 41801, Offset: 16
Upcoming instructions at RIP=0x80a1:
48 0f 07 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 ff ff 00 00 00 9b cf 00 ff
==========================
VM_APP - WRITE 6 a35a 12 - return 12
File written successfully.
VM_APP - WRITE 1 a378 28 - return 28
VM_APP - CLOSE 6 - return 0
VM_APP - EXIT 0
```

You should also find in the `vm_src/` directory files called:
- `ay_caramba.txt` that is the output of the program. 
  - You shoud see as output in the file, after the restore:
  ``` text 
  Hello World !!!
  Bye Bye !!!
  ```

## Network (Bonus)

Time for the bonus, the idea is pretty the same from before, but this time we will send our image through network.

### Save operation

The save operation have for `%rax` (syscall number) **400**.

``` bash
make save_client ARGS="{port} {ip adrress of server_VMM}"
```

#### Expected output

The output on the terminal of the command should look like (with `ARGS="4040 127.0.0.1"`):
``` bash
VM_APP - OPEN ./ay_caramba.txt 32834 511 - return 8
VM_APP - WRITE 8 a349 16 - return 16
VM_APP - SAVE requested
===== VM Image Info: ======
Registers:
RAX: 190
RBX: 0
RCX: 9979
RDX: 99bd
RSI: 10
RDI: a349
RSP: ddd8
RBP: ddd8
RIP: 80a1
RFLAGS: 212
SRegisters:
CR0: e0000011
CR3: 1000
CR4: 620
MSRs:
Number of MSRs: 3
EFER: c0000080
STAR: c0000081
LSTAR: c0000082
Memory Size: 61440
Guest Memory Physical Base: 0
Number of Open Files: 1
Open File 0 (./ay_caramba.txt): Guest FD: 8, Host FD: 8, Starting address: 41801, Offset: 16
Upcoming instructions at RIP=0x80a1:
48 0f 07 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 ff ff 00 00 00 9b cf 00 ff
==========================
connect failed, retry in 1 second...
connect failed, retry in 1 second...
connect failed, retry in 1 second...
Connected to VMM in 127.0.0.1:4040
Sent 1002369 bytes
```

You should also find in the `vm_src/` directory files called:
- `vm_image.sav` corresponding to the saved vm image 
- `mem2.txt` that is the dump of the full memory.
- `ay_caramba.txt` that is the output of the program. 
  - You shoud see as output in the file, after the save:
  ``` text 
  Hello World !!!
  ```

### Restore operation

``` bash
make restore_server
```

It creates a connection / socket on port **4040**.

#### Expected output

The output on the terminal of the command should look like:
``` bash
Waiting for VM image on port 4040...
Received 1002369 bytes
===== VM Image Info: ======
Registers:
RAX: 190
RBX: 0
RCX: 9979
RDX: 99bd
RSI: 10
RDI: a349
RSP: ddd8
RBP: ddd8
RIP: 80a1
RFLAGS: 212
SRegisters:
CR0: e0000011
CR3: 1000
CR4: 620
MSRs:
Number of MSRs: 3
EFER: c0000080
STAR: c0000081
LSTAR: c0000082
Memory Size: 61440
Guest Memory Physical Base: 0
Number of Open Files: 1
Open File 0 (./ay_caramba.txt): Guest FD: 8, Host FD: 8, Starting address: 41801, Offset: 16
Upcoming instructions at RIP=0x80a1:
48 0f 07 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 ff ff 00 00 00 9b cf 00 ff
==========================
VM_APP - WRITE 6 a35a 12 - return 12
File written successfully.
VM_APP - WRITE 1 a378 28 - return 28
VM_APP - CLOSE 6 - return 0
VM_APP - EXIT 0
```

You should also find in the `vm_src/` directory files called:
- `ay_caramba.txt` that is the output of the program. 
  - You shoud see as output in the file, after the restore:
  ``` text 
  Hello World !!!
  Bye Bye !!!
  ```

