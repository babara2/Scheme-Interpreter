/*
Ammar Babar, Lindsey Chavez, Kelsey Krantz
This is an implementation of linked lists in C.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "linkedlist.h"
#include "talloc.h"


/*
 * Create an empty list (a new Value object of type NULL_TYPE).
 */
Value *makeNull(){
	Value *head = talloc(sizeof(Value));
	head->type = NULL_TYPE;
	return head;
}

/*
 * Create a nonempty list (a new Value object of type CONS_TYPE).
 */
Value *cons(Value *car, Value *cdr){
	Value *node = talloc(sizeof(Value));
	node->type = CONS_TYPE;
	node->c.car = car;
	node->c.cdr = cdr;
	return node;
}

/*
 * Helper that goes over a list and prints the contents of a linked list.
 */
void displayHelper(Value *list){
	switch (list->type){
		case NULL_TYPE:
		   break;
		case INT_TYPE:
		   printf("%d ", list->i);
		   break;
		case DOUBLE_TYPE:
		   printf("%f ", list->d);
		   break;
		case STR_TYPE:
		   printf("%s ", list->s);
		   break;
		case PTR_TYPE:
		   printf("%p ", list->p);
		   break;
		case CLOSE_TYPE:
		   printf(") ");
		   break;
	    case OPEN_TYPE:
		   printf("( ");
		   break;
	    case BOOL_TYPE:
		   printf("%s ", list->s);
		   break;
	    case SYMBOL_TYPE:
		   printf("%s ", list->s);
		   break;
		case VOID_TYPE:
         break;
        case CLOSURE_TYPE:
           break;
		case BINDING_TYPE:
		   break;
		case PRIMITIVE_TYPE:
         break;
        case CONS_TYPE:
           displayHelper(list->c.car);
           displayHelper(list->c.cdr);
           break;
	}
}

/*
 * Print a representation of the contents of a linked list.
 */
void display(Value *list){
	printf("[ ");
	displayHelper(list);
	printf("]\n");
}

/*
 * Get the car value of a given list.
 * (Uses assertions to ensure that this is a legitimate operation.)
 */
Value *car(Value *list){
	assert(list !=NULL);
	assert(list->type==CONS_TYPE);
	return list->c.car;
}

/*
 * Get the cdr value of a given list.
 * (Uses assertions to ensure that this is a legitimate operation.)
 */
Value *cdr(Value *list){
	assert(list !=NULL);
	assert(list->type==CONS_TYPE);
	return list->c.cdr;
}

/*
 * Test if the given value is a NULL_TYPE value.
 * (Uses assertions to ensure that this is a legitimate operation.)
 */
bool isNull(Value *value){
	assert(value != NULL);
	if (value->type == NULL_TYPE){
		return true;
	}
	else{
		return false;
	}
}

/*
 * Compute the length of the given list.
 * (Uses assertions to ensure that this is a legitimate operation.)
 */
int length(Value *value){
  assert(value !=NULL);
	switch (value->type){
		case NULL_TYPE:
		   return 0;
		case INT_TYPE:
		   return 1;
		case DOUBLE_TYPE:
		   return 1;
		case STR_TYPE:
		   return 1;
		case PTR_TYPE:
		   return 1;
		case CLOSE_TYPE:
		   return 1;
	    case OPEN_TYPE:
		   return 1;
	    case BOOL_TYPE:
		   return 1;
	    case SYMBOL_TYPE:
		   return 1;
		case BINDING_TYPE:
		   return 2;
		case VOID_TYPE:
           return 1;
        case CLOSURE_TYPE:
           return 1;
        case PRIMITIVE_TYPE:
          return 1;
        case CONS_TYPE:
           return (1 + length(value->c.cdr));
	}
}


/*Helper for the reverse function that reverses the copied values.*/
Value *reverseHelper(Value *list, Value *result){
  assert(list !=NULL);
	switch (list->type){
		case NULL_TYPE:
          return result;
		case INT_TYPE:
          return list;
		case DOUBLE_TYPE:
          return list;
		case STR_TYPE:
          return list;
        case PTR_TYPE:
          return list;
        case CLOSE_TYPE:
		   return list;
	    case OPEN_TYPE:
		   return list;
	    case BOOL_TYPE:
		   return list;
	    case SYMBOL_TYPE:
		   return list;
		case BINDING_TYPE:
		   return list;
		case VOID_TYPE:
            return list;
        case CLOSURE_TYPE:
           return list;
        case PRIMITIVE_TYPE:
            return list;
        case CONS_TYPE:
          assert(list->c.cdr !=0);
          return reverseHelper((list->c.cdr), cons((list->c.car), result));

	}
}

/* Creates a new reversed list and frees the old one*/
Value *reverse(Value *list){
	Value *result = makeNull();
	Value *newList = reverseHelper(list, result);
    return newList;
}