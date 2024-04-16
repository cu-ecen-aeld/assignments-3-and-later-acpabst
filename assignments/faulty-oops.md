# FAULTY OOPS
Oops message:
```
root@qemuarm64:~# echo "hello_world" > /dev/faulty
1  [   45.903910] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
2  [   45.904440] Mem abort info:
3  [   45.904573]   ESR = 0x0000000096000045
4  [   45.904764]   EC = 0x25: DABT (current EL), IL = 32 bits
5  [   45.904970]   SET = 0, FnV = 0
6  [   45.905150]   EA = 0, S1PTW = 0
7  [   45.905301]   FSC = 0x05: level 1 translation fault
8  [   45.906450] Data abort info:
9  [   45.906685]   ISV = 0, ISS = 0x00000045
10 [   45.907087]   CM = 0, WnR = 1
11 [   45.907535] user pgtable: 4k pages, 39-bit VAs, pgdp=0000000043dac000
12 [   45.908126] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
13 [   45.909282] Internal error: Oops: 0000000096000045 [#1] PREEMPT SMP
14 [   45.910426] Modules linked in: scull(O) faulty(O) hello(O)
15 [   45.911298] CPU: 1 PID: 345 Comm: sh Tainted: G           O      5.15.150-yocto-standard #1
16 [   45.911552] Hardware name: linux,dummy-virt (DT)
17 [   45.911986] pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
18 [   45.912340] pc : faulty_write+0x18/0x20 [faulty]
19 [   45.913123] lr : vfs_write+0xf8/0x29c
20 [   45.913494] sp : ffffffc00bc53d80
21 [   45.914005] x29: ffffffc00bc53d80 x28: ffffff8001bf3700 x27: 0000000000000000
22 [   45.914540] x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
23 [   45.914995] x23: 0000000000000000 x22: ffffffc00bc53dc0 x21: 0000005594b06ba0
24 [   45.915419] x20: ffffff8003e49c00 x19: 000000000000000c x18: 0000000000000000
25 [   45.915844] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
26 [   45.916215] x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
27 [   45.916562] x11: 0000000000000000 x10: 0000000000000000 x9 : ffffffc008269fac
28 [   45.916982] x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
29 [   45.917604] x5 : 0000000000000001 x4 : ffffffc000b75000 x3 : ffffffc00bc53dc0
30 [   45.918237] x2 : 000000000000000c x1 : 0000000000000000 x0 : 0000000000000000
31 [   45.918821] Call trace:
32 [   45.919007]  faulty_write+0x18/0x20 [faulty]
33 [   45.919321]  ksys_write+0x74/0x10c
34 [   45.919520]  __arm64_sys_write+0x24/0x30
35 [   45.919730]  invoke_syscall+0x5c/0x130
36 [   45.919945]  el0_svc_common.constprop.0+0x4c/0x100
37 [   45.920265]  do_el0_svc+0x4c/0xb4
38 [   45.920462]  el0_svc+0x28/0x80
39 [   45.920634]  el0t_64_sync_handler+0xa4/0x130
40 [   45.920860]  el0t_64_sync+0x1a0/0x1a4
41 [   45.921193] Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
42 [   45.922292] ---[ end trace 85782f6c0b5daaeb ]---
Segmentation fault
```
Analysis

Line 1 indiactes what happened to cause the oops. In this case, there was a NULL pointer dereference
Line 32 shows where the error happened -> faulty, 18 bytes into the function faulty_write, which is 20 bytes long
