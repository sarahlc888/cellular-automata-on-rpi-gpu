# LOAD VECTORS FROM MEMORY
# load the addresses of vectors into registers
or r1, unif, 0; nop
or r2, unif, 0; nop
# set up VCD (Table 36: VCD DMA Load (VDR) Basic Setup Format)
# note: pitch = 8*2^MPITCH bytes. TODO: understand MPITCH? VPITCH?
# From rpi playground: MODEW = 0, MPITCH = 3, ROWLEN = 16, NROWS = 16, VPITCH=1, horizontal, ADDRXY = 0
# 1 000 0011 0000 0000 0001 0 00000000000
# Modified: MODEW = 0, MPITCH = 3, ROWLEN = 16, NROWS = 1, VPITCH=1, horizontal, ADDRXY = 0
# 1 000 0011 0000 0001 0001 0 00000000000 // 0x83011000
ldi vr_setup, 0x83011000

# load the vectors at r1 from main memory into the VPM (DMA load)
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
# TODO: does write address automatically increment? or have to re-set-up?
or vr_addr, r2, r2;
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 

# move from VPM to QPU registers
# read 16 vectors, unused, stride of 1, vertical, ignore, 32 bit, starting at 0x0 // 0x1200
# read 2 vectors, unused, stride of 1, vertical, ignore, 32 bit, starting at 0x0 // 0010 00 000001 0 0 10 00000000 // 0x201200
ldi vr_setup, 0x201200
mov ra4, vpm
mov rb5, vpm
nop;       nop;
nop;       nop;
nop;       nop;

# do addition
# add ra3, ra4, rb5;
mov ra3, rb5;
nop;       nop;
nop;       nop;
nop;       nop;

# move vector from QPU to VPM
ldi vw_setup, 0xa00 # configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0)
mov vpm, ra3; nop # move data into VPM

# store data from VPM to main memory (DMA store)
ldi vw_setup, 0x88010000 # config (16 rows, length 1, 0, vertical, addr 0x0, 32-bit)
or vw_addr, unif, 0;          nop # store to main memory
or rb39, vw_wait, ra39;       nop # Wait for the DMA to complete (rb50 = VPM_ST_WAIT)

nop; thrend;  nop;
nop;       nop;
nop;       nop;
