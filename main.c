/*
   Ammar Babar, Lindsey Chavez, Kelsey Krantz
   Main program that runs the interpreter
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "talloc.h"
#include "value.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"


int main(void) {
   Value *list = tokenize();
   Value *tree = parse(list);
   interpret(tree);
   tfree();
   return 0;
}