#include "fileTableThread.h"
#include "main.h"


threadFilePair::threadFilePair() {
    name = NULL;
    kid = -1;
    mode = -1;
}

threadFilePair::threadFilePair(char *n, int i, int m) {
    name = n;
    kid = i;
    mode = m;
}

threadFilePair::~threadFilePair() {
}

// the first and second slot always fro Console in and out
threadFileTable::threadFileTable() {
    ft = new threadFilePair*[10]; // max 10 openfile
    ft[0] = new threadFilePair("ConsoleInput", 0, 2);
    ft[1] = new threadFilePair("ConsoleOutput", 1, 2);
}

threadFileTable::~threadFileTable() {
    delete ft;
}

// first time open file, add both to kernel and thread file tables 
int threadFileTable::Add(OpenFile *file, char* name, int mode) {
    int kid = kernel->kft->Open(file, name);
    if (kid > -1) {
        // add to thread file table
        for (int i=2; i<10; i++) {
            if (!ft[i]) {
                ft[i] = new threadFilePair(name, kid, mode);
                return i;
            }
        }
        // no space for current thread
        return -1;
    }
    return -1;
}

// file opened in kernel and add to current thread
int threadFileTable::Add(int id, char* name, int mode) {
    for (int i=2; i<10; i++) {
        if (!ft[i]) {
            ft[i] = new threadFilePair(name, id, mode);
            return i;
        }
    }
    // no space for current thread
    return -1;
}

// check whether file is opened in current thread
int threadFileTable::IsOpen(char* name) {
    for (int i=2; i<10; i++) {
        // find openfileID
        if (ft[i] && strcmp(name, ft[i]->name) == 0) {
            return i;
        }
    }
    return -1;
}

// get file based on request type. request=0 is Read Only; request=1 is Write
OpenFile * threadFileTable::getFile(int id, int request) {
    if (!ft[id]) return NULL; // not opened
    if (request==0 && !(ft[id]->mode==1 || ft[id]->mode==2)) {
        return NULL;
    }
    if (request==1 && !(ft[id]->mode==2 || ft[id]->mode==3)) {
        return NULL;
    }
    return kernel->kft->getFile(id);
}

void threadFileTable::SetMode(int id, int mode) {
    ft[id]->mode = mode;
}

// erase file in thread level and call kernel level close()
int threadFileTable::Close(int id) {
    if (id < 0 || id > 10) {
        return -1;
    }
    int result = kernel->kft->Close(id);
    ft[id] = NULL;
    return result;
}

int threadFileTable::Remove(char* name) {
    for (int i=2; i<10; i++) {
        // find openfileID
        if (ft[i] && strcmp(name, ft[i]->name) == 0) {
            return kernel->kft->Remove(ft[i]->kid);
        }
    }
    // file not opend in current thread
    return -1;
}


void threadFileTable::Print() {
    for (int i=0; i<10; i++) {
        if (ft[i]) {
            cout << "at " << i << ", name: " << ft[i]->name << endl;
        }
    }
}