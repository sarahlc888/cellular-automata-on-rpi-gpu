# Load the value we want to add to the input into a register
ldi ra1, 0x1236

# Configure the VPM for writing. "You can find the QPU register address map on pages 37-38 where it shows writing to rb49 is “VPMVCD_WR_SETUP”.  The VPM interface is documented in section 7 and table 32 describes the format for this register.  0xa00 means to write with no stride, starting at VPM location (0, 0) with horizontal format and 32-bit elements."
# 1 0 1 00 0000000 # stride = 1, vertical, laned, 8-bit
ldi rb49, 0xa00

# Add the input value (first uniform - rb32) and the register with the hard-coded
# constant (ra1) into the VPM (rb48).
add rb48, ra1, rb32;       nop

## move 16 words (1 vector) back to the host (DMA)
# Table 34: VCD DMA Store (VDW) Basic Setup Format
# 10 0010000 0000001 0 0 00000000000 000 # 16 rows of length 1, 32 bit, horizontal, base address 0
ldi rb49, 0x88010000

## initiate the DMA (the next uniform - ra32 (UNIFORM_READ) - is the host address to write to))
# rb50 is VPM_ST_ADDR
or rb50, ra32, 0;          nop

# Wait for the DMA to complete
or rb39, rb50, ra39;       nop

# trigger a host interrupt (writing rb38) to stop the program
or rb38, ra39, ra39;       nop

nop; thrend;  nop;
nop;       nop;
nop;       nop;
