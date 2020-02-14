
#ifndef FILEHDRINDIRECT_H
#define FILEHDRINDIRECT_H

#include "disk.h"
#include "pbitmap.h"


class IndirectBlock {
    public:
    IndirectBlock();
    ~IndirectBlock();
    int Allocate(PersistentBitmap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(PersistentBitmap *bitMap);  // De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte
    

  private:
    int dataSectors[32];		// Disk sector numbers for each data 
					// block in the file
};


#endif // FILEHDRINDIRECT_H