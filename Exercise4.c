one 
#define MIN(a,b) (a < b) ? (a) : (b)

The correct answer is the following: #define min(a, b)  ((a) < (b) ? (a) : (b)) // The parentheses were missing.

two
int xto3(volatile int *x){
        return *x * *x * *x;
    }
	
The answer: 
/* volatile int *x is a pointer to an int that the compiler will treat as volatile. 
This means that the compiler will assume that it is possible for the variable that x is pointing at to have changed even if there is nothing in the source code to suggest that this might occur.
For example, if I set x to point to a regular integer, then every time I read or write *x the compiler is aware that the value may have changed unexpectedly.
You may be end up multiplying different numbers because it's volatile and could be changed unexpectedly. The correct code is as follow:*/

int xto3(volatile int *x)
{
int a = *x;
return a*a*a;
}

Four 

 int return0(int a, int *b) {
        *b=0;
        *a=42;
        return *b;
    }

//The answer: In order to return the value 0, just simply we can remove the pointers and we will have the following code:

 int return0(int a, int b) {
        b=0;
        a=42;
        return b;
    }	
