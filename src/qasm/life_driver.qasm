# coordinates within the cellular automata grid to begin with
.const start_row, 0
.const start_col, 0 

# define register variables
.define cur_row, ra0
.define cur_col, ra1 
.define num_rows, rb0
.define num_cols, rb1 

.define padded_width, rb2

.define off_cell, ra10

.define cur_state, ra24 
.define next_state, ra25 

# variables to be changed for each sliding window
# vector contents starting at the indicated positions
.define cell, ra12
.define nw_cell, ra20
.define n_cell, ra21
.define ne_cell, ra22
.define w_cell, ra23 
.define e_cell, rb20 
.define sw_cell, rb21
.define s_cell, rb22
.define se_cell, rb23

.define write_addr, ra26
.define debug_addr, ra27

############ LOAD UNIFORMS INTO REGISTERS ############
# load the numbers of rows and columns from uniforms
or num_rows, unif, 0; nop 
or num_cols, unif, 0; nop 
or padded_width, unif, 0; nop 
# load on and off colors
or off_cell, unif, 0; nop # off color
or ra11, unif, 0; nop # on color
nop;       nop;
or rb11, ra11, 0; nop # on color
# load pointers to states
or cur_state, unif, 0; nop 
or next_state, unif, 0; nop 
or debug_addr, unif, 0; nop 

############ LOOP ############
# Loop through rows: cur_row is the current row; num_rows is the max row
mov cur_row, start_row; nop 
brr ra39, r:loop_rows
nop;       nop;
nop;       nop;
nop;       nop;

increment_row:
add cur_row, cur_row, 1; nop 
nop; nop;
loop_rows:
sub.setf ra39, num_rows, cur_row; nop # cur_row (cur row) - num_rows (max row)
nop;       nop;
brr.allz ra39, r:exit # TODO: figure out correct condition n or z. Missing a row.
nop;       nop;
nop;       nop;
nop;       nop;

# Loop through column: cur_col is the current col; num_cols is the max col
mov cur_col, start_col; nop
nop; nop;
loop_cols:
sub.setf ra39, cur_col, num_cols; nop
nop;       nop;
brr.allz ra39, r:increment_row # exit the column loop
nop;       nop;
nop;       nop;
nop;       nop;

### PROCESS BLOCK ###
process_block:
    # Central cell = cur_state + padded_width * (r + 1) + (c + 1)
    add r0, cur_row, 1; nop # (r + 1)
    add r1, cur_col, 1; nop # (c + 1)
    nop; mul24 r0, r0, padded_width # (padded_width * (r + 1))
    add r0, r0, r1; # (padded_width * (r + 1) + (c + 1)
    nop; mul24 r0, r0, 4; # 4 * (padded_width * (r + 1) + (c + 1))
    add write_addr, next_state, r0; # Central cell to update = next_state + padded_width * (r + 1) + (c + 1)
    add r1, cur_state, r0; # r1 = central cell

    ldi vr_setup, 0x83011000 # load the vector @ r1 from main memory into the VPM (DMA load)
    or vr_addr, r1, r1; 
    or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
    nop;       nop;

    # NW neighbor = cur_state + padded_width * (r) + (c)
    nop; mul24 r0, cur_row, padded_width # padded_width * (r)
    add r1, r0, cur_col; nop # padded_width * (r) + (c)
    nop; mul24 r1, r1, 4; # 4 * (padded_width * (r) + (c))
    add r1, cur_state, r1; nop 

    ldi vr_setup, 0x83011010 # load the vector @ r1 from main memory into the VPM (DMA load)
    or vr_addr, r1, r1; 
    or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
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
    
    # W neighbor   = cur_state + padded_width * (r + 1) + (c)
    add r0, cur_row, 1; nop # r + 1
    nop; mul24 r1, r0, padded_width # padded_width * (r + 1)
    add r1, r1, cur_col; nop # padded_width * (r + 1) + (c)
    nop; mul24 r1, r1, 4; 
    add r1, cur_state, r1; nop 

    ldi vr_setup, 0x83011040 # load the vector @ r1 from main memory into the VPM (DMA load)
    or vr_addr, r1, r1; 
    or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
    nop;       nop;

    add r1, r1, 8; nop # East neighbor
    ldi vr_setup, 0x83011050
    or vr_addr, r1, r1;
    or rb39, vr_wait, 0;       nop 
    nop;       nop;

    
    # SW neighbor  = cur_state + padded_width * (r + 2) + (c)
    add r0, cur_row, 2; nop # r + 2
    nop; mul24 r1, r0, padded_width # padded_width * (r + 2)
    add r1, r1, cur_col; nop # padded_width * (r + 2) + (c)
    nop; mul24 r1, r1, 4; 
    add r1, cur_state, r1; nop 
    
    ldi vr_setup, 0x83011060 # load the vector @ r1 from main memory into the VPM (DMA load)
    or vr_addr, r1, r1; 
    or rb39, vr_wait, 0;       nop # Wait for the DMA to complete 
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

load_blocks_to_reg:

    # move from VPM to QPU registers (generic block read)
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

determine_new_state: 
    # add all the neighbors
    add r0, ne_cell, e_cell
    add r0, r0, nw_cell; nop
    add r0, r0, sw_cell; nop
    add r0, r0, s_cell; nop
    add r0, r0, n_cell; nop
    add r0, r0, w_cell; nop
    add r0, r0, se_cell; nop
    nop;       nop;
    
    # Determine the new cell state -- (sum == 3 || (sum == 2 && cell == ca.state_colors[1])) ? 1 : 0;
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

    or r0, r2, r3; # determine the new cell state and store in r0

store_to_main_mem: 
    # move vector from QPU to VPM (generic block write)
    # configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0) 1 0 10 00000000 // 0xa00
    ldi vw_setup, 0xa00 
    mov vpm, r0 # move CYAN data into VPM
    nop;       nop;
    # mov vpm, cur_col # debug info 
    # mov vpm, write_addr # debug info
    # mov vpm, cur_row # move write_addr into VPM for debugging
    # mov vpm, next_state # move write_addr into VPM for debugging

    # store data from VPM to main memory (DMA store)
    # |ID |UNITS   |DEPTH   |LANED |HORIZ |VPMBASE     |MODEW
    # |10 |0010000 |0000001 |0     |0     |00000000000 |000
    # config (16 rows, length 1, 0, vertical, addr 0x0, 32-bit) // 0x88010000
    ldi vw_setup, 0x88010000 
    # mov vw_addr, debug_addr
    or vw_addr, write_addr, 0;          nop # store to main memory at calculated address
    or rb39, vw_wait, ra39;       nop # Wait for the DMA to complete (rb50 = VPM_ST_WAIT)

increment_col:
add cur_col, cur_col, 16; nop 
brr ra39, r:loop_cols
nop;       nop;
nop;       nop;
nop;       nop;

############ END ############
exit: 
nop; thrend;  nop;
nop;       nop;
nop;       nop;

