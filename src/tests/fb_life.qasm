# load the addresses of vectors (within main memory) into registers
or ra10, unif, 0; nop # off color
or ra11, unif, 0; nop # on color
nop;       nop;
nop;       nop;
nop;       nop;

or rb11, ra11, 0; nop # on color

or r1, unif, 0; nop # the cell state
or r2, unif, 0; nop # nw neigh
or r3, unif, 0; nop # w neigh
or r0, unif, 0; nop # sw neigh

# set up VCD DMA Load (main memory -> VCM) (Table 36)
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 00000000000|
ldi vr_setup, 0x83011000
or vr_addr, r1, r1; # load the vectors at r1 from main memory into the VPM (DMA load)
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
nop;       nop;

# Set up with appropriate next write address (x = 0, y = 1)
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000001 0000| // 0x83011010
ldi vr_setup, 0x83011010
or vr_addr, r3, r3;
or rb39, vr_wait, 0;       nop 
nop;       nop;

# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000010 0000| // 0x83011020
ldi vr_setup, 0x83011020
or vr_addr, r0, r0;
or rb39, vr_wait, 0;       nop 
nop;       nop;

# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000011 0000| // 0x83011030
ldi vr_setup, 0x83011030
or vr_addr, r2, r2;
or rb39, vr_wait, 0;       nop 
nop;       nop;

# due north neighbor
add r2, r2, 4;
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000100 0000| // 0x83011040
ldi vr_setup, 0x83011040
or vr_addr, r2, r2;
or rb39, vr_wait, 0;       nop 
nop;       nop;

# NE neighbor
add r2, r2, 4;
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000101 0000| // 0x83011050
ldi vr_setup, 0x83011050
or vr_addr, r2, r2;
or rb39, vr_wait, 0;       nop 
nop;       nop;

# East neighbor
add r3, r3, 8; nop
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000001 0000| // 0x83011010
ldi vr_setup, 0x83011060
or vr_addr, r3, r3;
or rb39, vr_wait, 0;       nop 
nop;       nop;

# South neighbor
add r0, r0, 4;
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000010 0000| // 0x83011020
ldi vr_setup, 0x83011070
or vr_addr, r0, r0;
or rb39, vr_wait, 0;       nop 
nop;       nop;

# SE neighbor
add r0, r0, 4;
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 0000010 0000| // 0x83011020
ldi vr_setup, 0x83011080
or vr_addr, r0, r0;
or rb39, vr_wait, 0;       nop 
nop;       nop;


# move from VPM to QPU registers (generic block read)
# read 1 vector, unused, stride of 1, horizontal, ignore, 32 bit, starting at 0x0 
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 1000| 00|   000001|     1|     0|   10|  00000000| // 0x801a00
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 0001| 00|   000001|     1|     0|   10|  00000000| // 0x101a00
# FLAG change
ldi vr_setup, 0x901a00
mov ra4, vpm; nop
mov rb5, vpm; nop
nop;       nop;
mov ra1, vpm; nop
nop;       nop;
mov rb6, vpm; nop
mov ra6, vpm; nop
nop;       nop;
mov rb7, vpm; nop
mov ra7, vpm; nop
nop;       nop;
mov rb8, vpm; nop
mov ra8, vpm; nop
nop;       nop;

# convert states to 1s and 0s
sub.setf ra1, ra1, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz ra1, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz ra1, 0; nop 
nop;       nop;

sub.setf rb5, rb5, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz rb5, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz rb5, 0; nop 
nop;       nop;

sub.setf rb6, rb6, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz rb6, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz rb6, 0; nop 
nop;       nop;

sub.setf rb7, rb7, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz rb7, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz rb7, 0; nop 
nop;       nop;

sub.setf rb8, rb8, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz rb8, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz rb8, 0; nop 
nop;       nop;

sub.setf ra6, ra6, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz ra6, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz ra6, 0; nop 
nop;       nop;

sub.setf ra7, ra7, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz ra7, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz ra7, 0; nop 
nop;       nop;

sub.setf ra8, ra8, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz ra8, 1; nop # set 1 if neigh sum == on, 0 otherwise
nop;       nop;
mov.ifnz ra8, 0; nop 
nop;       nop;


# do addition
# ra4 is the cell state, do not add it
add r0, ra1, rb5
add r0, r0, rb6; nop
add r0, r0, rb7; nop
add r0, r0, rb8; nop
add r0, r0, ra6; nop
add r0, r0, ra7; nop
add r0, r0, ra8; nop
nop;       nop;

# Determine the new cell state
# ra4 = old cell state (0 or 1); r0 = neighbor sum
# r2, r3 are free now

sub.setf r3, r0, 3; nop # 0 if neighbor sum == 3, so Z should be set 
mov.ifz r3, ra11; nop # set 1 if neigh sum == 3, 0 otherwise
mov.ifnz r3, ra10; nop 

nop;       nop;

sub.setf r2, r0, 2; nop # 0 if neighbor sum == 2
mov.ifz r2, ra11; nop # set 1 if neigh sum == 2, 0 otherwise
mov.ifnz r2, ra10; nop 

nop;       nop;

and r2, r2, ra4; nop # check if neighbor sum == 2 AND cell was previously on

or r0, r2, r3; # determine the new cell state
# (sum == 3 || (sum == 2 && cell == ca.state_colors[1])) ? 1 : 0;

# move vector from QPU to VPM (generic block write)
# configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0) 1 0 10 00000000 // 0xa00
# configure VPM to be written in (stride=0, vertical, ignore packed/laned, 32-bit, address 0x0) 0 0 10 00000000 // 0x200
# stride = 1 --> 0x1a00 (but stride shouldn't matter bc there's only 1 vector)
ldi vw_setup, 0xa00 
# mov vpm, 12; nop # move data into VPM to sanity check
mov vpm, r0; nop # move data into VPM

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
