# variable names
.define cell, ra0
.define nw_cell, ra20
.define n_cell, ra21
.define ne_cell, ra22
.define w_cell, ra23 
.define e_cell, rb20 
.define sw_cell, rb21
.define s_cell, rb22
.define se_cell, rb23

.define off_cell, ra10

# load the addresses of vectors (within main memory) into registers
or off_cell, unif, 0; nop # off color
or ra11, unif, 0; nop # on color
nop;       nop;
or rb11, ra11, 0; nop # on color

# set up VCD DMA Load (main memory -> VCM) (Table 36)
# ID| MODEW| MPITCH| ROWLEN| NROWS| VPITCH| VERT|      ADDRXY|
#  1|   000|   0011|   0000|  0001|   0001|    0| 00000000000|
or r1, unif, 0; nop # the cell state
ldi vr_setup, 0x83011000
or vr_addr, r1, r1; # load the vectors at r1 from main memory into the VPM (DMA load)
or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
nop;       nop;

or r1, unif, 0; nop # nw neighbor
ldi vr_setup, 0x83011010
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;

add r1, r1, 4; # due north neighbor
ldi vr_setup, 0x83011020 
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;

add r1, r1, 4; # NE neighbor
ldi vr_setup, 0x83011030
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;

or r1, unif, 0; nop # w neigh
ldi vr_setup, 0x83011040
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;

add r1, r1, 8; nop # East neighbor
ldi vr_setup, 0x83011050
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;

or r1, unif, 0; nop # sw neigh
ldi vr_setup, 0x83011060
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;

add r1, r1, 4; # South neighbor
ldi vr_setup, 0x83011070
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;

add r1, r1, 4; # SE neighbor
ldi vr_setup, 0x83011080
or vr_addr, r1, r1;
or rb39, vr_wait, 0;       nop 
nop;       nop;


# move from VPM to QPU registers (generic block read)
# read 1 vector, unused, stride of 1, horizontal, ignore, 32 bit, starting at 0x0 
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 1000| 00|   000001|     1|     0|   10|  00000000| // 0x801a00
# ID|        -|  NUM|  -|   STRIDE| HORIZ| LANED| SIZE|      ADDR|
#  0|   000000| 0001| 00|   000001|     1|     0|   10|  00000000| // 0x101a00



ldi vr_setup, 0x901a00
mov cell, vpm; nop
mov nw_cell, vpm; nop
mov n_cell, vpm; nop
nop;       nop;
mov ne_cell, vpm; nop
mov w_cell, vpm; nop
nop;       nop;
mov e_cell, vpm; nop
mov sw_cell, vpm; nop
nop;       nop;
mov s_cell, vpm; nop
mov se_cell, vpm; nop
nop;       nop;

# convert states to 1s and 0s

sub.setf nw_cell, nw_cell, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz nw_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz nw_cell, 0; nop 

sub.setf n_cell, n_cell, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz n_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz n_cell, 0; nop 

sub.setf ne_cell, ne_cell, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz ne_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz ne_cell, 0; nop 

sub.setf w_cell, w_cell, rb11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz w_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz w_cell, 0; nop 

sub.setf e_cell, e_cell, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz e_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz e_cell, 0; nop 

sub.setf sw_cell, sw_cell, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz sw_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz sw_cell, 0; nop 

sub.setf s_cell, s_cell, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz s_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz s_cell, 0; nop 

sub.setf se_cell, se_cell, ra11; nop # 0 if neighbor sum == on 
nop;       nop;
mov.ifz se_cell, 1; nop # set 1 if neigh sum == on, 0 otherwise
mov.ifnz se_cell, 0; nop 


# do addition
# cell is the cell state, do not add it
add r0, ne_cell, e_cell
add r0, r0, nw_cell; nop
add r0, r0, sw_cell; nop
add r0, r0, s_cell; nop
add r0, r0, n_cell; nop
add r0, r0, w_cell; nop
add r0, r0, se_cell; nop
nop;       nop;

# Determine the new cell state
# cell = old cell state (0 or 1); r0 = neighbor sum
# r2, r3 are free now

sub.setf r3, r0, 3; nop # 0 if neighbor sum == 3, so Z should be set 
mov.ifz r3, ra11; nop # set 1 if neigh sum == 3, 0 otherwise
mov.ifnz r3, off_cell; nop 

nop;       nop;

sub.setf r2, r0, 2; nop # 0 if neighbor sum == 2
mov.ifz r2, ra11; nop # set 1 if neigh sum == 2, 0 otherwise
mov.ifnz r2, off_cell; nop 

nop;       nop;

and r2, r2, cell; nop # check if neighbor sum == 2 AND cell was previously on

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
