# Citation: taken directly from https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/

# Load the value we want to add to the input into a register (for general use)
ldi r1, 0x1234

# Configure the VPM for writing. "QPU register address map on pages 37-38 where it shows writing to vw_setup is “VPMVCD_WR_SETUP”.  The VPM interface is documented in section 7 and table 32 describes the format for this register.  0xa00 means to write with no stride, starting at VPM location (0, 0) with horizontal format and 32-bit elements."
# 1 0 1 00 0000000 # stride = 1, vertical, laned, 8-bit // 0xa00
# 1 0 1 10 0000000 # stride = 1, vertical, laned, 32-bit // 0xb00
# 1 1 1 10 0000000 # stride = 1, horizontal, laned, 32-bit // 0xf00
ldi vw_setup, 0xa00

# Add the input value (first unif) and the register with the hard-coded constant (r1) into the VPM (rb48).
add vpm, r1, unif;       nop

## move 16 words (1 vector) back to the host (DMA)
# Table 34: VCD DMA Store (VDW) Basic Setup Format
# 10 0010000 0000001 0 0 00000000000 000 # 16 rows of length 1, 32 bit, vertical, base address 0 // 0x88010000
# 10 0010000 0000010 0 0 00000000000 000 # 16 rows of length 2, 32 bit, vertical, base address 0 // 0x88020000
# 10 0010000 0000000 0 0 00000000000 000 # 16 rows of length 0, 32 bit, vertical, base address 0 // 0x88000000
# 10 0001111 0000000 0 0 00000000000 000 # 0-indexed 16 rows of length 1, 32 bit, vertical, base address 0 // 0x87800000

# 10 0010000 0000001 0 1 00000000000 000 # 16 rows of length 1, 32 bit, horizontal, base address 0 // 0x88014000
ldi vw_setup, 0x88010000

# stride set up (no stride) // led to all 0s
# 11 000000.... // 0xc0000000
# blockmode 1 // 0xc0008000
# ldi vw_setup, 0xc0008000

## initiate the DMA (the next uniform - unif (UNIFORM_READ) - is the host address to write to))
# writing to rb50 = VPM_ST_ADDR
or rb50, unif, 0;          nop

# Wait for the DMA to complete (reading from rb50 = VPM_ST_WAIT)
or rb39, rb50, ra39;       nop

nop; thrend;  nop;
nop;       nop;
nop;       nop;
