#ifndef KFILETABLE_H
#define KFILETABLE_H

#include "openfile.h"

class kernelFilePair {
    public:
    kernelFilePair();
    kernelFilePair(OpenFile *f, int i, char* s);
    ~kernelFilePair();
    
    OpenFile* file;
    int id;
    int references;
    char* name;
    bool isDeleted; // request to delete
};


// keep track and manage all OpenFile objects for every process
class kernelFileTable {
    public:
    kernelFileTable();
    ~kernelFileTable();
    // insert an Openfile into table
    int Open(OpenFile *file, char* name);
    // reduce references. If necessary, delete file.
    int Close(int id); 
    OpenFile * getFile(int id);
    int IsOpen(char* name);
    int Remove(int id);
    bool IsDelete(int id);
    void AddReferences(int id);

    private:
    kernelFilePair **ft;
};

#endif // KFILETABLE_H