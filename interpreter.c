/*
   Ammar Babar, Lindsey Chavez, Kelsey Krantz
   This is an implementation of an evaluator of Scheme code
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

//Exits in case of evaluation error
void evaluationError(){
   texit(EXIT_FAILURE);
}

//Looks for the symbol in the frame bindings or the parent frame
Value *lookUpSymbol(Value *symbol, Frame *frame){
   Value *listOfBindings = frame->bindings;
   if (listOfBindings->type == NULL_TYPE){
      if (frame->parent == NULL){
         printf("Binding not found\n");
         evaluationError();
         return symbol;
      }
      else{
         return lookUpSymbol(symbol, frame->parent);
      }
   }
   //going through the bindings
   while(!(listOfBindings->type == NULL_TYPE)){
      Value *head = car(listOfBindings);
      Value *currentSymbol = head->b.key;
      if (!strcmp(currentSymbol->s, symbol->s)){
         //check if binding is uninitialized
         if(head->b.val->type == NULL_TYPE){
            printf("ERROR: variable uninitialized.\n");
            evaluationError();
         }
         return head->b.val;
      }
      else{
         head = car(listOfBindings);
         listOfBindings = cdr(listOfBindings);
      }
   }
   //Check parent
   if (frame->parent == NULL){
      printf("Binding not found\n");
      evaluationError();
      return symbol;
   }
   else{
      return lookUpSymbol(symbol, frame->parent);
   }
}

//evaluates if statements
Value *evalIf(Value *args, Frame *frame){
   //error checks for bad syntax for if  
   if(args->type == NULL_TYPE || 
      car(args)->type == NULL_TYPE ||
      cdr(args)->type == NULL_TYPE || 
      car(cdr(args))->type == NULL_TYPE ||
      cdr(cdr(args))->type == NULL_TYPE ||
      car(cdr(cdr(args)))->type == NULL_TYPE){
      printf("ERROR: bad syntax for if statement.\n");
      evaluationError();
   }
   Value *condition = eval(car(args), frame);
   if (condition->type == BOOL_TYPE){
      if(!strcmp(condition->s,"#t")){ //when condition is true
         return eval(car(cdr(args)), frame);
      }
      //run when condition is false
      else{
         return eval(car(cdr(cdr(args))), frame);
      }
   }
   //anything else is considered true
   else{
      return eval(car(cdr(args)), frame);
   }
}

//checks if bindings to be added are repeated
int checkBindingRepeat(Value *symbol, Value *listOfBindings){
   while(listOfBindings->type != NULL_TYPE){
      if(!strcmp(symbol->s, car(listOfBindings)->b.key->s)){
         printf("ERROR: Duplicate binding name.\n");
         evaluationError();
      }
      listOfBindings = cdr(listOfBindings);
   }
   return 1;
}

//Evaluates let statements
Value *evalLet(Value *args, Frame *frame){
   Frame *newFrame = talloc(sizeof(Frame));
   newFrame->parent = frame;
   Value *listOfBindings = makeNull();
   if(args->type == NULL_TYPE ||
      cdr(args)->type == NULL_TYPE){
      printf("ERROR: invalid input for let statement\n");
      evaluationError();
   }
   Value *bindingsToAdd = car(args);
   //going over the list of bindings to add to frame
   while(bindingsToAdd->type != NULL_TYPE){
      if(bindingsToAdd->type != CONS_TYPE || 
         car(bindingsToAdd)->type != CONS_TYPE){
         printf("ERROR: invalid input for let statement\n");
         evaluationError();
      }
      Value *bindingToAdd = car(bindingsToAdd);
      Value *binding = talloc(sizeof(Value));
      binding->type = BINDING_TYPE;
      if(car(bindingToAdd)->type != SYMBOL_TYPE || 
         cdr(bindingToAdd)->type != CONS_TYPE){
         printf("ERROR: bad syntax in let\n");
         evaluationError();
      }
      if(checkBindingRepeat(car(bindingToAdd), listOfBindings)){
         binding->b.key = car(bindingToAdd);
         binding->b.val = eval(car(cdr(bindingToAdd)),frame);

         listOfBindings = cons(binding, listOfBindings);
         bindingsToAdd = cdr(bindingsToAdd);
      }
   }
   newFrame->bindings = listOfBindings;
   return eval(car(cdr(args)), newFrame); //evaluate body of let
}

//adds binding to a given frame when we know the symbol is in list of bindings
void sameFrameBindingAdder(Value *symbol, Value *newVal, Frame *frame){
   Value *listOfBindings = frame->bindings;
   //going through the bindings
   while(!(listOfBindings->type == NULL_TYPE)){
      Value *head = car(listOfBindings);
      Value *currentSymbol = head->b.key;
      if (!strcmp(currentSymbol->s, symbol->s)){
         head->b.val = newVal;
         break;
      }
      else{
         head = car(listOfBindings);
         listOfBindings = cdr(listOfBindings);
      }
   }
}

//adds bindings for letrec
void evalLetRecHelper2(Value *bindingsToAdd, Frame *frame){
   while(bindingsToAdd->type != NULL_TYPE){
      Value *bindingToAdd = car(bindingsToAdd);
      Value *symbol = car(bindingToAdd);
      Value *newVal = eval(car(cdr(bindingToAdd)), frame);
      sameFrameBindingAdder(symbol, newVal, frame);
      bindingsToAdd = cdr(bindingsToAdd);
   }
}

//checks if epxression to be bound have uninitliazed references
int evalLetRecHelper(Value *bindingsToAdd, Frame *frame){
   while(bindingsToAdd->type != NULL_TYPE){
      Value *bindingToAdd = car(bindingsToAdd);
      if(cdr(bindingToAdd)->type != CONS_TYPE){
         printf("ERROR: invalid input for letrec statement\n");
         evaluationError();
      }
      Value *temp = eval(car(cdr(bindingToAdd)), frame);
      bindingsToAdd = cdr(bindingsToAdd);
   }
   return 1;
}

//Evaluates let rec statements
Value *evalLetRec(Value *args, Frame *frame){
   Frame *newFrame = talloc(sizeof(Frame));
   newFrame->parent = frame;
   Value *listOfBindings = makeNull();
   if(args->type == NULL_TYPE ||
      cdr(args)->type == NULL_TYPE){
      printf("ERROR: invalid input for letrec statement\n");
      evaluationError();
   }
   Value *bindingsToAdd = car(args);
   //going over the list of bindings to add to frame
   while(bindingsToAdd->type != NULL_TYPE){
      if(bindingsToAdd->type != CONS_TYPE || 
         car(bindingsToAdd)->type != CONS_TYPE){
         printf("ERROR: invalid input for letrec statement\n");
         evaluationError();
      }
      Value *bindingToAdd = car(bindingsToAdd);
      Value *binding = talloc(sizeof(Value));
      binding->type = BINDING_TYPE;
      if(car(bindingToAdd)->type != SYMBOL_TYPE){
         printf("ERROR: bad syntax in let\n");
         evaluationError();
      }
      if(checkBindingRepeat(car(bindingToAdd), listOfBindings)){
         binding->b.key = car(bindingToAdd);
         binding->b.val = makeNull();

         listOfBindings = cons(binding, listOfBindings);
         bindingsToAdd = cdr(bindingsToAdd);
      }
   }
   //error checks and adds bindings
   bindingsToAdd = car(args);
   newFrame->bindings = listOfBindings;
   if(evalLetRecHelper(bindingsToAdd, newFrame)){
      evalLetRecHelper2(bindingsToAdd, newFrame);
   }
   return eval(car(cdr(args)), newFrame); //evaluate body of let
}

//helps let* evaluation by making new frames and adding bindings
Frame *evalLetStarHelper(Value *bindingToAdd, Frame *frame){
   Frame *newFrame = talloc(sizeof(Frame));
   newFrame->parent = frame;
   Value *listOfBindings = makeNull();
   Value *binding = talloc(sizeof(Value));
   binding->type = BINDING_TYPE;
   binding->b.key = car(bindingToAdd);
   binding->b.val = eval(car(cdr(bindingToAdd)),frame);
   listOfBindings = cons(binding, listOfBindings);
   newFrame->bindings = listOfBindings;
   return newFrame;
}

//Evaluates let* statements
Value *evalLetStar(Value *args, Frame *frame){
   Frame *newFrame = frame;
   Frame *oldFrame = frame;
   if(args->type == NULL_TYPE ||
      cdr(args)->type == NULL_TYPE){
      printf("ERROR: invalid input for let statement\n");
      evaluationError();
   }
   Value *bindingsToAdd = car(args);
   //going over the list of bindings to add to frame
   while(bindingsToAdd->type != NULL_TYPE){
      if(car(bindingsToAdd)->type == NULL_TYPE){
         printf("ERROR: invalid input for let statement\n");
         evaluationError();
      }
      Value *bindingToAdd = car(bindingsToAdd);
      if(car(bindingToAdd)->type != SYMBOL_TYPE){
         printf("ERROR: bad syntax in let\n");
         evaluationError();
      }
      newFrame = evalLetStarHelper(bindingToAdd, oldFrame);
      oldFrame = newFrame;
      bindingsToAdd = cdr(bindingsToAdd);
   }
   return eval(car(cdr(args)), newFrame); //evaluate body of let
}

//Evaluates define 
Value *evalDefine(Value *args, Frame *frame){
   Frame *newFrame = frame;
   while(newFrame->parent != NULL){
      newFrame=newFrame->parent;
   }
   Value *binding = talloc(sizeof(Value));
   binding->type = BINDING_TYPE;
   binding->b.key = car(args);
   binding->b.val = eval(car(cdr(args)), frame);
   newFrame->bindings = cons(binding, newFrame->bindings);
   Value *defineConfirm = talloc(sizeof(Value));
   defineConfirm->type= VOID_TYPE;
   return defineConfirm;
}

//function to recursively evaluate expressions to be passed to a function
Value *lambdaHelper(Value *args, Frame *frame){
   if(args->type == NULL_TYPE){
      Value *nullElem = makeNull();
      return nullElem;
   }
   if (cdr(args)->type==NULL_TYPE){
      Value *damn = eval(car(args), frame);
      Value *lastElem = makeNull();
      return cons(damn, lastElem);
   }
   else{
      return cons(eval(car(args), frame), lambdaHelper(cdr(args), frame)); 
   }
}

//Error Check: no duplicates in params
int checkArgRepeat(Value *arg, Value *paramList){
   while(paramList->type != NULL_TYPE){
      if(!strcmp(arg->s, car(paramList)->s)){
         printf("ERROR: Duplicate argument name.\n");
         evaluationError();
      }
      paramList = cdr(paramList);
   }
   return 1;
}

//checks for valid args
Value *checkArgs(Value *params){
   Value *tempList = makeNull();
   if(params->type != NULL_TYPE && params->type != CONS_TYPE 
      && params->type != SYMBOL_TYPE){
      printf("ERROR: lambda expression written incorrectly.\n");
      evaluationError();
   }
   else if(params->type == SYMBOL_TYPE){
      return params;
   }
   while(params->type != NULL_TYPE){
      if(car(params)->type != SYMBOL_TYPE){
         printf("ERROR: function param not an identifier\n");
         evaluationError();
      }
      if((checkArgRepeat(car(params), tempList))){
         tempList = cons(car(params), tempList);
      }
      params = cdr(params);
   }
   return reverse(tempList);
}

//Evaluates lambda
Value *evalLambda(Value *args, Frame *frame){
   Value *Closure = talloc(sizeof(Value));
   Closure->type = CLOSURE_TYPE;
   //Error checks
   if(args->type == NULL_TYPE){
      printf("Lambda expression doesn't contain params or body.\n");
      evaluationError();
   }
   //Error checks
   Closure->cl.params = checkArgs(car(args));
   if(cdr(args)->type == NULL_TYPE){
      printf("Lambda expression doesn't contain body.\n");
      evaluationError();
   }
   Closure->cl.body = car(cdr(args));
   Closure->cl.frame = frame;
   return Closure;
}

//function to add numbers
Value *primitiveAdd(Value *args){
   float sum = 0.0;
   while(args->type != NULL_TYPE){
      if(!(car(args)->type == INT_TYPE 
         || car(args)->type == DOUBLE_TYPE)){
         printf("ERROR: contract violation.\n");
         evaluationError();
      }
      if(car(args)->type == DOUBLE_TYPE){
         sum = sum + car(args)->d;
      }
      if(car(args)->type == INT_TYPE){
         sum = sum + (float)car(args)->i;
      }
      args = cdr(args);
   }
   Value *result = talloc(sizeof(Value));
   result->type = DOUBLE_TYPE;
   result->d = sum;
   return result;
}

//function to subtract numbers
Value *primitiveSubtract(Value *args){
   float diff = 0.0;
   int firstChecker = 0;
   if(length(args) < 1){
      printf("ERROR: Contract Violation. Expected one arg, none given.\n");
      evaluationError();
   }
   else if(length(args) == 1){
      if(!(car(args)->type == INT_TYPE || car(args)->type == DOUBLE_TYPE)){
            printf("ERROR: contract violation.\n");
            evaluationError();
      }
      if(car(args)->type == DOUBLE_TYPE){
         diff = -car(args)->d;
      }
      if(car(args)->type == INT_TYPE){
         diff = -(float)car(args)->i;
      }
   }
   else{
      while(args->type != NULL_TYPE){
         if(!(car(args)->type == INT_TYPE 
            || car(args)->type == DOUBLE_TYPE)){
            printf("ERROR: contract violation.\n");
            evaluationError();
         }
         if(car(args)->type == DOUBLE_TYPE && firstChecker == 0){
            diff = car(args)->d;
            firstChecker++;
         }
         else if(car(args)->type == INT_TYPE && firstChecker == 0){
            diff = (float)car(args)->i;
            firstChecker++;
         }
         else if(car(args)->type == DOUBLE_TYPE){
            diff = diff - car(args)->d;
         }
         else if(car(args)->type == INT_TYPE){
            diff = diff - (float)car(args)->i;
         }
         args = cdr(args);
      }
   }
   Value *result = talloc(sizeof(Value));
   result->type = DOUBLE_TYPE;
   result->d = diff;
   return result;
}

//function to multiply numbers
Value *primitiveMultiply(Value *args){
   float product=1.0;
    while(args->type != NULL_TYPE){
      if(!(car(args)->type == INT_TYPE 
         || car(args)->type == DOUBLE_TYPE)){
         printf("ERROR: contract violation.\n");
         evaluationError();
      }
      if(car(args)->type == DOUBLE_TYPE){
         product = product * car(args)->d;
      }
      if(car(args)->type == INT_TYPE){
      product = product * (float)car(args)->i;
      }
      args = cdr(args);
   }
   Value *result = talloc(sizeof(Value));
   result->type = DOUBLE_TYPE;
   result->d = product;
   return result;
}

//helper for primitive divide
float primitiveDivideHelper(Value *args, float quotient, int firstChecker){
   while(args->type != NULL_TYPE){
      if(!(car(args)->type == INT_TYPE || car(args)->type == DOUBLE_TYPE)){
         printf("ERROR: contract violation.\n");
         evaluationError();
      }
      if(car(args)->type == DOUBLE_TYPE && firstChecker == 0){
         if(car(args)->d == 0.0){
            printf("ERROR: Division by zero.\n");
            evaluationError();
         }
         quotient = car(args)->d;
         firstChecker++;
      }
      else if(car(args)->type == INT_TYPE && firstChecker == 0){
         if(car(args)->i == 0){
            printf("ERROR: Division by zero.\n");
            evaluationError();
         }
         quotient = (float)car(args)->i;
         firstChecker++;
      }
      else if(car(args)->type == DOUBLE_TYPE){
         if(car(args)->d == 0.0){
            printf("ERROR: Division by zero.\n");
            evaluationError();
         }
         quotient = quotient/car(args)->d;
      }
      else if(car(args)->type == INT_TYPE){
         if(car(args)->i == 0){
            printf("ERROR: Division by zero.\n");
            evaluationError();
         }
         quotient = quotient/(float)car(args)->i;
      }
      args = cdr(args);
   }
   return quotient;
}

//function to divide numbers
Value *primitiveDivide(Value *args){
    float quotient = 0.0;
   int firstChecker = 0; //checks if we are on first item
   if(length(args) < 1){
      printf("ERROR: Contract Violation. Expected one arg, none given.\n");
      evaluationError();
   }
   else if(length(args) == 1){
      if(!(car(args)->type == INT_TYPE || car(args)->type == DOUBLE_TYPE)){
            printf("ERROR: contract violation.\n");
            evaluationError();
      }
      if(car(args)->type == DOUBLE_TYPE){
         if(car(args)->d == 0.0){
            printf("ERROR: Division by zero.\n");
            evaluationError();
         }
         quotient = 1/car(args)->d;
      }
      if(car(args)->type == INT_TYPE){
         if(car(args)->i == 0){
            printf("ERROR: Division by zero.\n");
            evaluationError();
         }
         quotient = 1/(float)car(args)->i;
      }
   }
   else{
      quotient = primitiveDivideHelper(args, quotient, firstChecker);
   }
   Value *result = talloc(sizeof(Value));
   result->type = DOUBLE_TYPE;
   result->d = quotient;
   return result;
}

//helper function for primitiveLessThanEquals
char *primitiveLEqHelper(Value * firstItem, Value *secondItem, char *boolean){
    if(firstItem->type == DOUBLE_TYPE && secondItem->type == DOUBLE_TYPE){
      if(firstItem->d > secondItem->d){
         boolean = "#f";
      }
   }
   else if(firstItem->type == DOUBLE_TYPE && secondItem->type == INT_TYPE){
      if(firstItem->d > (float)secondItem->i){
         boolean = "#f";
      }
   }
   else if(firstItem->type == INT_TYPE && secondItem->type == DOUBLE_TYPE){
      if((float)firstItem->i > secondItem->d){
         boolean = "#f";
      }
   }
   else{
      if((float)firstItem->i > (float)secondItem->i){
         boolean = "#f";
      }
   }
   return boolean;
}

//function to check for <=
Value *primitiveLessThanEquals(Value *args){
   char *boolean = "#t";
   if(length(args) < 2){
      printf("ERROR: arity mismatch. Expected at least 2 args.\n");
   }
   else{
      Value *firstItem = car(args);
      args = cdr(args);
      while(args->type != NULL_TYPE){
         Value *secondItem = car(args);
         if(!(firstItem->type == INT_TYPE 
            || firstItem->type == DOUBLE_TYPE) ||
            !(secondItem->type == INT_TYPE 
               || secondItem->type == DOUBLE_TYPE)){
            printf("ERROR: contract violation.\n");
            evaluationError();
         }
         boolean = primitiveLEqHelper(firstItem, secondItem, boolean);
         firstItem = secondItem;
         args = cdr(args);
      }
   }
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = boolean;
   return result;
}

//primitive equals helper
char *primitiveEqHelper(Value *firstItem, Value * secondItem, char *boolean){
   if(firstItem->type == CLOSURE_TYPE || firstItem->type == CONS_TYPE){
      if(firstItem->type == CONS_TYPE && car(firstItem)->type == NULL_TYPE
         && car(secondItem)->type == NULL_TYPE){
         boolean = "#t";
      }
      else if(!(firstItem == secondItem)){
         boolean = "#f";
      }
   }
   else if(firstItem->type == BOOL_TYPE || firstItem->type == STR_TYPE){
      if(strcmp(firstItem->s, secondItem->s)){
         boolean = "#f";
      }
   }
   else if(firstItem->type == DOUBLE_TYPE){
      if(firstItem->d!=secondItem->d){
         boolean = "#f";
      }
   }
   else if(firstItem->type == INT_TYPE){
      if(firstItem->i != secondItem->i){
         boolean = "#f";
      }
   }
   return boolean;
}

//function to check for eq?
Value *primitiveEquals(Value *args){
   char *boolean = "#t";
   if(length(args)!=2){
      printf("ERROR: arity mismatch. Arguements not equal to 2.\n");
      evaluationError();
   }
   Value *firstItem = car(args);
   Value *secondItem = car(cdr(args));
   if(firstItem->type == VOID_TYPE || secondItem->type == VOID_TYPE){
      printf("ERROR: define not allowed in this context.\n");
      evaluationError();
   }
   if(firstItem->type != secondItem->type){
      boolean = "#f";
   }
   else{
      boolean = primitiveEqHelper(firstItem, secondItem, boolean);
   }
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = boolean;
   return result;
}

//checks if input argument is a pair
Value *primitivePair(Value *args){
   char *boolean = "#f";
   if(length(args) != 1){
      printf("ERROR: arity mismatch. Expected one argument.\n");
      evaluationError();
   }
   if(car(args)->type == VOID_TYPE){
      printf("ERROR: define not allowed in this context.\n");
      evaluationError();
   }
   else{
      if(car(args)->type == CONS_TYPE){
         boolean = "#t";
      }
   }
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = boolean;
   return result;
}

//helper function for check equal
char *EqHelper(Value * firstItem, Value *secondItem, char *boolean){
    if(firstItem->type == DOUBLE_TYPE && secondItem->type == DOUBLE_TYPE){
      if(firstItem->d != secondItem->d){
         boolean = "#f";
      }
   }
   else if(firstItem->type == DOUBLE_TYPE && secondItem->type == INT_TYPE){
      if(firstItem->d != (float)secondItem->i){
         boolean = "#f";
      }
   }
   else if(firstItem->type == INT_TYPE && secondItem->type == DOUBLE_TYPE){
      if((float)firstItem->i != secondItem->d){
         boolean = "#f";
      }
   }
   else{
      if((float)firstItem->i != (float)secondItem->i){
         boolean = "#f";
      }
   }
   return boolean;
}

//checks if two numbers are equal
Value *checkEqual(Value *args){
   char *boolean = "#t";
   if(length(args) < 2){
      printf("ERROR: arity mismatch. Expected at least 2 args.\n");
   }
   else{
      Value *firstItem = car(args);
      args = cdr(args);
      while(args->type != NULL_TYPE){
         Value *secondItem = car(args);
         if(!(firstItem->type == INT_TYPE || firstItem->type == DOUBLE_TYPE) ||
            !(secondItem->type == INT_TYPE 
               || secondItem->type == DOUBLE_TYPE)){
            printf("ERROR: contract violation.\n");
            evaluationError();
         }
         boolean = EqHelper(firstItem, secondItem, boolean);
         firstItem = secondItem;
         args = cdr(args);
      }
   }
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = boolean;
   return result;
}

//checks if the input is zero
Value *checkZero(Value *args){
   char *boolean = "#f";
   if(length(args) != 1){
      printf("ERROR: arity mismatch. Expected 1 argument.\n");
      evaluationError();
   }
   if(!(car(args)->type == INT_TYPE || car(args)->type == DOUBLE_TYPE)){
      printf("ERROR: contract violation.\n");
      evaluationError();
   }
   if(car(args)->type == DOUBLE_TYPE){
      if(car(args)->d == 0.0){
         boolean = "#t";
      }
   }
   if(car(args)->type == INT_TYPE){
      if(car(args)->i == 0){
         boolean = "#t";
      }
   }
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = boolean;
   return result;
}

//check if lists are the same
char *checkListEq(Value *firstItem, Value *secondItem, char *boolean){
   if(length(firstItem)!=length(secondItem)){
      boolean = "#f";
   }
   else{
      if(car(firstItem)->type == NULL_TYPE
         && car(secondItem)->type == NULL_TYPE){
         boolean = "#t";    
      }
      else{
         while(firstItem->type != NULL_TYPE && secondItem-> type != NULL_TYPE){
            if(car(firstItem)->type == VOID_TYPE 
               || car(secondItem)->type == VOID_TYPE){
               printf("ERROR: define not allowed in this context.\n");
               evaluationError();
            }
            if(car(firstItem)->type != car(secondItem)->type){
               boolean = "#f";
               break;
            }
            else{
               if(car(firstItem)->type != CONS_TYPE){
                  boolean = primitiveEqHelper(car(firstItem),
                   car(secondItem), boolean);
               }
               else{
                  boolean = checkListEq(car(firstItem),
                   car(secondItem), boolean);
               }
               if(!strcmp(boolean,"#f")){
                  break;
               }
               else{
                  firstItem = cdr(firstItem);
                  secondItem = cdr(secondItem);
               }
            }
         }
      }
   }
   return boolean;
}

//primitive equals helper
char *checkEqsHelper(Value *firstItem, Value *secondItem, char *boolean){
   if(firstItem->type == CONS_TYPE){
      boolean = checkListEq(firstItem, secondItem, boolean);
   }
   if(firstItem->type == CLOSURE_TYPE){
      if(firstItem->type == CONS_TYPE && car(firstItem)->type == NULL_TYPE
         && car(secondItem)->type == NULL_TYPE){
         boolean = "#t";
      }
      else if(!(firstItem == secondItem)){
         boolean = "#f";
      }
   }
   else if(firstItem->type == BOOL_TYPE || firstItem->type == STR_TYPE){
      if(strcmp(firstItem->s, secondItem->s)){
         boolean = "#f";
      }
   }
   else if(firstItem->type == DOUBLE_TYPE){
      if(firstItem->d!=secondItem->d){
         boolean = "#f";
      }
   }
   else if(firstItem->type == INT_TYPE){
      if(firstItem->i != secondItem->i){
         boolean = "#f";
      }
   }
   return boolean;
}

//function that checks equal
Value *checkEquals(Value *args){
   char *boolean = "#t";
   if(length(args)!=2){
      printf("ERROR: arity mismatch. Arguements not equal to 2.\n");
      evaluationError();
   }
   Value *firstItem = car(args);
   Value *secondItem = car(cdr(args));
   if(firstItem->type == VOID_TYPE || secondItem->type == VOID_TYPE){
      printf("ERROR: define not allowed in this context.\n");
      evaluationError();
   }
   if(firstItem->type != secondItem->type){
      boolean = "#f";
   }
   else{
      boolean = checkEqsHelper(firstItem, secondItem, boolean);

   }
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = boolean;
   return result;
}

//returns x modulo y
Value *findModulo(Value *args){
   int mod = 0;
   if(length(args) != 2){
      printf("ERROR: arity mismatch. Expected 2 arguments.\n");
      evaluationError();
   }
   if(car(args)->type != INT_TYPE || car(cdr(args))->type != INT_TYPE){
      printf("ERROR: contract violation. Expected intger.\n");
      evaluationError();
   }
   else{
      mod = car(args)->i % car(cdr(args))->i;
   }
   Value *result = talloc(sizeof(Value));
   result->type = INT_TYPE;
   result->i = mod;
   return result;
}

//function to retrieve car of list
Value *primitiveCar(Value *args){
   if(args->type == NULL_TYPE){
      printf("ERROR: contract violation.\n");
      evaluationError();
   }
   if(!(car(args)->type == CONS_TYPE)){
      printf("ERROR: contract violation.\n");
      evaluationError();
   }
   if((cdr(args)->type != NULL_TYPE)){
      printf("ERROR: contract violation.\n");
      evaluationError();
   }
   if(car(car(car(args)))->type == CONS_TYPE){
      return cons(car(car(car(args))), makeNull());
   }
   else{
   return car(car(car(args)));
   }
}

//function to retrieve cdr of list
Value *primitiveCdr(Value *args){
   if(args->type == NULL_TYPE){
      printf("ERROR: contrat violation.\n");
      evaluationError();
   }
   if(!(car(args)->type == CONS_TYPE)){
      printf("ERROR: contrat violation.\n");
      evaluationError();
   }
   if((cdr(args)->type != NULL_TYPE)){
      printf("ERROR: contract violation.\n");
      evaluationError();
   }
   if(cdr(car(car(args)))->type != CONS_TYPE){
      return cdr(car(car(args)));
   }
   else{
   return cons((cdr(car(car(args)))),makeNull());
   }
}

//cleans car values fed into cons
Value *carCleaner(Value *carOfList){
   if(carOfList->type == CONS_TYPE){
      if(length(carOfList) == 1 && car(carOfList)->type == CONS_TYPE){
         return car(carOfList);
      }
      else if(length(carOfList) == 1 && car(carOfList)->type != CONS_TYPE
         && car(carOfList)->type == NULL_TYPE){
         return car(carOfList);
      }
      else if(length(carOfList) == 1 && car(carOfList)->type != CONS_TYPE){
         return carOfList;
      }
      return carOfList;
   }
   else{
      return carOfList;
   }
}

//cleans cdr values fed into cons
Value *cdrCleaner(Value *cdrOfList){
   if(cdrOfList->type == CONS_TYPE){
      if(length(cdrOfList) == 1 && car(cdrOfList)->type == CONS_TYPE){
         return car(cdrOfList);
      }
      else if(length(cdrOfList) == 1 && car(cdrOfList)->type != CONS_TYPE
         && car(cdrOfList)->type == NULL_TYPE){
         return car(cdrOfList);
      }
      else if(length(cdrOfList) == 1 && car(cdrOfList)->type != CONS_TYPE){
         return cdrOfList;
      }
      return cdrOfList;
   }
      else{
         return cdrOfList;
      }
}

//function to cons two items together
Value *primitiveCons(Value *args){
   Value *consO = makeNull();
   if(length(args) != 2){
      printf("ERROR: arity mismatch,\n");
      evaluationError();
   }
   if(car(args)->type==VOID_TYPE || car(args)->type==NULL_TYPE){
      printf("ERROR: bad input for cons,\n");
      evaluationError();
   }
   if(car(cdr(args))->type==VOID_TYPE || car(cdr(args))->type==NULL_TYPE){
      printf("ERROR: bad input for cons,\n");
      evaluationError();
   }
   consO = cons(cons(carCleaner(car(args)), 
      cdrCleaner(car(cdr(args)))),consO);
   return consO;
}

//function to test is something is null
Value *primitiveNull(Value *args){
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   if(length(args) > 1){
      printf("ERROR: arity mismatch.\n");
      evaluationError();
   }
   if(args->type == NULL_TYPE){
      printf("ERROR: contract violation.\n");
      evaluationError();
   }
   if(car(args)->type == NULL_TYPE){
      printf("ERROR: contract violation.\n");
      evaluationError();
   }
   else if(car(args)->type != CONS_TYPE){
      result->s = "#f";
   }
   else if(car(car(args))->type == NULL_TYPE){
      result->s= "#t";
   }
   else{
      result->s="#f";
   }
   return result;
}
//Check if something is a proper list
int checkList(Value *args){
   int boolean = 1;
   if(args->type == VOID_TYPE){
      printf("ERROR: Define not allowed in this context.\n");
      evaluationError();
   }
   if(args->type != NULL_TYPE && args->type != CONS_TYPE){
      boolean = 0;
   }
   else{
      if(args->type != NULL_TYPE){

         while(args->type != NULL_TYPE){
            if(args->type != CONS_TYPE){
               boolean = 0;
               break;
            }
            args = cdr(args);
         }
      }
   }
   return boolean;
}

//helper method for append
Value *appendHelper(Value *lst1, Value* lst2){
   if(lst1->type == NULL_TYPE){
      return lst2;
   }
   else{
      return cons(car(lst1), appendHelper(cdr(lst1), lst2));
   }
}

//implementation of append for scheme
Value *appendMaker(Value *args){
   int len = length(args);
   Value *result = makeNull();
   if(len == 1){
      if(car(args)->type != VOID_TYPE){
         return car(args);
      }
      else{
         printf("ERROR: define not allowedin this context.\n");
         evaluationError();
      }
   }
   if(len > 1){
      Value *firstItem = car(args);
      args = cdr(args);
      while(args->type != NULL_TYPE){
         if(firstItem->type != CONS_TYPE || !checkList(car(firstItem))){
            printf("ERROR: expected mpair?\n");
            evaluationError();
         }
         Value *secondItem = car(args);
         if(car(args)->type == VOID_TYPE){
            printf("ERROR: define not allowedin this context.\n");
            evaluationError();
         }
         else if(car(args)->type == CONS_TYPE){
            result =  cons(appendHelper(car(firstItem),
             car(secondItem)), makeNull());
         }
         else{
            result = cons(appendHelper(car(firstItem), 
               secondItem), makeNull());
         }
         firstItem = result;
         args= cdr(args);
      }
   }
   return result;
}

//implementation of list for scheme
Value *listMaker(Value *args){
   Value *theList = makeNull();
   if(length(args) == 1){
      if(car(args)->type == NULL_TYPE || car(args)->type == VOID_TYPE){
         printf("ERROR: bad input for list.\n");
         evaluationError();
      }
      else{
         theList = cons(args, theList);
      }
   }
   else{
      while(args->type != NULL_TYPE){
         if(car(args)->type == CONS_TYPE){
            theList = cons(car(car(args)), theList);
            args = cdr(args);
         }
         else{
         theList = cons(car(args), theList);
         args = cdr(args);
         }
      }
   }
   theList = reverse(theList);
   theList = cons(theList, makeNull());
   return theList;
}

//function that ands args
Value *evalAnd(Value *args){
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = "#t";
   while(args->type != NULL_TYPE){
      if(car(args)->type == VOID_TYPE){
         printf("ERROR: define not allowed in this context.\n");
         evaluationError();
      }
      if(car(args)->type == BOOL_TYPE){
         if(!strcmp(car(args)->s, "#f")){
            result = car(args);
            break;
         }
      }
      result = car(args);
      args = cdr(args);
   }
   return result;
}

//function that ors args
Value *evalOr(Value *args){
   Value *result = talloc(sizeof(Value));
   result->type = BOOL_TYPE;
   result->s = "#t";
   while(args->type != NULL_TYPE){
      if(car(args)->type == VOID_TYPE){
         printf("ERROR: define not allowed in this context.\n");
         evaluationError();
      }
      if(car(args)->type == BOOL_TYPE){
         if(!strcmp(car(args)->s, "#f")){
            result = car(args);
            args = cdr(args);
         }
         else{
            result = car(args);
            break;
         }
      }
      else{
         result = car(args);
         break;
      }
   }
   return result;
}

//set! helper
void evalSetBangHelper(Value *symbol, Value *newVal, Frame *frame){
   int eyeReallyDidChange = 0;
   Value *listOfBindings = frame->bindings;
   if (listOfBindings->type == NULL_TYPE){
      if (frame->parent == NULL){
         printf("Binding not found\n");
         evaluationError();
      }
      else{
         evalSetBangHelper(symbol, newVal, frame->parent);
      }
   }
   //going through the bindings
   while(!(listOfBindings->type == NULL_TYPE)){
      Value *head = car(listOfBindings);

      Value *currentSymbol = head->b.key;
      if (!strcmp(currentSymbol->s, symbol->s)){
         head->b.val = newVal;
         eyeReallyDidChange = 1;
         break;
      }
      else{
         head = car(listOfBindings);
         listOfBindings = cdr(listOfBindings);
      }
   }
   //going to parent
   if (!eyeReallyDidChange && frame->parent == NULL){
      printf("Binding not found\n");
      evaluationError();
   }
   else if(!eyeReallyDidChange){
      evalSetBangHelper(symbol, newVal, frame->parent);
   }
}

//implementation of set!
Value *evalSetBang(Value *args, Frame *frame){
   if(length(args) != 2){
      printf("ERROR: bad syntax for set!\n");
      evaluationError();
   }
   if(car(args)->type != SYMBOL_TYPE 
      || car(cdr(args))->type == VOID_TYPE 
      || car(cdr(args))->type == NULL_TYPE){
      printf("ERROR: bad syntax for set!\n");
      evaluationError();
   }
   Value *symbol = car(args);
   Value *newVal = eval(car(cdr(args)), frame);
   evalSetBangHelper(symbol, newVal, frame);
   Value *defineConfirm = talloc(sizeof(Value));
   defineConfirm->type= VOID_TYPE;
   return defineConfirm;
}

//implementation of begin
Value *evalBegin(Value *args){
   Value *result = talloc(sizeof(Value));
   result->type = VOID_TYPE;
   while(args->type != NULL_TYPE){
      result = car(args);
      args = cdr(args);
   }
   return result;
}

//cond helper
int evalCondHelper(Value *cond, Frame *frame, int check){
   if(length(cond) != 2){
      printf("ERROR: we do not handle such foolishness.\n");
      evaluationError();
   }
   else{
      if(check == 1){
         if(car(cond)->type == SYMBOL_TYPE){
            if(!(strcmp(car(cond)->s, "else"))){
               return 1;
            }
         }
      }
      else if(check == 0){
         if(car(cond)->type == SYMBOL_TYPE){
            if(!(strcmp(car(cond)->s, "else"))){
               printf("ERROR: we do not handle such foolishness.\n");
               evaluationError();
            }
         }
         
      }
      //error checks for bad syntax for if  
      if(car(cond)->type == NULL_TYPE ||
         cdr(cond)->type == NULL_TYPE || 
         car(cdr(cond))->type == NULL_TYPE){
         printf("ERROR: bad syntax for cond statement.\n");
         evaluationError();
      }
      Value *condition = eval(car(cond), frame);
      if (condition->type == BOOL_TYPE){
         if(!strcmp(condition->s,"#t")){ //when condition is true
            return 1;
         }
         //run when condition is false
         else{
            return 0;
         }
      }
      //anything else is consiered true
      else{
         return 1;
      }
   }
  return 1;
}

//implement cond
Value *evalCond(Value *args, Frame *frame){
   int tracker = 0;
   int check = 0;
   Value *result = talloc(sizeof(Value));
   result->type = VOID_TYPE;
   while(args->type != NULL_TYPE){
      if(tracker == length(args)){
         check = 1;
      }
      if(car(args)->type != CONS_TYPE){
         printf("ERROR: bad syntax in cond.\n");
         evaluationError();
      }
      else{
         int resultInt = evalCondHelper(car(args), frame, check);
         if(resultInt){
            result = eval(car(cdr(car(args))), frame);
            break;
         }
         else{
            args = cdr(args);
         }
      }
      tracker++;
   }
   return result;
}

//applies lambda expressions
Value *apply(Value *function, Value *args){
   Frame *newFrame = talloc(sizeof(Frame));
   newFrame->parent= function->cl.frame;

   if(function->type == PRIMITIVE_TYPE){
      return (function->pf)(args);
   }
   Value *paramsList = function->cl.params;
   Value *listOfBindings = makeNull();
   if(paramsList->type == SYMBOL_TYPE){
      Value *binding = talloc(sizeof(Value));
      binding->type = BINDING_TYPE;
      binding->b.key = paramsList;
      binding->b.val = listMaker(args);
      listOfBindings = cons(binding, listOfBindings);
   }  
   //error check that params is the same size as arguments 
   else{
      if (length(function->cl.params) != length(args)){
      printf("ERROR: arity mismatch\n");
      evaluationError();
       }
      Value *argsList = args;
      while (paramsList->type != NULL_TYPE){ 
         Value *binding = talloc(sizeof(Value));
         binding->type = BINDING_TYPE;
         binding->b.key = car(paramsList);
         binding->b.val = car(argsList);
         listOfBindings = cons(binding, listOfBindings);
         paramsList = cdr(paramsList);
         argsList = cdr(argsList);
      }
   }
   newFrame->bindings = listOfBindings;
   //evaluate
   Value *lambdaConfirm = talloc(sizeof(Value));
   lambdaConfirm = eval(function->cl.body, newFrame);
   return lambdaConfirm;
}

//helps eval handle cons cases
Value *consHelper(Value *tree, Frame *frame){
   //if first is cons_type, should be lambda eventually
   Value *first = car(tree);
   Value *args = cdr(tree);
   if (first->type == CONS_TYPE){
      Value *isLambda = eval(first, frame);
      //expect closure type
      assert(isLambda->type == CLOSURE_TYPE); 
      args = lambdaHelper(args, frame);
      return apply(isLambda, args);
   }
   //cons type should only be args
   if(!(first->type == SYMBOL_TYPE)){
      printf("ERROR: Not a procedure.\n");
      evaluationError();
   }
   if (!strcmp(first->s, "if")) {
      return evalIf(args, frame);
   }
   if (!strcmp(first->s, "quote")){
      if(length(args) != 1){
         printf("ERROR: wrong number of parts in quote.\n");
         evaluationError();
      }
      return args;
   }
   if (!strcmp(first->s, "let")){
      return evalLet(args, frame);
   }
   if (!strcmp(first->s, "let*")){
      return evalLetStar(args, frame);
   }
   if (!strcmp(first->s, "define")){
      return evalDefine(args, frame);
   }
   if (!strcmp(first->s, "lambda")){
      return evalLambda(args, frame);
   }
   if (!strcmp(first->s, "set!")){
      return evalSetBang(args, frame);
   }
   if (!strcmp(first->s, "cond")){
      return evalCond(args, frame);
   }
   if (!strcmp(first->s, "letrec")){
      return evalLetRec(args, frame);
   }
   if (lookUpSymbol(first,frame)->type == CLOSURE_TYPE){
      args = lambdaHelper(args, frame);
      return apply(lookUpSymbol(first, frame), args);
   }
   if (lookUpSymbol(first,frame)->type == PRIMITIVE_TYPE){
      args = lambdaHelper(args, frame);
      return apply(lookUpSymbol(first, frame), args);
   }
   else {
      printf("Not a recognized special form\n");
      evaluationError();
   }
   return tree;
}

//Evals token in frame
Value *eval(Value *tree, Frame *frame) {
   switch (tree->type) {
      case INT_TYPE:
      case DOUBLE_TYPE:
      case STR_TYPE:
      case BOOL_TYPE:
         return tree;
      case PTR_TYPE:
      case OPEN_TYPE:
      case CLOSE_TYPE:
      case NULL_TYPE:
      case BINDING_TYPE:
      case VOID_TYPE:
      case CLOSURE_TYPE:
      case PRIMITIVE_TYPE:
         break;
      case SYMBOL_TYPE:
         return lookUpSymbol(tree, frame);
      case CONS_TYPE: {
         return consHelper(tree, frame);
      }
   }
   return tree; //returning the tree if input not evaluated
}

// Helper that goes over a list and prints the contents of a linked list.
void displayHelper3(Value *list){
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
      case VOID_TYPE:
         break;
      case CLOSURE_TYPE:
         printf("#procedure");
         break;
      case PRIMITIVE_TYPE:
         printf("#procedure");
         break;
      case BINDING_TYPE:
         displayHelper3(list->c.car);
         displayHelper3(list->c.cdr);
         break;
     case CONS_TYPE:
         displayHelper3(list->c.car);
         if(list->c.cdr->type != CONS_TYPE && list->c.cdr->type != NULL_TYPE){
         printf(". ");
         }
         displayHelper3(list->c.cdr);
         break;
   }
}

//Displays a evaluated expression to the screen
void printResult(Value *tree){
   while(tree->type != NULL_TYPE){
      if (tree->type == CONS_TYPE){
         if(car(tree)->type == NULL_TYPE){
            printf("( ");
            printf(") ");
         }
         if(car(tree)->type == CONS_TYPE){
            printf("( ");
            //recursively calling print on expr in (expr)
            printResult(car(tree)); 
            printf(") ");
         }
         else{
            displayHelper3(car(tree)); //print non cons-type tokens
         }
         if(cdr(tree)->type != CONS_TYPE && cdr(tree)->type != NULL_TYPE){
            printf(". ");
           }
         if(cdr(tree)->type==CONS_TYPE && length(cdr(tree))==1){
            displayHelper3(cdr(tree));
            tree = makeNull();
         }
         else{
            tree = cdr(tree);
         }
      }
      else{
         displayHelper3(tree);
         break;
      }
   }
}

//binds a function to a symbol
void bind(char *name, Value *(function)(Value *), Frame *frame){
   Value *value = makeNull();
   value->type = PRIMITIVE_TYPE;
   value->pf = function;
   Value *symbol = talloc(sizeof(Value));
   symbol->type = SYMBOL_TYPE;
   symbol->s = name;
   Value *binding = talloc(sizeof(Value));
   binding->type = BINDING_TYPE;
   binding->b.key = symbol;
   binding->b.val = value;
   frame->bindings = cons(binding, frame->bindings);
}

//Makes a top frame and evaluates each s-expression inside it
void interpret(Value *tree){
   Frame *top = talloc(sizeof(Frame));
   top->bindings = makeNull();
   top->parent = NULL; //this is the top frame
   bind("+", primitiveAdd, top);
   bind("car", primitiveCar, top);
   bind("cdr", primitiveCdr, top);
   bind("null?", primitiveNull, top);
   bind("cons", primitiveCons, top);
   bind("-", primitiveSubtract, top);
   bind("*", primitiveMultiply, top);
   bind("/", primitiveDivide, top);
   bind("<=", primitiveLessThanEquals, top);
   bind("eq?", primitiveEquals, top);
   bind("pair?", primitivePair, top);
   bind("=", checkEqual, top);
   bind("zero?", checkZero, top);
   bind("equal?", checkEquals, top);
   bind("modulo", findModulo, top);
   bind("list", listMaker, top);
   bind("append", appendMaker, top);
   bind("and", evalAnd, top);
   bind("or", evalOr, top);
   bind("begin", evalBegin, top);
   while(tree->type != NULL_TYPE){
      Value *result = eval(car(tree), top);
      if(!(result->type == VOID_TYPE || result->type == NULL_TYPE)){
         printResult(result);
         printf("\n");
      }
      if(result->type == NULL_TYPE){
         printf("()\n");
      }
      tree= cdr(tree);
   }
}