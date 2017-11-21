# Scheme Interpreter

This project implements a Scheme Interpreter for RSR5 in C. The project 
builds off a linkedlist represented as a cons type (Scheme), and uses Value 
structs(see value.h) to store information about each token seen in the 
input. These tokenized inputs(generated using the tokenizer in tokenizer.c) 
are parsed using a modified/simplified recursive descent parser, and 
are finally interpreted(see interpreter.c for details). The evaluation 
is done using an environment (and frame) model.




Usage
===========
To Run:
1. Go to Directory
2. Make 
3. ./interpreter < testFile
4. Check for memory leaks: make memtest < testFile
5. Have fun:)



Credits
===========
Big thanks to Kelsey Krantz and Lindsey Chavez for providing solid support.
Prof. Jed Yang was an invaluable resource in figuring out problems with the 
interpreter. All test cases were tested in racket which was an great resource.