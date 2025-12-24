# VM Cold migration

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
Memory Size: 61440
Guest Memory Physical Base: 0
Number of Open Files: 1
Open File 0 (./ay_caramba.txt): Guest FD: 8, Host FD: 8, Offset: 16
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
Memory Size: 61440
Guest Memory Physical Base: 0
Number of Open Files: 1
Open File 0 (./ay_caramba.txt): Guest FD: 8, Host FD: 8, Offset: 16
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
