# load a vector
# TODO: move a vector from main memory into the VPM (DMA load) // use rb50, unif, 0;  
# TODO: and then move from VPM to QPU registers
ldi r1, 0x3
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
