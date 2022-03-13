# sarahlc888-aviously25-project
## Project title

## Team members

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
- `system.c` and `system.h` from CS107E code

GPU research
- Consulted past GPU project code from [ahconkey-JoshFrancisCodes-project](https://github.com/cs107e/ahconkey-JoshFrancisCodes-project). Before being given access to their code, we did independent research that covered most of their references as well. Their code was particularly helpful in elucidating how to directly write to GPU registers and circumvent the mailbox.
- https://www.mnm-team.org/pub/Fopras/rixe19/PDF-Version/rixe19.pdf
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