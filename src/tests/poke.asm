# Citation: taken directly from https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/

# Load the value we want to add to the input into a register (for general use)
ldi r1, 0x10;

# Configure the VPM for writing. "QPU register address map on pages 37-38 where it shows writing to vw_setup is “VPMVCD_WR_SETUP”.  The VPM interface is documented in section 7 and table 32 describes the format for this register.  0xa00 means to write with no stride, starting at VPM location (0, 0) with horizontal format and 32-bit elements."
# 1 0 1 00 0000000 # stride = 1, vertical, laned, 8-bit
ldi vw_setup, 0xa00

# Add the input value (first unif) and the register with the hard-coded constant (r1) into the VPM (rb48).
add vpm, unif, r1;       nop

## move 16 words (1 vector) back to the host (DMA)
# Table 34: VCD DMA Store (VDW) Basic Setup Format
# 10 0010000 0000001 0 0 00000000000 000 # 16 rows of length 1, 32 bit, horizontal, base address 0
ldi vw_setup, 0x88010000

## initiate the DMA (the next uniform - unif (UNIFORM_READ) - is the host address to write to))
# writing to rb50 = VPM_ST_ADDR
or rb50, unif, 0;          nop

# Wait for the DMA to complete (reading from rb50 = VPM_ST_WAIT)
or rb39, rb50, ra39;       nop

nop; thrend;  nop;
nop;       nop;
nop;       nop;
