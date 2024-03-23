# cube-bench-classic
A very simple benchmark involving a cube for Classic Mac OS System 7

I wanted something a bit more relatable than a bar chart or flashing patterned rectangles to get a feel for performance between classic Macs (LC III, etc.). So for #Marchintosh 2024, I wrote my own benchmark with the classic bouncing cube(s). A relatable mix of floating point, regular computation, and QuickDraw calls.

![Screenshot of the benchmark](screenshot.png)

## Compilation

Developed with Symantec C++ for Macintosh 7.0, using a mix of real hardware, working in BasiliskII, or in VS Code. Mostly in BasiliskII.

To compile, open the Project Manager and create a new Mac Application Project. Add the contents of cubes/ and then add a few libraries:

* ANSI++
* CPlusLib
* MacTraps (this should be there by default)

You may want to set Symantec C++ and THINK C options to make use of your hardware FPU if present:

* Generate 68881 instructions
* Use 881 for transcendental

## Usage

Command-R to run from within Symantec C++. To quit, click anywhere outside the window.

There are a lot of keyboard commands, check out the but switch statement in [main.cpp](cubes/main.cpp) for details.

At some point the h key will bring up on screen help.

Tested under System 7.5.3 on an LC, LCII, LCII, LC475, and BasiliskII 1.0.0_p20240224. Seems to crash in BasiliskII with a large window size for reasons unknown.

Performance is displayed as TPF, or ticks per frame. One tick corresponds to one screen refresh, and so will be about 1/60th of a second for most people.
