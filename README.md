# Scheme Interpreter

This project implements a Scheme Interpreter for RSR5 in C. The project 
builds off a linkedlist represented as a cons type (Scheme), and uses Value 
structs(see value.h) to store information about each token seen in the 
input. These tokenized inputs(generated using the tokenizer in tokenizer.c) 
are parsed using a modified/simplified recursive descent parser, and 
are finally interpreted(see interpreter.c for details). The evaluation 
is done using an environment (and frame) model.

Limitations
===========
The program does not handle all scheme commands. For details, look at the 
bindings and special forms in interpreter.c.

Usage
===========
To Run:
1. Go to Directory
2. `make interpreter < testFile` 
3. `make memtest < testFile` for memory tests using Valgrind.
4. `make clean` to remove all compiled files.

`testFile` could be selected from the list of test cases in directory:
`test-cases`

Credits
===========
Big thanks to Kelsey Krantz and Lindsey Chavez for providing solid support.
Prof. Jed Yang was an invaluable resource in figuring out problems with the 
interpreter. All test cases were tested in racket which was an great resource.
