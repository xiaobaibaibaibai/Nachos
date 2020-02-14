#include "syscall.h"


int
main()
{
    int result=-1;
    int id =-1;
    int size = 0;
    
    // 1: X    2: W    3: WX
    // 4: R    5: RX   6: RW
    result = Create("some.html", 6);
    Exit(result);
}
