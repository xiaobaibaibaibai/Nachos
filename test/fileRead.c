#include "syscall.h"


int
main()
{
    int result=-1;
    int consoleIn = 0;
    int consoleOut = 1;
    int size = 25;
    char *buffer;
    
    // RO=1     RW=2    APPEND=3
    int id = Open("some.html", 2);
    size = Read(buffer, size, id);
    size = Write(buffer, size, consoleOut);
    result = Close(id);
    Exit(result);
}   
