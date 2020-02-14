

#include "filehdr.h"
#include "filehdrIndirect.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

IndirectBlock::IndirectBlock() {
    for (int i=0; i<32; i++) {
        dataSectors[i] = -1;
    }
}


IndirectBlock::~IndirectBlock() {
}

// allocate indrect block contiguous
int IndirectBlock::Allocate(PersistentBitmap *freeMap, int c)
{
    int contiguous = 0;
    for (int i=0; i<32 && contiguous < c; i++) {
        if (dataSectors[i] == -1) {
            dataSectors[i] = freeMap->FindAndSet();
            contiguous++;
        }
    }
    return contiguous;
}

// clean whole sectors
void IndirectBlock::Deallocate(PersistentBitmap *freeMap)
{
    for (int i = 0; i < 32; i++) {
        if ((int) dataSectors[i] > -1) {
            int sector = dataSectors[i];
            ASSERT(freeMap->Test((int) dataSectors[i]));
            freeMap->Clear((int) dataSectors[i]);
        }
    }
}


void
IndirectBlock::FetchFrom(int sector)
{
    kernel->synchDisk->ReadSector(sector, (char *)this);
}


void
IndirectBlock::WriteBack(int sector)
{
    kernel->synchDisk->WriteSector(sector, (char *)this); 
}


int
IndirectBlock::ByteToSector(int offset)
{
    return(dataSectors[offset / SectorSize]);
}

