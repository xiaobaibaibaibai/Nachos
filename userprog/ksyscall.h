/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

// ./nachos -cp ../test/fileCreate create -cp ../test/fileWrite write -cp ../test/fileRead read
// ./nachos -x create -x write -x read

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include <string.h>

char* getStringInMem(int addr) {
    char *buffer = new char[100];

    for (int i=0; i<100; i++) {
        buffer[i] = kernel->machine->mainMemory[addr+i];
        if (buffer[i] == '\0') {
            break;
        }
    }
    buffer[99] = '\0';
    return buffer;
}


void Copy(char *from, char *to)
{
    int fd;
    OpenFile* openFile;
    int amountRead, fileLength;
    char *buffer;

    // Open UNIX file
    if ((fd = OpenForReadWrite(from,FALSE)) < 0) {       
        printf("Copy: couldn't open input file %s\n", from);
        return;
    }

    // Figure out length of UNIX file
    Lseek(fd, 0, 2);            
    fileLength = Tell(fd);
    Lseek(fd, 0, 0);

    // Create a Nachos file of the same length
    DEBUG('f', "Copying file " << from << " of size " << fileLength <<  " to file " << to);
    if (!kernel->fileSystem->Create(to, fileLength, 7)) {   // Create Nachos file
        printf("Copy: couldn't create output file %s\n", to);
        Close(fd);
        return;
    }
    
    openFile = kernel->fileSystem->Open(to);
    ASSERT(openFile != NULL);
    
    // Copy the data in TransferSize chunks
    buffer = new char[128];
    while ((amountRead=ReadPartial(fd, buffer, sizeof(char)*128)) > 0)
        openFile->Write(buffer, amountRead);    
    delete [] buffer;

    // Close the UNIX and the Nachos files
    delete openFile;
    Close(fd);
    cout << "cp " << from << " => " << to << endl;
}

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

// execute a sub-process
void executeProgram(void *filename) {
    AddrSpace *space = new AddrSpace;
    ASSERT(space != (AddrSpace *)NULL);
    if (space->Load((char*)filename)) {  // load the program into the space
        space->Execute();         // run the program
    } else {
        cout << filename << ": command not found" << endl;
        kernel->currentThread->Finish();
    }
}

// if not current thread, use sleep to block until child exit
int SysJoin(int childId) {
    if (childId != kernel->currentThread->tid) {
        IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
        kernel->currentThread->Sleep(false);
        (void) kernel->interrupt->SetLevel(oldLevel);
    }
    return 1;
}

// if status is normal, wake parent thread up.
void SysExit(int status) {
    cout << kernel->currentThread->getName() << ": Exit: " << status << endl;
    if (status > -1) {
        IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
        Thread* pt = kernel->currentThread->getParent();
        if (pt) {
            kernel->tmpBool = true;
            kernel->scheduler->ReadyToRun(pt);
        }
        kernel->currentThread->Finish();
        (void) kernel->interrupt->SetLevel(oldLevel);
    } else {
        cout << "Exit wrong: " << status << endl;
    }
}

// restore to thread to run
void resumeFork(int which) {
    kernel->currentThread->RestoreUserState();
    kernel->machine->Run();
}

int SysFork(int r4) {
    Thread *t = new Thread("threadFork");
    t->space = new AddrSpace(*(kernel->currentThread->space));
    t->SaveUserState();
    t->updateRegister(r4);
    t->Fork((VoidFunctionPtr) resumeFork, (void *) 0);
    kernel->scheduler->Print();
    kernel->currentThread->Yield();

    return t->tid;
}

// create 
int SysCreate(int nameAddr, int protection) {
    bool result = FALSE;
    char *fileName = getStringInMem(nameAddr);
    if (fileName==NULL) {
        return -1;
    }

    result = kernel->fileSystem->Create(fileName, 0, protection);
    
    if (result) {
        // cout << kernel->currentThread->getName() << ": create " << fileName << " successful " << endl;
    } else {
        cout << kernel->currentThread->getName() << ": create " << fileName << " fail" << endl;
    }
    
    return result ? 1 : -1;
}

// mark as remove in kernel level and that file will be remove
// when no on refers it.
int SysRemove(int nameAddr) {
    cout << "------------In SysRemove()------------" << endl;
    bool result = FALSE;
    char *fileName = getStringInMem(nameAddr);
    cout << "remove file name: " << fileName << endl;
    result = kernel->currentThread->tft->Remove(fileName);
    return result;
}

// check request mode is valid
bool validMode(OpenFile *file, int mode) {
    if (file == NULL) {
        return false;
    }
    int protection = file->GetProtect();
    // no authority
    if (protection <= 0) {
        cout <<  " no authority protection: " << protection << endl;
        return false;
    }
    if (protection != 7) {
        // read only
        if (mode==1 && !(protection==4 || protection==6 || protection==5)) {
            cout << "Invalid mode for Read" << endl;
            return false;
        }
        // read and write
        else if (mode==2 && protection != 6) {
            cout << "Invalid mode for RW" << endl;
            return false;
        }
        // APPEND
        else if (mode==3 && !(protection==2 || protection==3 || protection==6)) {
            cout << "Invalid mode for Append" << endl;
            return false;
        }
    }
    // implement APPEND
    if (mode==3) {
        file->Seek(file->Length());
    }
    return true;
}

/*
1. file is opened by current thread: 
    (1) check whether mode is valid for protection
    (3) set mode
    (2) return id
2. file is opened by other threads but not by current thread: 
    (1) check whether mode is valid for protection
    (2) add file to current thread file table and set mode
    (3) set seek position for current thread
3. file is never opened
    (1) fileSystem->Open(fileName)
    (2) check whether mode is valid for protection
    (3) add file to current thread file table and kernel file table
    (4) set seek position for current thread 
*/

OpenFileId SysOpen(int nameAddr, int mode) {
    char *fileName = getStringInMem(nameAddr);
    if (fileName == NULL) {
        cout << kernel->currentThread->getName() << ": SysOpen fileName is NULL " << endl;
        return -1;
    }
    OpenFileId id = -1;
    OpenFile *file = NULL;
    
    // file is opened by current thread
    id = kernel->currentThread->tft->IsOpen(fileName);
    if (id > -1) {
        file = kernel->currentThread->tft->getFile(id, 2);
        if (validMode(file, mode)) {
            kernel->currentThread->tft->SetMode(id, mode);
            // cout << kernel->currentThread->getName() << ": SysOpen case1 successfull: " << id << endl;
            return id;
        }
        return -1;
    }
    
    // file is opened by other threads but not by current thread: 
    id = kernel->kft->IsOpen(fileName);
    if (id > -1) {
        file = kernel->kft->getFile(id);
        kernel->kft->AddReferences(id);
        if (validMode(file, mode)) {
            id = kernel->currentThread->tft->Add(id, fileName, mode);
            if (id > -1) { // set seek position
                file->SetSeekPosition(0);
            }
            // cout << kernel->currentThread->getName() << ": SysOpen case2 successfull: " << id << endl;
            return id;
        }
        return -1;
    }
    
    // file is never opened before
    file = kernel->fileSystem->Open(fileName);
    if (file==NULL) {
        cout << kernel->currentThread->getName() << ": SysOpen() " << fileName << " not exit" << endl;
        return -1;
    }
    if (!validMode(file, mode)) {
        return -1;
    }
    // first time adding will also add to kernel level file table
    id = kernel->currentThread->tft->Add(file, fileName, mode);
    if (id > -1) { // set seek position
        file->SetSeekPosition(0);
        // cout << kernel->currentThread->getName() << ": SysOpen case3 successfull: " << id << endl;
    } else {
        cout << kernel->currentThread->getName() << " open " << fileName << " fail"<< endl;
    }
    return id;
}

// Close file in thread level and decrease references in kernel level.
int SysClose(OpenFileId id) {
    if (id == -1) {
        cout << kernel->currentThread->getName() << ": SysClose fail " << endl;
        return id;
    }
    int result = -1;
    result = kernel->currentThread->tft->Close(id);
    return result;
}

// base on OpenFileId to write to file or console
int SysWrite(int contentAddr, int size, OpenFileId id) {
    if (id == -1 || size <= 0) {
        cout << kernel->currentThread->getName() << ": SysWrite fail, " <<
        "id=" << id << ", size=" << size << endl;
        return -1;
    }

    int result = -1;
    char *buffer = getStringInMem(contentAddr);
    // check specific size of input 
    if (size > (unsigned)strlen(buffer)) {
        size = (unsigned)strlen(buffer);
    }
    // write to console
    if (id==ConsoleOutput) {
        kernel->ioLock->Acquire();
        for (int i=0; i<size; i++) {
            kernel->synchConsoleOut->PutChar(buffer[i]);
        }
        if (strcmp(buffer, "--") != 0) {
            cout << endl;
        }
        result = size;
        kernel->ioLock->Release();
    } else {
        // write to file
        kernel->ioLock->Acquire();
        // open requested file
        OpenFile *file = kernel->currentThread->tft->getFile(id, 1);
        if (file==NULL) {
            cout << kernel->currentThread->getName() << ": SysWrite(): " << id << " not exit" << endl;
            result = -1;
        } else {
            result = file->Write(buffer, size);
        }
        kernel->ioLock->Release();
    }
    
    delete [] buffer;
    return result;
}

// base on OpenFileId to read from file or console
int SysRead(int buffAddr, int size, OpenFileId id) {
    if (id == -1 || size == -1) {
        cout << kernel->currentThread->getName() << ": SysRead fail, " <<
        "id=" << id << ", size=" << size << endl;    
        return -1;
    }
    int result = -1;
    char *buffer = new char[size];
    // read from console
    if (id==ConsoleInput) {
        kernel->ioLock->Acquire();
        for (int i=0; i<size; i++) {
            buffer[i] = kernel->synchConsoleIn->GetChar();
        }
        result = size;
        kernel->ioLock->Release();
    } else {
        // read from file
        kernel->ioLock->Acquire();
        OpenFile *file = kernel->currentThread->tft->getFile(id, 0);
        if (file==NULL) {
            cout << kernel->currentThread->getName() << ": SysRead() " << id << " not exit" << endl;
            result = -1;
        } else {
            result = file->Read(buffer, size);
        }
        kernel->ioLock->Release();
    }
    // write to main memory
    if (result > 0) {
        kernel->memLock->Acquire();
        for (int i=0; i<result; i++) {
            kernel->machine->mainMemory[buffAddr+i] = buffer[i];
        }
        kernel->memLock->Release();
    } else {
        cout << kernel->currentThread->getName() << ": loading : " << result << " to mainMemory " << endl;
    }
     
    delete [] buffer;
    return result;
}

int SysSeek(int position, OpenFileId id) {
    if (id == -1) {
        return id;
    }
    int result = -1;
    // cout << "seek position: " << position << ", file id: " << id << endl;
    OpenFile *file = kernel->currentThread->tft->getFile(id, 3);
    if (file==NULL) {
        cout << kernel->currentThread->getName() << ": SysSeek() " << id << " not exit" << endl;
        result = -1;
    } else {
        file->Seek(position);
        result = 1;
    }
    return result;
}


int SysExec(int nameAddr) {
    char *input = getStringInMem(nameAddr);
    if (input==NULL) {
        return -1;
    }
    // parse command
    if (strcmp(input, "ls") == 0) {
        kernel->fileSystem->List();
    } else if (strcmp(input, "pwd") == 0) {
        cout << "/" << endl; 
    } else if (input[0]=='c' && input[1]=='d' && input[2]==' ') {
        char *address = &(input[3]);

    } else if (input[0]=='m' && input[1]=='k' && input[2]=='d' 
            && input[3]=='i' && input[4]=='r' && input[5]==' ') {
        char *dirName = &(input[6]);
        kernel->fileSystem->MakeDirectory(dirName, 0, 1);
    } else if (input[0]=='c' && input[1]=='p' && input[2]==' ') {
        char *argus = &(input[3]);
        int len1 = -1, len2 = -1;
        int i = 0;
        while (argus[i] != ' ' && argus[i] != '\0') {
            len1++;
            i++;
        }
        len1++;
        char *name1 = new char[len1+1];
        for (int j=0; j<len1; j++) {
            name1[j] = argus[j];
        }
        name1[len1] = '\0';
        argus = &(input[3+len1+1]);
        i = 0;
        while (argus[i] != ' ' && argus[i] != '\0') {
            // cout << argus[i];
            len2++;
            i++;
        }
        len2++;
        char *name2 = new char[len1+1];
        for (int j=0; j<len1; j++) {
            name2[j] = argus[j];
        }
        name2[len2] = '\0';
        Copy(name1, name2);
    } else if (input[0]=='m' && input[1]=='v' && input[2]==' ') {
        char *argus = &(input[3]);
        int len1 = -1, len2 = -1;
        int i = 0;
        while (argus[i] != ' ' && argus[i] != '\0') {
            len1++;
            i++;
        }
        len1++;
        char *name1 = new char[len1+1];
        for (int j=0; j<len1; j++) {
            name1[j] = argus[j];
        }
        name1[len1] = '\0';
        argus = &(input[3+len1+1]);
        i = 0;
        while (argus[i] != ' ' && argus[i] != '\0') {
            // cout << argus[i];
            len2++;
            i++;
        }
        len2++;
        char *name2 = new char[len1+1];
        for (int j=0; j<len1; j++) {
            name2[j] = argus[j];
        }
        name2[len2] = '\0';
        cout << "mv " << name1 << " " << name2 << endl;
    } else if (input[0]=='r' && input[1]=='m' && input[2]==' ') {
        char *fileName = &(input[3]);
        int id = kernel->kft->IsOpen(fileName);
        // if file current is opened, mark as delete
        if (id > -1) {
            kernel->kft->Remove(id);
        } else {
            kernel->fileSystem->Remove(fileName);
        }
    } else if (input[0]=='r' && input[1]=='m' && input[2]=='d' 
            && input[3]=='i' && input[4]=='r' && input[5]==' ') {
        char *dirName = &(input[6]);
        cout << "rmdir " << dirName << endl;
    } else if (input[0]=='c' && input[1]=='h' && input[2]=='m' 
            && input[3]=='o' && input[4]=='d' && input[5]==' ') {
        char *mode = &(input[6]);
        cout << "chmod " << mode << endl;
    } else {
        // run user program
        Thread *t = new Thread(input);
        t->Fork((VoidFunctionPtr) executeProgram, input); 
        t->parentT = kernel->currentThread;
        kernel->currentThread->children->Append(t);
        return t->tid;
    }
    
    return kernel->currentThread->tid;
}


#endif /* ! __USERPROG_KSYSCALL_H__ */
