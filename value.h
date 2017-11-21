#ifndef VALUE_H
#define VALUE_H

typedef enum {
   PTR_TYPE,
   INT_TYPE,
   DOUBLE_TYPE,
   STR_TYPE,
   CONS_TYPE,
   BOOL_TYPE,
   SYMBOL_TYPE,
   OPEN_TYPE,
   CLOSE_TYPE,
   BINDING_TYPE,
   NULL_TYPE,
   VOID_TYPE,
   CLOSURE_TYPE,
   PRIMITIVE_TYPE
} valueType;

struct Value {
   valueType type;
   union {
      void *p;
      int i;
      double d;
      char *s;
      struct ConsCell {
         struct Value *car;
         struct Value *cdr;
      } c;
      struct binding{
         struct Value *key;
         struct Value *val;
      } b;
      struct Closure{
         struct Value *params;
         struct Value *body;
         struct Frame *frame;
      } cl;
      struct Value *(*pf)(struct Value*);
   };
};

typedef struct Value Value;


#endif
