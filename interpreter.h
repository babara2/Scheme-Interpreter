#include "talloc.h"
#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include <stdlib.h>

#ifndef INTERPRETER_H
#define INTERPRETER_H

struct Frame {
   Value *bindings;
   struct Frame *parent;
};
typedef struct Frame Frame;

void interpret(Value *tree);
Value *eval(Value *expr, Frame *frame);

#endif
