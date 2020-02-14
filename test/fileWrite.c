#include "syscall.h"


int
main()
{
    int result=-1;
    int consoleIn = 0;
    int consoleOut = 1;
    char *buffer;

    // RO=1     RW=2    APPEND=3
    int id = Open("some.html", 2);
    int size = Write("123456789abcdefg", 12, id);
    result = Close(id);
    Exit(result);
}
