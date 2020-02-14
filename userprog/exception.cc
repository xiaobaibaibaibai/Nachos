// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
        case SyscallException: {
            int r4 = (int)kernel->machine->ReadRegister(4);
            TranslationEntry *pageEntry = kernel->machine->pageTable;
            int vpn = r4 / PageSize;
            int offset = r4 % PageSize;
            int PPN = pageEntry[vpn].physicalPage;
            int addr4 = PPN * PageSize + offset;
            
            switch(type) {
                case SC_Halt: {
	                DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
	                SysHalt();
	                ASSERTNOTREACHED();
	                break;
                }
                case SC_Add: {
                    DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
                            /* int op2 */(int)kernel->machine->ReadRegister(5));
                    DEBUG(dbgSys, "Add returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Exit: {
                     
                    DEBUG(dbgSys, "Exit: " << kernel->machine->ReadRegister(4) << "\n");
                    SysExit(addr4);
                    // kernel->currentThread->Finish();
                    
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Exec: {
                     
                    DEBUG(dbgSys, "Execute: " << kernel->machine->ReadRegister(4) << "\n");
                    SpaceId result = SysExec(addr4);
                    DEBUG(dbgSys, "Execute returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Join: {
                     
                    DEBUG(dbgSys, "Join: " << kernel->machine->ReadRegister(4) << "\n");
                    int result = SysJoin(addr4);
                    DEBUG(dbgSys, "Join returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_ThreadFork: {
                     
                    DEBUG(dbgSys, "Fork: " << kernel->machine->ReadRegister(4) << "\n");
                    ThreadId result = SysFork(addr4);
                    DEBUG(dbgSys, "Fork returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Create: {
                     
                    DEBUG(dbgSys, "Create " << kernel->machine->ReadRegister(4) << " "<< 
                                                kernel->machine->ReadRegister(5) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysCreate(addr4, 
                                        (int)kernel->machine->ReadRegister(5));
                    DEBUG(dbgSys, "Create returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Remove: {
                     
                    DEBUG(dbgSys, "Remove() " << kernel->machine->ReadRegister(4) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysRemove(addr4);
                    DEBUG(dbgSys, "Remove() returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Open: {
                     
                    DEBUG(dbgSys, "Open() " << kernel->machine->ReadRegister(4) << " " <<
                                                kernel->machine->ReadRegister(5) << "\n");
                    /* Process SysAdd Systemcall*/
                    OpenFileId result;
                    result = SysOpen(addr4,
                                    (int)kernel->machine->ReadRegister(5));
                    DEBUG(dbgSys, "Open() returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Write: {
                     
                    DEBUG(dbgSys, "Write() " << kernel->machine->ReadRegister(4) << " " <<
                                 (int)kernel->machine->ReadRegister(5) << " " <<
                                 (int)kernel->machine->ReadRegister(6) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysWrite(addr4,
                                    (int)kernel->machine->ReadRegister(5),
                                    (int)kernel->machine->ReadRegister(6));
                    DEBUG(dbgSys, "Write() returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Read: {
                     
                    DEBUG(dbgSys, "Read() " << kernel->machine->ReadRegister(4) <<
                                 (int)kernel->machine->ReadRegister(5) << 
                                 (int)kernel->machine->ReadRegister(6) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysRead(addr4,
                                    (int)kernel->machine->ReadRegister(5),
                                    (int)kernel->machine->ReadRegister(6));
                    DEBUG(dbgSys, "Read() returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Seek: {
                     
                    DEBUG(dbgSys, "Seek() " << kernel->machine->ReadRegister(4) <<
                                 (int)kernel->machine->ReadRegister(5) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysSeek(addr4,
                                    (int)kernel->machine->ReadRegister(5));
                    DEBUG(dbgSys, "Seek() returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                case SC_Close: {
                     
                    DEBUG(dbgSys, "Close() " << kernel->machine->ReadRegister(4) << "\n");
                    /* Process SysAdd Systemcall*/
                    int result;
                    result = SysClose(addr4);
                    DEBUG(dbgSys, "Close() returning with " << result << "\n");
                    /* Prepare Result */
                    kernel->machine->WriteRegister(2, (int)result);
                    /* Modify return point */
                    {
                    /* set previous programm counter (debugging only)*/
                    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
                    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
                    /* set next programm counter for brach execution */
                    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                    }
                    return;
                    ASSERTNOTREACHED();
                    break;
                }
                default: {
	                cerr << "Unexpected system call " << type << "\n";
	                break;
                }
            }
            break;
        }
        case PageFaultException: {
		    //Fetch the virtual address where has PageFaultException
            int pageFaultAddress = (int)kernel->machine->ReadRegister(BadVAddrReg);
            //Fetch the virtual page number of the thread's pageTable
            int vpn = (int)pageFaultAddress / PageSize;
            //Check the free physical page number in main memory	
            int PPN = kernel->freeMap->FindAndSet();
            //Fetch the page fault entry of the currentThread
            TranslationEntry* pageEntry = kernel->currentThread->space->getPageEntry(vpn);
            if(PPN != -1){ //Free physical page
                //Update the page entry
                pageEntry->physicalPage = PPN;
                pageEntry->valid = TRUE;
                cout << "   PPN=" << PPN << endl;
                
                //Read data from swapSpace file and copy it into main memory
                kernel->swapSpace->ReadAt(
                    &(kernel->machine->mainMemory[PPN * PageSize]), 
                    PageSize,
                    pageEntry->virtualPage * PageSize);

                //Update FIFOEntryList, append the used physical page at the end of list
                kernel->FIFOEntryList->Append(pageEntry);
            }
            return;
            break;
        }
        default: {
            cerr << "Unexpected user mode exception" << (int)which << "\n";
            break;
        }
    }
    ASSERTNOTREACHED();
}
