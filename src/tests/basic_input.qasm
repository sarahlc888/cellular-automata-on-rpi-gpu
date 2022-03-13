# LOAD VECTORS FROM MEMORY
# load the addresses of vectors into registers
or r1, unif, 0; nop
or r2, unif, 0; nop
# set up VCD (Table 36: VCD DMA Load (VDR) Basic Setup Format)
# note: pitch = 8*2^MPITCH bytes. TODO: understand MPITCH? VPITCH?

# Adapted from rpi playground
# Modified: MODEW = 0, MPITCH = 3, ROWLEN = 16, NROWS = 1, VPITCH=1, horizontal, ADDRXY = 0
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 00000000000|
# 1 000 0011 0000 0001 0001 0 00000000000 // 0x83011000
ldi vr_setup, 0x83011000
# load the vectors at r1 from main memory into the VPM (DMA load)
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
# Set up with appropriate next write address
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 00000000001|
# 1 000 0011 0000 0001 0001 0 00000000000 // 0x83011001
ldi vr_setup, 0x83011001
or vr_addr, r2, r2;
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 

# move from VPM to QPU registers
# read 16 vectors, unused, stride of 1, vertical, ignore, 32 bit, starting at 0x0 // 0x1200
# read 2 vectors, unused, stride of 1, vertical, ignore, 32 bit, starting at 0x0 // 0010 00 000001 0 0 10 00000000 // 0x201200
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 0010| 00|   000001|     0|     0|   10|  00000000|
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 0010| 00|   000001|     1|     0|   10|  00000000| // 0x201a00
ldi vr_setup, 0x201a00
mov ra4, vpm
mov rb5, vpm
nop;       nop;
nop;       nop;
nop;       nop;

# do addition
# add ra3, ra4, rb5;
# mov ra3, rb5;
add ra3, ra4, 2;
nop;       nop;
nop;       nop;
nop;       nop;

# move vector from QPU to VPM (generic block write)
# configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0) 1 0 10 00000000 // 0xa00
# configure VPM to be written in (stride=0, vertical, ignore packed/laned, 32-bit, address 0x0) 0 0 10 00000000 // 0x200
# stride = 1 --> 0x1a00 (but stride shouldn't matter bc there's only 1 vector)
ldi vw_setup, 0xa00 
# mov vpm, 12; nop # move data into VPM to sanity check
mov vpm, ra3; nop # move data into VPM

# store data from VPM to main memory (DMA store)
# |ID |UNITS   |DEPTH   |LANED |HORIZ |VPMBASE     |MODEW
# |10 |0010000 |0000001 |0     |0     |00000000000 |000
# config (16 rows, length 1, 0, vertical, addr 0x0, 32-bit) // 0x88010000
# |ID |UNITS   |DEPTH   |LANED |HORIZ |VPMBASE     |MODEW
# |10 |0000001 |0010000 |0     |1     |00000000000 |000
# config (1 row, length 16, 0, horizontal, addr 0x0, 32-bit) // 0x80904000
ldi vw_setup, 0x88010000 
or vw_addr, unif, 0;          nop # store to main memory
or rb39, vw_wait, ra39;       nop # Wait for the DMA to complete (rb50 = VPM_ST_WAIT)

nop; thrend;  nop;
nop;       nop;
nop;       nop;
