# OOPS Message Analysis
The echo "hello_world" > /dev/faulty command was run which resulted in the following log. 
## OOPS Message
<code>
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
Mem abort info:
  ESR = 0x96000045
  EC = 0x25: DABT (current EL), IL = 32 bits
  SET = 0, FnV = 0
  EA = 0, S1PTW = 0
  FSC = 0x05: level 1 translation fault
Data abort info:
  ISV = 0, ISS = 0x00000045
  CM = 0, WnR = 1
user pgtable: 4k pages, 39-bit VAs, pgdp=00000000420c7000
[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
Internal error: Oops: 96000045 [#1] SMP
Modules linked in: hello(O) faulty(O) scull(O)
CPU: 0 PID: 159 Comm: sh Tainted: G           O      5.15.18 #1
Hardware name: linux,dummy-virt (DT)
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
pc : faulty_write+0x14/0x20 [faulty]
lr : vfs_write+0xa8/0x2b0
sp : ffffffc008c9bd80
x29: ffffffc008c9bd80 x28: ffffff80020ea640 x27: 0000000000000000
x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
x23: 0000000040001000 x22: 000000000000000c x21: 0000005594a72b00
x20: 0000005594a72b00 x19: ffffff80020a0d00 x18: 0000000000000000
x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000
x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
x5 : 0000000000000001 x4 : ffffffc0006f7000 x3 : ffffffc008c9bdf0
x2 : 000000000000000c x1 : 0000000000000000 x0 : 0000000000000000
Call trace:
 faulty_write+0x14/0x20 [faulty]
 ksys_write+0x68/0x100
 __arm64_sys_write+0x20/0x30
 invoke_syscall+0x54/0x130
 el0_svc_common.constprop.0+0x44/0xf0
 do_el0_svc+0x40/0xa0
 el0_svc+0x20/0x60
 el0t_64_sync_handler+0xe8/0xf0
 el0t_64_sync+0x1a0/0x1a4
Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
---[ end trace 769748e948f705d7 ]--
</code>

## Analysis
As seen in the first line of the log: <code>Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000</code>; this error message is caused by the kernel attempting to dereference a NULL pointer.  

The mem abort info and data abort info specify the register information at the time of the fault. 
The line <code>CPU: 0 PID: 159 Comm: sh Tainted: G           O      5.15.18 #1</code> denotes the CPU on which the fault occurred; the PID and process name causing the OOPS with the PID and Comm field; and the Tainted: 'G' flag specifies that the kernel was tainted by loading a proprietary module.

<code> pc : faulty_write+0x14/0x20 [faulty]</code> provides the program counter value at the time of the oops. Based on this line, we can deduce that the oops message occurred when executing the faulty_write function. The byte offset at which the crash occurred is 0x14 or 20 bytes into the function while the length of the function is denoted by 0x20 or 32 bytes. 

The Link register and stack pointer contents are displayed as well in the subsequent lines. Following these values are the CPU register contents and the call trace. The call trace provides the list of functions that were called before the oops occurred. The <code> Code: </code> section provides a hex-dump of the machine code that was executing at the time the oops occurred. 

 


