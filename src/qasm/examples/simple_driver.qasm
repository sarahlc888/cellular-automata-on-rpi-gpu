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

### PROCESS BLOCK ###
# Get pointers to central cell and neighbors in cur_state and the central cell in next_state
process_block:
    mov cur_row, 0; nop
    mov cur_col, 0; nop
    # Central cell = cur_state + padded_width * (r + 1) + (c + 1)
    add r0, cur_row, 1; nop # (r + 1)
    add r1, cur_col, 1; nop # (c + 1)
    nop; mul24 r0, r0, padded_width # (padded_width * (r + 1))
    add r0, r0, r1; # (padded_width * (r + 1) + (c + 1)
    nop; mul24 r0, r0, 4; # 4 * (padded_width * (r + 1) + (c + 1))
    add write_addr, next_state, r0; # Central cell to update = next_state + padded_width * (r + 1) + (c + 1)

store_to_main_mem: 
    # move vector from QPU to VPM (generic block write)
    # configure VPM to be written in (stride=0, horizontal, ignore packed/laned, 32-bit, address 0x0) 1 0 10 00000000 // 0xa00
    ldi vw_setup, 0xa00 
    # ldi vpm, 0xFFFF00FF # move magenta data into VPM
    ldi vpm, 0xdeadbeef # move magenta data into VPM
    # mov vpm, write_addr # move write_addr into VPM for debugging
    # mov vpm, next_state # move write_addr into VPM for debugging

    # store data from VPM to main memory (DMA store)
    # |ID |UNITS   |DEPTH   |LANED |HORIZ |VPMBASE     |MODEW
    # |10 |0010000 |0000001 |0     |0     |00000000000 |000
    # config (16 rows, length 1, 0, vertical, addr 0x0, 32-bit) // 0x88010000
    ldi vw_setup, 0x88010000 
    or vw_addr, debug_addr, 0;          nop # store to main memory at calculated address
    # or vw_addr, write_addr, 0;          nop # store to main memory at calculated address
    or rb39, vw_wait, ra39;       nop # Wait for the DMA to complete (rb50 = VPM_ST_WAIT)

############ END ############
exit: 
nop; thrend;  nop;
nop;       nop;
nop;       nop;

