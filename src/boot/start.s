// Identify this section as the one to go first in binary image
.section ".text.start"

.globl _start
_start:
    mov sp, #0x8000000
    mov fp, #0
    bl _cstart
set_delay:
    mov r0, #0xff000
delay:
    subs r0, r0, #1
    bpl delay
bootloader: b 0x200000
