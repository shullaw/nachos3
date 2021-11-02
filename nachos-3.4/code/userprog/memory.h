#ifndef MEMORY_H
#define MEMORY_H

#include "bitmap.h"
#include "sync.h"
#include "list.h"
#include "machine.h"
#include "addrspace.h"

#define FIFO 0

class Memory 
{
    public:
            Memory(int numPages, int method);
            ~Memory();

            int Allocate(AddrSpace *space, int pageNumVirt);
            void Deallocate(int pageNumPhys);
            bool IsAllocated(int pageNumPhys);
            void Print() 
            { 
                bitmap->Print(); 
            }
}