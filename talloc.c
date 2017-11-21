/*
Ammar Babar, Lindsey Chavez, Kelsey Krantz
This is an implementation of a rudimentary garbage
collector.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "talloc.h"

//Global active list
static Value *activeList;

/*
 * Create an empty active list (a new Value object of type NULL_TYPE).
 */
Value *tMakeNull(){
	Value *head = malloc(sizeof(Value));
	head->type = NULL_TYPE;
	return head;
}

/*
 * Create a nonempty active list (a new Value object of type CONS_TYPE).
 */
Value *tCons(Value *car, Value *cdr){
	Value *node = malloc(sizeof(Value));
	node->type = CONS_TYPE;
	node->c.car = car;
	node->c.cdr = cdr;
	return node;
}


/*
 * Get the car value of a given list.
 */
Value *tCar(Value *list){
	assert(list !=NULL);
	assert(list->type==CONS_TYPE);
	return list->c.car;
}

/*
 * Get the cdr value of a given list.
 */
Value *tCdr(Value *list){
	assert(list !=NULL);
	return list->c.cdr;
}

/*
 * Test if the given value is a NULL_TYPE value.
 */
bool tIsNull(Value *value){
	assert(value != NULL);
	if (value->type == NULL_TYPE){
		return true;
	}
	else{
		return false;
	}
}

/*
 * Compute the length of the given active list.
 */
int tLength(Value *value){
  assert(value !=NULL);
	switch (value->type){
		case NULL_TYPE:
		   return 1;
		case PTR_TYPE:
		   return 1;
		case INT_TYPE:
		   return 1;
		case DOUBLE_TYPE:
		   return 1;
		case STR_TYPE:
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
           return (1 + tLength(value->c.cdr));
	}
}

/*
 * A malloc-like function that allocates memory, tracking all allocated
 * pointers in the "active list."
 */
void *talloc(size_t size){
   if (activeList == NULL){
   	activeList = tMakeNull();
   }
   void *newPtr = malloc(size);
   Value *valuePtr = malloc(sizeof(Value));
   valuePtr->type = PTR_TYPE;
   valuePtr->p = newPtr;
   activeList = tCons(valuePtr, activeList);
   return newPtr; 
}

/*
 * Free all pointers allocated by talloc, as well as whatever memory you
 * malloc'ed to create/update the active list.
 */
void tfree(){
	int i;
	int length = tLength(activeList);
   for (i = 0; i < length; i++){
   	if (tIsNull(activeList)){
   		free(activeList);
        activeList = NULL;
   	}
   	else{
   		Value *temp = tCar(activeList);
   		free(temp->p);
   		free(temp);
   		Value *temp2 = activeList;
   		activeList = tCdr(activeList);
   		free(temp2);
   	}
   }
}

/*
 * A simple two-line function to stand in the C function "exit", which calls
 * tfree() and then exit().
 */
void texit(int status){
	tfree();
	exit(status);
}