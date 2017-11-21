/*
   Ammar Babar, Lindsey Chavez, Kelsey Krantz
   This is an implementation of a tokenizer for scheme inputs in C.
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

//SETTING A BUFFER SIZE FOR HOW BIG WE WANT OUR STRINGS AND SYMBOLS
static const int BUFFER_SIZE = 1000;

//HELPER METHOD FOR TOKENIZING SYMBOLS
Value *symbolMaker(Value *list, char charRead, char* symbols){
	char *symString = talloc(sizeof(char)*BUFFER_SIZE); //STRING TO BE POINTED TO BY SYMBOL
	int indexTracker = 0;
	Value *symVal = talloc(sizeof(Value)); //MAKING VALUE OBJECT TO STORE SYMBOL
	symVal->type = SYMBOL_TYPE;
    while (isalpha(charRead) || strchr(symbols, charRead) || isdigit(charRead)){
		symString[indexTracker] = charRead;
	    indexTracker++;
	    charRead = fgetc(stdin);
	}
	symString[indexTracker] = '\0';
	symVal->s = symString;
	list = cons(symVal, list);
	charRead = ungetc(charRead, stdin); //PUSHING UNUSED SYMBOL BACK ON stdin
	return list;
}

//HELPER METHOD FOR TOKENIZING DOUBLES
Value *doubleMaker(int indexTracker, char *numArray, Value *list){
	numArray[indexTracker] = '.'; //
	indexTracker++;
	char charRead = fgetc(stdin);
	while(isdigit(charRead)){
    	numArray[indexTracker]=charRead;
  	    indexTracker++;
  	    charRead=fgetc(stdin);
  	}
  	if(charRead == ' ' || charRead == ')' || charRead == ';' || charRead == '\n' || charRead == EOF){ //ACCEPTABLE CHARACTERS

  	   Value *dVal = talloc(sizeof(Value));
	   dVal->type = DOUBLE_TYPE;
       dVal->d = atof(numArray);
       list = cons(dVal, list);
  	}
  	else{
  		printf("Bad input. \n");
  		texit(EXIT_FAILURE);
  	}
    charRead = ungetc(charRead ,stdin);
	return list;
}	

//Helper to tokenize Booleans
Value *boolMaker(Value *list, char charRead){
	charRead = fgetc(stdin);
	if (charRead == 't' || charRead == 'T'){ //TRUE CASE
		charRead = fgetc(stdin);
		if(charRead == ' ' || charRead == '\n' || charRead == EOF || charRead == ')' 
		|| charRead == ';' || charRead == '\t'){ //ACCEPTABLE CHARACTERS
			Value *tBool = talloc(sizeof(Value));
		    tBool->type = BOOL_TYPE;
		    tBool->s = "#t"; 
		    list = cons(tBool, list);
	    }
	    else{
	    	printf("Bad input\n");
	    	texit(EXIT_FAILURE);
	    }

	}
	else if (charRead == 'f' || charRead == 'F'){ //FALSE CASE
		charRead = fgetc(stdin);
		if(charRead == ' ' || charRead == '\n' || charRead == EOF || charRead == ')'
		|| charRead == ';' || charRead == '\t'){ //ACCEPTABLE CHARACTERS
			Value *tBool = talloc(sizeof(Value));
		    tBool->type = BOOL_TYPE;
		    tBool->s = "#f"; 
		    list = cons(tBool, list);
	    }
	    else{
	    	printf("Bad input\n");
	    	texit(EXIT_FAILURE);
	    }
	}
	else{
		printf("Bad input\n");
		texit(EXIT_FAILURE);
	}
	charRead = ungetc(charRead ,stdin);
	return list;
}

//Helper to tokenize strings
Value *stringMaker(Value *list, char charRead){
	Value *string = talloc(sizeof(Value)); //MAKING THE VALUE OBJECT
			string->type = STR_TYPE;
			int indexTracker = 0;
			char *theString = talloc(sizeof(char)*BUFFER_SIZE); //STRING TO BE STORED BY THE VALUE OBJECT  
		    charRead = fgetc(stdin);
		    while ( charRead != '"'){
		    	//Checking escaped characters
		    	if (charRead == '\\'){
		    		charRead = fgetc(stdin);
		    		if (charRead == 'n'){
		    			theString[indexTracker] = '\n';
		    			indexTracker++;
		    		}
		    		if (charRead == 't'){
		    			theString[indexTracker] = '\t';
		    			indexTracker++;
		    		}
		    		if (charRead == '\\'){
		    			theString[indexTracker] = '\\';
		    			indexTracker++;
		    		}
		    		if (charRead == '\''){
		    			theString[indexTracker] = '\'';
		    			indexTracker++;
		    		}
		    		if (charRead == '"'){
		    			theString[indexTracker] = '"';
		    			indexTracker++;
		    		}		
		    	}
                //Checking if end of file is reached before a closing "
                else if(charRead== EOF){
                    printf("Syntax error, endless string\n");
                    texit(EXIT_FAILURE);  
                  }
                //Adding the remaining characters to our string array
                else{
                   theString[indexTracker] = charRead;
                   indexTracker++;
                }
                charRead = fgetc(stdin);
		    }
		    theString[indexTracker] = '\0';
		    string->s = theString;
            list = cons(string, list);
            return list;
		}

//Helper for tokenizing numbers
Value *numMaker(Value *list, char charRead){
  char numArray[BUFFER_SIZE] = ""; //ARRAY TO HOLD CHARS TO BE CONVERTED TO DIGITS
  int indexTracker=0;
  if (charRead == '.'){
  	charRead = fgetc(stdin);
  	if(isdigit(charRead)){
  		charRead = ungetc(charRead, stdin);
  		list = doubleMaker(indexTracker, numArray, list);
  		return list;
  	}
  	else{
  	 	printf("Bad input\n");
  		texit(EXIT_FAILURE);
  	}
  }
  if(charRead == '+'){ //PLUS SIGN
  	charRead=fgetc(stdin);
  	if(isdigit(charRead)){
  		numArray[indexTracker]= '+';
  		indexTracker++;
  	}
  	else if(charRead == '.'){ //CHECKING FOR FLOAT
  		numArray[indexTracker]= '+';
  		indexTracker++;
  		charRead = fgetc(stdin);
  	    if(isdigit(charRead)){
  		   charRead = ungetc(charRead, stdin);
  		   list = doubleMaker(indexTracker, numArray, list);
  		   return list;
  	    }
  	    else{
  	 	   printf("Bad input\n");
  		   texit(EXIT_FAILURE);
  	    }
  	}
  	else if(charRead == ' ' || charRead == '\n' || charRead == EOF ||
  	 charRead == ')' || charRead == ';' || charRead == '\t'){ //ACCEPTABLE CHARACTERS
  	   Value *plusSymbol = talloc(sizeof(Value));
	   plusSymbol->type = SYMBOL_TYPE;
       plusSymbol->s = "+";
       list = cons(plusSymbol, list);
  	}
  	else{
  		printf("Bad input\n");
  		texit(EXIT_FAILURE);
  	}
  }

  else if(charRead == '-'){ // MINUS SIGN
  	charRead=fgetc(stdin);
  	if(isdigit(charRead)){
  		numArray[indexTracker]= '-';
  		indexTracker++;
  	}
    else if(charRead == '.'){ //CHECKING FOR FLOAT
    	numArray[indexTracker]= '-';
  		indexTracker++;
  		charRead = fgetc(stdin);
  		if(isdigit(charRead)){
  		   charRead = ungetc(charRead, stdin);
  		   list = doubleMaker(indexTracker, numArray, list);
  		   return list;
  	    }
  	    else{
  	 	   printf("Bad input\n");
  		   texit(EXIT_FAILURE);
  	    }
  	}
  	else if(charRead == ' ' || charRead == '\n' || charRead == EOF ||
  	 charRead == ')' || charRead == ';' || charRead == '\t'){ //ACCEPTABLE CHARACTERS
  	   Value *minusSymbol = talloc(sizeof(Value));
	   minusSymbol->type = SYMBOL_TYPE;
       minusSymbol->s = "-";
       list = cons(minusSymbol, list);
  	}
  	else{

  		printf("Bad input\n");
  		texit(EXIT_FAILURE);
  	}
  }

  if(isdigit(charRead)){ //CHECKING IF NUMBER SEEN
  	while(isdigit(charRead)){
    	numArray[indexTracker]=charRead;
  	    indexTracker++;
  	    charRead=fgetc(stdin);
  	}
  	if(charRead == '.'){ //CHECKING FOR DOUBLE
  		list = doubleMaker(indexTracker, numArray, list);
  		return list;
  	}
  	else if(charRead == ' ' || charRead == ')' || charRead == ';' || charRead == '\n' || charRead == EOF){ //ACCEPTABLE CHRACATERS

  	   Value *integer = talloc(sizeof(Value));
	   integer->type = INT_TYPE;
       integer->i = atoi(numArray);
       list = cons(integer, list);
  	}
  	else{
  		printf("Bad input. \n");
  		texit(EXIT_FAILURE);
  	}
  }
  charRead = ungetc(charRead, stdin);
  return list;
}

Value *tokenize(){
	Value *list = makeNull();
    char charRead = fgetc(stdin);
    char symbols[] = "+-!$%&*/:<>^=?~_";
	while (charRead != EOF) {
		/* OPEN Case */
		if(charRead == '(') {
			Value *open = talloc(sizeof(Value));
			open->type = OPEN_TYPE;
			list = cons(open, list);
			charRead = fgetc(stdin);
		}
		/* CLOSE Case */
		else if(charRead == ')') {
			Value *close = talloc(sizeof(Value));
			close->type = CLOSE_TYPE;
			list = cons(close, list);
			charRead = fgetc(stdin);
		}
		/* BOOL Case */
		else if(charRead == '#') {
			list = boolMaker(list, charRead);
			charRead = fgetc(stdin);
		}
		/* STRING Case*/	
        else if (charRead == '"'){
        	list = stringMaker(list, charRead);
        	charRead = fgetc(stdin);
        }
        	
        /*NUMBER Case*/
        else if(isdigit(charRead) || charRead== '+'|| charRead== '-' || charRead== '.'){
          list = numMaker(list, charRead);
          charRead = fgetc(stdin);
       }
       //SYMBOL CASE: 
       else if(isalpha(charRead) || strchr(symbols, charRead)){
       	       list = symbolMaker(list, charRead, symbols);
       	       charRead = fgetc(stdin);
		}
		//COMMENT CASE:
		else if(charRead == ';'){
			while(charRead != EOF && charRead != '\n'){
				charRead = fgetc(stdin);
			}
		}
		//NEGOTIATING SPACES AND TABS
		else if(charRead == ' ' || charRead == '\n' || charRead == '\t'){
			charRead = fgetc(stdin);
		}
		//EVERYTHING ELSE
		else{
			printf("Bad input son.\n");
			texit(EXIT_FAILURE);
		}
   }
   return reverse(list); //RETURNING REVERSE OF LIST SINCE WE CONS TO THE FRONT ALWAYS
}

//DISPLAYS THE TOKENS IN THE LIST
void displayTokens(Value *list){
	assert(list->type != PTR_TYPE); //WE DONT WANT A POINTER TO BE A TOKEN
    switch (list->type){
		case NULL_TYPE:
		   break;
		case INT_TYPE:
		   printf("%d : integer \n", list->i);
		   break;
		case DOUBLE_TYPE:
		   printf("%f : double \n", list->d);
		   break;
		case STR_TYPE:
		   printf("%s : string \n", list->s);
		   break;
		case PTR_TYPE:
		   break;
		case CLOSE_TYPE:
		   printf(") :close\n");
		   break;
	    case OPEN_TYPE:
		   printf("( :open\n");
		   break;
	    case BOOL_TYPE:
		   printf("%s :boolean\n", list->s);
		   break;
	    case SYMBOL_TYPE:
		   printf("%s :symbol\n", list->s);
		   break;
		case BINDING_TYPE:
		   displayTokens(list->c.car);
           displayTokens(list->c.cdr);
           break;
        case VOID_TYPE:
           break;
        case CLOSURE_TYPE:
           break;
        case PRIMITIVE_TYPE:
           break;
        case CONS_TYPE:
           displayTokens(list->c.car);
           displayTokens(list->c.cdr);
           break;
	}
}