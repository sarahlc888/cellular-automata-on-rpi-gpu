# Program to add four vectors together. Steps:
# - reads the addresses of the vectors from uniforms
# - loads vectors from main memory into the VPM
# - read vectors from the VPM into QPU registers
# - computes sum
# - writes vectors from the QPU registers into the VPM
# - stores the vectors from the VPM into main memory

# Uniforms
# (1-4) addresses of vectors to add together
# (2) address to write result in

##### LOAD VECTORS FROM MAIN MEMORY INTO VPM (using DMA LOAD) #####
# load the addresses of vectors into registers
or r1, unif, 0; nop
or r2, unif, 0; nop
or r3, unif, 0; nop
or r0, unif, 0; nop

# Set up VCD (Table 36: VCD DMA Load (VDR) Basic Setup Format)
# MODEW = 0, MPITCH = 3, ROWLEN = 16, NROWS = 1, VPITCH=1, horizontal, ADDRXY = 0
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|       ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000000 0000|
ldi vr_setup, 0x83011000
or vr_addr, r1, r1; # load the vector at address r1
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 

# Set up with modified write address (x = 0, y = 1) to fill the VPM's second row
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|       ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000001 0000|
ldi vr_setup, 0x83011010
or vr_addr, r3, r3;
or rb39, vr_wait, 0;       nop 

# Write to VPM's third row
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000010 0000| 
ldi vr_setup, 0x83011020
or vr_addr, r0, r0;
or rb39, vr_wait, 0;       nop 

# Write to VPM's fourth row
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|       ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000011 0000|
ldi vr_setup, 0x83011030
or vr_addr, r2, r2;
or rb39, vr_wait, 0;       nop 

##### MOVE VECTORS FROM VPM TO QPU REGISTERS (using generic block read) #####
# read 4 vectors, stride of 1, horizontal, 32 bit, starting at 0x0 
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 0100| 00|   000001|     1|     0|   10|  00000000| 
ldi vr_setup, 0x401a00
mov ra4, vpm; nop
mov rb5, vpm; nop
mov ra1, vpm; nop
mov rb6, vpm; nop

##### COMPUTE VECTOR SUM #####
add r0, ra4, rb5
add r0, r0, ra1; nop
add r0, r0, rb6; nop

##### MOVE RESULT VECTOR FROM QPU REGISTERS TO VPM (using generic block write) #####
# configure VPM to be written in (stride=0, horizontal, 32-bit, address 0x0) 1 0 10 00000000 // 0xa00
# (but stride shouldn't matter bc there's only 1 vector)
ldi vw_setup, 0xa00 
mov vpm, r0; nop # move data into VPM

##### MOVE RESULT VECTOR FROM VPM INTO MAIN MEMORY (using DMA STORE) #####
# Set up store of 16 rows, 1 column, vertical, addr 0x0, 32-bit elements
# |ID |UNITS   |DEPTH   |LANED |HORIZ |VPMBASE     |MODEW
# |10 |0010000 |0000001 |0     |0     |00000000000 |000
ldi vw_setup, 0x88010000 
or vw_addr, unif, 0;          nop # store to main memory
or rb39, vw_wait, ra39;       nop # Wait for the DMA to complete (rb50 = VPM_ST_WAIT)

nop; thrend;  nop;
nop;       nop;
nop;       nop;
