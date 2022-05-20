# Starter Kernel
This is the starter kernel code for the spring 2022 offering of CSC 395: Advanced Operating Systems at Grinnell College. This project includes the basic elements required to build and run a simple kernel on an x86_64 machine emulated with QEMU.

## Acknowledgements
This starter code is based on the following example projects:
- [OSDev.org Bare Bones Kernel](https://wiki.osdev.org/Bare_bones)
- [Stivale2 Barebones Kernel](https://github.com/stivale/stivale2-barebones)

In addition to the above example projects, the following references were used when setting up this code:
- [OSDev.org GCC Cross Compiler Instructions](https://wiki.osdev.org/GCC_Cross-Compiler)
- [Stivale2 Reference](https://github.com/stivale/stivale/blob/master/STIVALE2.md)

# Project Implementation
Our group chose to attempt to create a debugging system that lives in the kernel. Our goals were to create a breakpoint macro `BREAK` that would trigger a debug interrupt, display a simple interactive debug interface when the interrupt is called, and include more informative error messages. We chose to build on top of the kernel that Sam Eagen (@sve009) implemented over the course of the first half of the course. 

You can find most debug code in `kernel/debug.*` files. The new interrupt can be found in `kernel/idt.c` file in `exception3`. 

General Running
---------------
To run the debugging interface, `#include "debug.h"` in the file you wish to place a breakpoint, then write `BREAK` where you want to add a breakpoint. Make and run your project and you will see a debug prompt pop up. From there, you can do a couple of different commands:
- _continue_: this will exit the breakpoint and continue execution of your program 
- _print_: there are three options for printing
    * You can print the working stack by typing `print stack`
    * You can print the value at a specific address by typing `print [address]`, where the address is preceded by `0x`. If you do not enter the address in this format, you will be shown an error message and then re-prompted for a command
        * If you simply type `print`, you will be prompted for an address
    * You can print out a null terminated string with `print string [address]`, which will print all memory values in ascii format until the next 0.
If you enter any other commands, an error message will give you a list of valid prompts and re-prompt for your input

Out of the Box
--------------
If you make and run the program as is, you will find that there is a `BREAK` statement already implemented in `kernel/boot.c` on line 190. This causes a breakpoint to trigger before allowing you to run any modules. To ignore this breakpoint, either comment it out of the code, or type continue to proceed to the user modules. 

Pro Tips and Other Things to Know
---------------------------------
There are a couple of thing that may help you intuit the use of this simple debugger. 
1. Please notice that the backspace functionality in this kernel is a little bit funky. Hitting it at the beginnings of lines can cause some errors.
    * One way to get around this issue if it comes up is by printing to the terminal exactly what you want to enter into the debugging prompt right before you call the breakpoint. Then, you can just copy and paste instead of typing. 
2. We were unfortunately unable to implement single-step execution. However, you can somewhat get around this by inserting a breakpoint after each step that is executed. By repeatedly using the `continue` option, you can essentially walk thought your code intruction-by-instruction similar to how single-step mode works. 
