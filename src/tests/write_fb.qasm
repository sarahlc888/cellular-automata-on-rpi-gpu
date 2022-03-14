# LOAD VECTORS FROM MEMORY
# load the addresses of vectors into registers
or r1, unif, 0; nop
# set up VCD (Table 36: VCD DMA Load (VDR) Basic Setup Format)

# Adapted from rpi playground
# Modified: MODEW = 0, MPITCH = 3, ROWLEN = 16, NROWS = 1, VPITCH=1, horizontal, ADDRXY = 0
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 00000000000|
# 1 000 0011 0000 0001 0001 0 00000000000 // 0x83011000
ldi vr_setup, 0x83011000
or vr_addr, r1, r1; # load the vectors at r1 from main memory into the VPM (DMA load)
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
nop;       nop;
nop;       nop;
nop;       nop;

# move from VPM to QPU registers (generic block read)
# read 1 vector, unused, stride of 1, horizontal, ignore, 32 bit, starting at 0x0 
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 0001| 00|   000001|     1|     0|   10|  00000000| // 0x101a00
# FLAG change
ldi vr_setup, 0x101a00
mov r2, vpm; nop
nop;       nop;
nop;       nop;
nop;       nop;

# move vector from QPU to VPM (generic block write)
# configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0) 1 0 10 00000000 // 0xa00
# configure VPM to be written in (stride=0, vertical, ignore packed/laned, 32-bit, address 0x0) 0 0 10 00000000 // 0x200
# stride = 1 --> 0x1a00 (but stride shouldn't matter bc there's only 1 vector)
ldi vw_setup, 0xa00 
mov vpm, r2; nop # move data into VPM

# store data from VPM to main memory (DMA store)
# |ID |UNITS   |DEPTH   |LANED |HORIZ |VPMBASE     |MODEW
# |10 |0010000 |0000001 |0     |0     |00000000000 |000
# config (16 rows, length 1, 0, vertical, addr 0x0, 32-bit) // 0x88010000
ldi vw_setup, 0x88010000 
or vw_addr, unif, 0;          nop # store to main memory
or rb39, vw_wait, ra39;       nop # Wait for the DMA to complete (rb50 = VPM_ST_WAIT)

nop; thrend;  nop;
nop;       nop;
nop;       nop;
