## Cellular Automata Simulator

## Team members
- Avi Udash
- Sarah Chen

## Project description
We seek to create a program that simulates cellular automata on an HDMI display. We will extend the graphics library to support common automata such as the game of life, Langton's loop, etc.

We will support various pre-set start states (which we will store on the SD card using a basic filesystem) that users can select from using a hardware control board.

The board will also include two knobs and a button that will enable users to select which automata to display, to select which start state to use, to customize individual pixels within the grid, and to start the simulation.

We hope to support relatively large scale simulations and will do so by leveraging the GPU. This is a stretch goal because we would need to write code to download code to the GPU and call that code. Fallback positions would include
- Building off of existing GPU libraries (possibly from the past CS 107E project?)
- Resorting to using the CPU only and focusing on polishing other aspects of the project. (We could try to take on another challenge instead, such as using ESP32 to customize the start state remotely. We could also potentially allow the users to create and save their own start states.)

We aim to learn how to read potentiometers and convert their signal to digital output using SPI protocol. We also hope to learn more about the role of the GPU within the Raspberry Pi and the specific advantages/differences that it carries, relative to CPU. 


## Hardware, budget
- 2 potentiometers to implement knobs for controls (will acquire from course supply)
- 1 button for selection (in hand)
- Analog-to-digital converters for the potentiometers (will acquire MCP3008 from course supply)
- Cardboard/foam box for the control panel (will buy)

Budget: <$10

## Major tasks, member responsibilities
Major task breakdown and who is responsible for each task.

- **Implement the automata on HDMI display** - Sarah
    - Relatively straightforward task 
    - Build a cellular automata module on top of the graphics library to support 1-3 different types of cellular automata (starting with the game of life). The library should be able to read start states from a specified file format and run the automata for a specified amount of steps/time (or indefinitely). It should display on a double-buffered display.
    - Optional: the library should write states to files as well.
- **File system** - Avi
    - The file system is needed to store and then select from pre-settings 
    - We plan to follow the provided guide on the CS 107E website to make a lightweight file system
- **Hardware controller board** - Both
    - Create the actual board
- **Sensor input** - Avi
    - Support potentiometers and button
    - SPI for analog-to-digital conversion
- **GPU speedups** - Both
    - We hope to enable nontrivially large-scale simulations by familiarizing with and then using the GPU

## Schedule, midpoint milestones
By next week, we hope to
- Implement the basic cellular automata library that can interface with the file system
- Set up the sensors 
- Learn more about GPU to determine exact plan of action

## Resources needed, issues
Books/code/tools/materials
- We are having a hard time finding solidified information about the Raspberry Pi GPU and how to program it. We would definitely benefit from a guide to approaching it, if one exists, and possibly past code. (We saw that it was mentioned in the past projects page.)
- We would like the example SPI driver code for MCP3008 mentioned on the sensors page.

Possible risks
- We are concerned about being able to support a large-scale, fast-paced simulation using the GPU. 
- We are concerned that the project might be too simple if the GPU component fails (or if we were to use existing code to support it rather than building from scratch).

Help needed
- We would appreciate guidance on the feasibility of our project and the risks described above.