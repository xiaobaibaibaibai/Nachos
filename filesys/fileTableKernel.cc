#include "fileTableKernel.h"
#include "main.h"

kernelFilePair::kernelFilePair() {
    file = NULL;
    id = -1;
    references = 0;
    name = NULL;
    isDeleted = false;
}

kernelFilePair::kernelFilePair(OpenFile *f, int i, char* s) {
    file = f;
    id = i;
    references = 1;
    name = s;
    isDeleted = false;
}

kernelFilePair::~kernelFilePair() {
    delete file;
    id = -1;
    references = 0;
    name = NULL;
}


// first time open file, add both to kernel and thread file tables 
kernelFileTable::kernelFileTable() {
    ft = new kernelFilePair*[10]; // max 10
    ft[0] = new kernelFilePair(NULL, 0, "ConsoleInput");
    ft[1] = new kernelFilePair(NULL, 1, "ConsoleOutput");
}


kernelFileTable::~kernelFileTable() {
    delete[] ft;
}

// check whether file is opened in kernel level
int kernelFileTable::IsOpen(char* name) {
    for (int i=2; i < 10; i++) {
        if (ft[i] && strcmp(name, ft[i]->name) == 0) {
            return ft[i]->id;
        }
    }
    return -1;
}

// first time open, insert into table
int kernelFileTable::Open(OpenFile *file, char* name) {
    for (int i=2; i < 10; i++) {
        if (!ft[i]) {
            ft[i] = new kernelFilePair(file, i, name);
            return i;
        }
    }
    return -1;
}

// reduce references and may delete file
int kernelFileTable::Close(int id) {
    if (!ft[id]) {
        return -1;
    }
    ft[id]->references--;
    // if no threads refered and require to delete
    if (ft[id]->isDeleted && ft[id]->references == 0) {
        return kernel->fileSystem->Remove(ft[id]->name);
    }
    // only close file
    if (ft[id]->references == 0) {
        ft[id] = NULL;
    }
    return 1;
}

// mark as deleted and attemp to delete
int kernelFileTable::Remove(int id) {
    if (!ft[id]) {
        return -1;
    }
    ft[id]->isDeleted = true;
    return Close(id);
}

// if request to delete, return NULL. Otherwise, return that file
OpenFile * kernelFileTable::getFile(int id) {
    if (ft[id]->isDeleted) {
        return NULL;
    }
    return ft[id]->file;
}

bool kernelFileTable::IsDelete(int id) {
    return ft[id]->isDeleted;
}

void kernelFileTable::AddReferences(int id) {
    ft[id]->references++;
}