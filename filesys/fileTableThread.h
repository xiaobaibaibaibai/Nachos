#ifndef TFILETABLE_H
#define TFILETABLE_H

#include "openfile.h"


class threadFilePair {
    public:
    threadFilePair();
    threadFilePair(char *n, int i, int m);
    ~threadFilePair();
    
    char* name;
    int kid;
    int mode; // RO=1, RW=2, APPEND=3
};


// keep track and manage all OpenFile objects for its owned process
class threadFileTable {
    public:
    threadFileTable();
    ~threadFileTable();
    // first time open file
    int Add(OpenFile *file, char* name, int mode);
    // file opened in kernel so add to current thread
    int Add(int id, char* name, int mode);
    int Close(int id);
    OpenFile * getFile(int id, int request);
    int Remove(char* name);
    int IsOpen(char* name);
    void SetMode(int id, int mode);
    
    void Print();
    
    private:
    threadFilePair **ft;
};


#endif // TFILETABLE_H