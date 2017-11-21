/*
   Ammar Babar, Lindsey Chavez, Kelsey Krantz
   This is an implementation of a parser for scheme inputs in C.
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

/*Takes a linked list of tokens from a Scheme program and returns a pointer 
to a parse tree representing that program */
Value *parse(Value *tokens){
	Value *tree = makeNull(); //make empty tree
	int depth  = 0; //initializing depth to zero
	Value *stack = makeNull(); //making an empty stack
	Value *current = tokens;
	assert(current != NULL && "Error (parse): null pointer");
	while (current->type != NULL_TYPE){
		Value *token = car(current);
		if ((depth == 0 && (token->type) == CLOSE_TYPE) || depth < 0){
			printf("Syntax error: too many close parentheses\n");
			texit(EXIT_FAILURE);
		}
		if (token->type == OPEN_TYPE){
			depth++;
		}
		if (depth == 0 && token->type != OPEN_TYPE 
			&& token->type != CLOSE_TYPE){
			tree = cons(token,tree); //saw atom, added to tree
		}
		if (depth > 0){
			if (token->type == CLOSE_TYPE){
				depth--;
				Value *expr = makeNull(); //expression to be stack or tree
				while (car(stack)->type != OPEN_TYPE){
					expr = cons(car(stack), expr); 
					stack = cdr(stack);
				}
				stack = cdr(stack);
				if (depth == 0){
					tree = cons(expr, tree); //adding to tree since depth is 0
				} 
				else{
					stack = cons(expr, stack); //adding expr to stack 
				}
			}
			else{
				stack = cons(token,stack); //push token on stack 
			}
		}
		current = cdr(current); //updating the current tokens list
	}
	if (depth > 0){
		printf("SYNTAX ERROR: not enough close parentheses.\n");
        texit(EXIT_FAILURE);
	}
    else if (depth < 0){
       printf("SYNTAX ERROR: too many close parentheses.\n");
       texit(EXIT_FAILURE);
       }
    return reverse(tree); //reversing to preserve order
}

/* Helper that goes over a list and prints the contents of a linked list. */
void displayHelper2(Value *list){
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
		   printf("\"%s\" ", list->s);
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
		case BINDING_TYPE:
		   break;
		case VOID_TYPE:
           break;
        case CLOSURE_TYPE:
           break;
        case PRIMITIVE_TYPE:
         break;
        case CONS_TYPE:
           displayHelper2(list->c.car);
           displayHelper2(list->c.cdr);
           break;
	}
}

/*Displays a parse tree to the screen, using parentheses 
to denote tree structure*/
void printTree(Value *tree){
   while(tree->type != NULL_TYPE){
	   if(car(tree)->type == CONS_TYPE){
	      printf("( ");
	      printTree(car(tree)); //recursively calling print on expr in (expr)
	      printf(") ");
	   }
	   else{
	      displayHelper2(car(tree)); //print non cons-type tokens
	   }
	   tree = cdr(tree);
	}
}