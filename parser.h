#include "talloc.h"
#include "value.h"
#include "linkedlist.h"
#include <stdlib.h>

#ifndef PARSER_H
#define PARSER_H

/*Takes a linked list of tokens from a Scheme program and 
returns a pointer to a parse tree representing that program */
Value *parse(Value *tokens);

/*Displays a parse tree to the screen, using parentheses 
to denote tree structure*/
void printTree(Value *tree);

#endif
