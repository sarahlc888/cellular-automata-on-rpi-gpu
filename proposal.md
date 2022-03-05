## Cellular Automata Simulator

## Team members
- Avi Udash
- Sarah Chen

## Project description
We seek to create a program that simulates cellular automata on an HDMI display. Building upon the graphics library, we will support common automata such as the Game of Life, beginning at various pre-set start states that users can select from using a hardware control board.

The board (which will include two knobs and a button) will enable users to:
- Select which automata to display
- Select which start state to use
- Customize individual pixels within the grid
- Start/stop the simulation

We hope to support relatively large-scale, fast-paced simulations and will do so by leveraging the GPU. This is a stretch goal because writing code to download code to the GPU and call that code appears to complex. Fallback positions would include:
- Building off of existing GPU libraries (possibly from the past CS 107E project?)
- Resorting to using the CPU only and focusing on polishing other aspects of the project, such as possibly: 
    - Allowing the users to save their own start state presets
    - Displaying on an LED matrix rather than HDMI monitor
    - Supporting more complex automata to simulate fluid dynamics
    - Supporting 1D cellular automata like Rule 30
    - Using ESP32 to customize the start state remotely

Regardless of the project's final shape, we hope to learn:
- How to read potentiometers and convert their signal to digital output using SPI protocol
- How to set up a basic file system (to store preset state data on the SD card)
- The role of the GPU within the Raspberry Pi and the specific advantages/differences that it carries, relative to CPU


## Hardware, budget
- 2 potentiometers to implement knobs for controls (will acquire from course supply)
- 1-2 buttons for selection (in hand)
- Analog-to-digital converters for the potentiometers (will acquire MCP3008 from course supply)
- Cardboard/foam box for the control panel (will buy)

Budget: <$10

## Major tasks, member responsibilities

- **Implement the automata module** - Sarah
    - Relatively straightforward task 
    - Build a cellular automata module on top of the graphics library to run 1-3 different types of cellular automata on an HDMI display (starting with the game of life). 
    - The library should be able to read start states from a specified file format and run the automata for a specified amount of steps/time (or indefinitely).
    - The library can be made more sophisticated as time allows, i.e. to write states to files as well.
- **File system** - Avi
    - The file system is needed to store and then select from state start presets. 
    - We plan to follow the provided guide on the CS 107E website to make a lightweight file system to store these presets, likely as image data.
- **Sensor input** - Both
    - Support potentiometers and button
    - Use SPI for analog-to-digital conversion
    - Possibly an etch-a-sketch style interface
- **Hardware controller board** - Both
    - Create the physical board to hold the controls
- **GPU speedups** - Both
    - We hope to enable nontrivially large-scale simulations by familiarizing with and then using the GPU

## Schedule, midpoint milestones
By next week, we hope to
- Implement the basic cellular automata library simulation
- Successfully interface with the file system
- Get the sensors fully working (and design the exact controls)
- Determine exact plan of action for GPU

## Resources needed, issues
Books/code/tools/materials
- We are having a hard time finding solidified information about the Raspberry Pi GPU and how to program it. We would definitely benefit from a guide to approaching it, if one exists, and possibly past code. (We saw that it was mentioned in the past projects page.)
- We would like the example SPI driver code for MCP3008 mentioned on the sensors page.

Possible risks
- We are concerned about successfully leveraging the GPU, since it appears to be slightly ambitious.
- We are also concerned that the project might be too simple if the GPU component fails (i.e. if we were to omit that component or use existing code rather than building from scratch).

Help needed
- We would appreciate guidance on the feasibility of our project and the risks described above.