## Notes on the Raspberry Pi's GPU

### Overview
GPU vs QPU vs VPU

TODO: clarify why memory has to be locked and stuff and allocated

The general workflow is as follows

- Write assembly file
- Run assembler using `~/Downloads/tools/vc4asm/build-Darwin-arm64-Release/vc4asm -i vc4.qinc -C FILE.c FILE.qasm`
- Include `FILE.c` within the `.c` driver program
- Note: must make clean every time you modify the assembly
- If run into issues, a reset should work. If gpu programs consistently require reboots beforehand, this signals that something is wrong with your code.

The modules build from together as follows
- `mailbox` - communicate with the GPU via the mailbox (from $CS107E/src)
- `mailbox_functions` - use the `mailbox` module to support enabling the QPU, allocating/locking/freeing memory, sending code to the VPU/QPUs (from previous project)
- `qpu` - implements functions to initialize the QPU, allocate/free memory for the QPU, run a program on the QPU, and modify/read QPU status (from previous project)

Our modules built on top of these to TODO


### QPU assembly
Src/qasm/examples contains example files

### Submitting a program to the QPU
How to write to registers to submit programs
How uniforms come into the equation

### Memory
Relationship between main memory, VPM, and QPU registers


The VPM and VCD are set up by writing 32-bits of data to the setup register. The type fo setup data is identified by the MSBs of the value.

#### Transferring data from main memory to VPM

For general-purpose computing, the Vertex Cache DMA (VCD) executes reads from the main memory to the VPM. It places the data read from main memory into the VPM's QPU read FIFO. It can read sequences of 16 32-bit words from main memory by loading blocks of scattered memory, 2D byte arrays, etc. It can place them horizontally or vertically in the VPM memory.

#### Transferring data from VPM to main memory

The VDW stores vertical or horizontal VPM data into 2D arrays of data in memory


### Tips 
Engler’s theorem of epsilon-steps
Peeling back the layers of abstraction 


# Note: do not use r4 (read only)
# Note: Ra39 is no op, which explains why to use brr ra39, label 
- [ ] Note that the mailbox will not actually return 0 or whatever it claims 


- Figure out why there's junk between labels, requiring the 4*j code when accessing results copied from VPM to main memory
