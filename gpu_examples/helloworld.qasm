# Program to add a constant value (specified in the qasm) and a uniform (passed
# to the GPU). It stores the result at the address in main memory (at an address
# specified by another uniform) by writing to the VPM and storing from the VPM
# back to main memory via DMA (direct memory access).

# Uniforms
# (1) number to add
# (2) address to write result in

# Citation: code taken directly from https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/
# and https://github.com/elorimer/rpi-playground/blob/master/QPU/helloworld/helloworld.asm
# Sarah Chen made minor adaptations and added further explanatory comments.

# Load the value we want to add to the input into a register
ldi r1, 0x1019

# Configure the VPM for writing (see Table 32 for generic block write setup format)
# 1 0 10 00000000 -> stride=0, horizontal, 32-bit elements, starting at VPM location (0, 0)
ldi vw_setup, 0xa00

# Add the input value (first uniform) and the hard-coded constant (r1) into the VPM
add vpm, r1, unif;       nop

# move 16 words (1 vector) back to the host memory (DMA)
# Set up. See Table 34: VCD DMA Store (VDW) Basic Setup Format
# |ID |UNITS   |DEPTH   |LANED |HORIZ |VPMBASE     |MODEW
# |10 |0010000 |0000001 |0     |0     |00000000000 |000
# |   |16 rows | 1 col  |-     |vert  |base addr 0 |32-bit
ldi vw_setup, 0x88010000

# Perform the store
# The next uniform is the host address to write to. Write the uniform to rb50 
# (VPM_ST_ADDR) to initiate the DMA.
or rb50, unif, 0;          nop

# Wait for the DMA to complete by reading from rb50 = VPM_ST_WAIT
or rb39, rb50, ra39;       nop

nop; thrend;  nop;
nop;       nop;
nop;       nop;
