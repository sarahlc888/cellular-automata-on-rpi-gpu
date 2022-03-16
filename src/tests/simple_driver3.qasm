.const start_row, 0
.const start_col, 0 

# constants
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
.define cell, ra0
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

    # move from VPM to QPU registers (generic block read)
    # ldi vr_setup, 0x101a00
    # mov cell, vpm; nop

    # TODO: convert to 1s and 0s, determine new cell state
store_to_main_mem: 
    # move vector from QPU to VPM (generic block write)
    # configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0) 1 0 10 00000000 // 0xa00
    # ldi vw_setup, 0xa00 
    # mov vpm, cell # move CYAN data into VPM
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

