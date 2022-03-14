# sarahlc888-aviously25-project
## Project title

## Team members

To Do - Avi
- Physical interface (make a board)
- Better UI to select CA type then preset option
- Start/stop simulation w button 
- Could allow saving custom start state presets
- Rig up new potentiometers when they come in, smoothing if time allows (low priority) 

To Do - Sarah
- Get basic GPU working 
- Decide the resolution (aiming 1024)
- Then file compression (needed for big start states) 
- Make 3 presets for each CA (4th is custom)
- Speed up GPU / improve approach in general
- Add more CA
- Could vectorize wireworld 
## Project description
### Components
- CA representation library
- FAT FS support for reading/writing states
- Hardware
- SPI
- Performance optimization
    - CPU speed ups - enabling cache
    - Fine tuning algorithm using profiler (e.g. test performance using modulo for wrapping vs. ternary conditions)
    - GPU
        - Understanding relationship between main memory, VPM, and QPU registers; how uniforms come into the equation; how to write to registers to submit programs
## Member contribution
A short description of the work performed by each member of the team.

## References
Cite any references/resources that inspired/influenced your project. 
If your submission incorporates external contributions such as adopting 
someone else's code or circuit into your project, be sure to clearly 
delineate what portion of the work was derivative and what portion is 
your original work.

Cellular automata library
- Our WireWorld implementation referenced [this](https://mathworld.wolfram.com/WireWorld.html) description of the cellular automaton (including the embedded GIF of OR, XOR, and AND gates).
- We used the [FAT Filesystem module](http://elm-chan.org/fsw/ff/00index_e.html), CS107E [guide](http://cs107e.github.io/guides/extras/sd_library/) to FatFS, and the example project at `$CS107E/examples/sd_fatfs` to write code to read and write presets. 
- To enable the cache, we used `system.c` and `system.h` from CS107E code
- We used code from the profiler extension of assignment 7 (Sarah's submission)

GPU research
- Broadcom manual
    - QPU Register Address Map on p. 37-38
    - Section 7: VPM and VCD (particularly the tables for QPU Registers for VPM and VCD Functions)
- A past CS 107E GPU project, "Bare Metal C QPU Library for the Raspberry Pi" by ahconkey and JoshFrancisCodes [(GitHub)](https://github.com/cs107e/ahconkey-JoshFrancisCodes-project)
    - Before being given access to this code, we did thorough independent research that covered many of their project's references (especially the sources that they heavily based their code on). However, their code was an invaluable starting point because unlike those references, it was baremetal and used the CS 107E mailbox code.
    - This code was particularly helpful in demonstrating: how to submit a program to the GPU by directly writing to GPU registers, how to allocate/free/lock memory for the GPU, how to use `#include` to load assembled QPU programs (and in guiding us to use the [vc4asm](http://maazl.de/project/vc4asm/doc/) assembler).
    - We use their `mailbox_functions` and `qpu` modules in their entirety, with only minor changes.
    - Note: the code did not compile in its given form, so many adjustments were made. Almost all work hours involving the GPU centered around getting a simple poke program to consistently work by consulting this code as well as well as the below references. (The next steps were working with loading/storing more complicated vectors.)
- [Hacking The GPU For Fun And Profit](https://rpiplayground.wordpress.com/category/gpu/)
    - Our QPU assembly code was heavily based off of the accompanying [GitHub repository](https://github.com/elorimer/rpi-playground), especially the `helloworld` and `SHA-256` examples.
- [SIMD processing of AES on the Raspberry Pi’s GPU](https://www.mnm-team.org/pub/Fopras/rixe19/PDF-Version/rixe19.pdf)
    - P. 13-17 provide a valuable overview of the GPU, which makes reading the Broadcom Manual much more accessible
- [gpu-deadbeef](https://github.com/0xfaded/gpu-deadbeef) demonstrates how to write to GPU registers to write from QPU registers into the VPM and then from the VPM into main memory.
- [QPU Demo: DMA Transfers](https://asurati.github.io/wip/post/2021/09/28/qpu-demo-dma-transfers/) breaks down DMA transfers with useful examples. This was helpful in determining how to load/store vector data from main memory to the GPU VPM.
- We consulted code for the existing QPU libraries [GPU_FFT](http://www.aholme.co.uk/GPU_FFT/Main.htm), [QPULib](https://github.com/mn416/QPULib), and [pi-gemm](https://github.com/jetpacapp/pi-gemm/blob/master/helpers.asm).
- General background
    - https://github.com/ali1234/vcpoke
    - https://www.linuxtut.com/en/2e85318989170f967e4b/
    - https://www.elesoftrom.com.pl/blog/en/vc4-3d-programming.php#_vpm

Vectorized game of life
- Approach inspired by https://www.r-bloggers.com/2018/10/conways-game-of-life-in-r-or-on-the-importance-of-vectorizing-your-r-code/
    - "The way this code works is: it builds 8 copies of the life-table, one shifting each neighboring cell into the current cell-position. With these 8 new matrices the entire life forward evolution rule is computed vectorized over all cells using the expression “(pop==3) | (d & (pop>=2) & (pop<=3))“. Notice the vectorized code is actually shorter: we handle the edge-cases by zero-padding."

## Self-evaluation
How well was your team able to execute on the plan in your proposal?  
Any trying or heroic moments you would like to share? Of what are you particularly proud: the effort you put into it? the end product? 
the process you followed? what you learned along the way? Tell us about it!

## Photos
You are encouraged to submit photos/videos of your project in action. 
Add the files and commit to your project repository to include along with your submission.

## Notes on the GPU
The VPM and VCD are set up by writing 32-bits of data to the setup register. The type fo setup data is identified by the MSBs of the value.
### Transferring data from main memory to VPM

For general-purpose computing, the Vertex Cache DMA (VCD) executes reads from the main memory to the VPM. It places the data read from main memory into the VPM's QPU read FIFO. It can read sequences of 16 32-bit words from main memory by loading blocks of scattered memory, 2D byte arrays, etc. It can place them horizontally or vertically in the VPM memory.

### Transferring data from VPM to main memory 
The VDW stores vertical or horizontal VPM data into 2D arrays of data in memory