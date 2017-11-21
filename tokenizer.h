#include "talloc.h"
#include "value.h"
#include "linkedlist.h"
#include <stdlib.h>

#ifndef TOKENIZER_H
#define TOKENIZER_H

/*
   Reads the input stream and returns a pointer to a linkedlist
   consisting of tokens.
*/
Value *tokenize();

/*
   Takes a list of tokens and displays them, with their type.
*/
void displayTokens(Value *list);

#endif