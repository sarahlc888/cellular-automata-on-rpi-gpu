# LOAD VECTORS FROM MEMORY
# load the addresses of vectors into registers
or r1, unif, 0; nop
or r2, unif, 0; nop

# load a vector
# move a vector from main memory into the VPM (DMA load)
# 1 000 0011 0000 0001 0001 1 00000000000 // 0x83011800
# horiz 0x83011000
ldi ra49, 0x83011000
or vr_addr, r2, r2; nop
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
nop;       nop;
nop;       nop;
nop;       nop;

# TODO: and then move from VPM to QPU registers
ldi vr_setup, 0x101a00
mov r1, vpm; nop 
nop;       nop;
nop;       nop;
nop;       nop;

# ldi r1, 0x4
ldi r2, 0x2

# do addition
add r3, r1, r2;

# move vector from QPU to VPM
ldi vw_setup, 0xa00 # configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0)
mov vpm, r3; nop # move data into VPM

# store data from VPM to main memory (DMA store)
ldi vw_setup, 0x88010000 # config (16 rows, length 1, 0, vertical, addr 0x0, 32-bit)
or vw_addr, unif, 0;          nop # store to main memory
or rb39, vw_wait, ra39;       nop # Wait for the DMA to complete (rb50 = VPM_ST_WAIT)

nop; thrend;  nop;
nop;       nop;
nop;       nop;
