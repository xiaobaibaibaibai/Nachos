/* add.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int
main()
{
  int result;
  
  result = Write("123456789abcdefg", 15, 1);
  
  Exit(0);
  /* not reached */
}
