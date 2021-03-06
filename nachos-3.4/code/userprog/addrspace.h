// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "noff.h"  // for pageSize


// for uniprogramming
#define UserStackSize		1024

// for multiprogramming
// #define UserStackNumPage 64  // increase this as necessary!
// #define UserStackSize		(UserStackNumPage * PageSize)  // multiple of pageSize

// #define MaxUserThreads      16  // UserThreads * UserStackNumPage = UserStackSize

class AddrSpace {
  public:

    AddrSpace(OpenFile *executable, int threadID);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
          // before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    void loadFromFile(int vpn, int physAddr);

    OpenFile *executable;
    char *swapFileName;
    NoffHeader noffH;
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    int actualSize, uniqid, openFrame;
    bool *valid;

};

#endif // ADDRSPACE_H
