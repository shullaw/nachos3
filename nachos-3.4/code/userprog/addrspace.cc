// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------
// taslk
static void
SwapHeader(NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

/* ---------------------NOTES/NOT IMPLEMENTED---------------------------------- */ // SHULLAW
//  Modify the AddrSpace constructor so that pageTable is not allocated physical pages starting from 0.
//  Use a bitmap to keep track of which pages are allocated and check if the page is allocated before allocating it.
//  Assign the physical page number to the pageTable entry and set the pageTable entry to be valid.
//  Also, set the pageTable entry to be read-only.
// The pageTable is a two-dimensional array of PageInfo structs.
// The first dimension is the page number.
// The second dimension is the offset within the page.
// The PageInfo struct contains the virtual address, the physical address, and the read/write status.
// The virtual address is the address of the pageTable entry.
// The physical address is the address of the page in physical memory.
// The read/write status is whether the page is read-only or read-write.
// Once you know which physical page to allocate, zero out the page in physical memory.
// Change the pageTable entry to be valid.
/* ------------------------------------------------------- */

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
/* ------------------SHULLAW-------------------------------- */
//	"executable" is the file containing the object code to load into memory
//  Reach here from two places:
//  1.) user program that runs using -x argument and handled in progtest.cc
//  2.) user exceptions that occur when the main user program calls Exec() to run other user programs
//  Need to handle:
//  1.) Remove all ASSERTs in this file and surround them with if-else statements are print out the error message
//  2.) After size of executable is determined, allocate the physical memory for the program and handle swap files (TASK 4)
//  Create file name based on threadID
//  Base code sets threadID to 1 as default and incremented in SC_EXEC after setting up AddrSpace
//  Update AddrSpace to AddrSpace(OpenFile *executable, int threadID)
//----------------------------------------------------------------------
// AddrSpace::AddrSpace()
// {
//     pageTable = NULL;
// }

AddrSpace::AddrSpace(OpenFile *executable, int threadID) /* ------------------SHULLAW-------------------------------- */
{
    NoffHeader noffH;
    unsigned int i, size;
    int tid, openFrame, startFrame;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    if (noffH.noffMagic != NOFFMAGIC)
    {
        printf("Not a noff file: %d\n", noffH.noffMagic);
        Exit(-1);
    }

    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size; // we need to increase the size
                                                                          // to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
    // machine->DumpState();  // Machine registers

    // ASSERT(numPages <= NumPhysPages);		// check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory
    printf("AddrSpace: Initializing address space, num pages %d, size %d\n",
           numPages, size);
    printf("AddrSpace: Number of pages: %d\n", numPages);
    printf("AddrSpace: Number of physical pages: %d\n", NumPhysPages);
    printf("AddrSpace: threadID: %d\n", threadID);

    if (numPages > NumPhysPages)
    {
        tid = -1 * (threadID + 1);
        printf("AddrSpace: Initialization failed (numPages > NumPhysPages).\n");
        printf("AddrSpace: Error code: %d\n", tid);
        // for now, must quit the program if it does not fit into memory
        if (executable)
            delete executable; // StartProcess() is not able to run the program
        printf("Exit(tid): %d\n", tid);
        Exit(tid);
            }

    // first, set up the translation
    if (numPages <= NumPhysPages)
    {
        pageTable = new TranslationEntry[numPages];
        for (i = 0; i < numPages; i++)
        {
            printf("Bitmap BEFORE Find(): ");
            bitMap->Print();
            openFrame = bitMap->Find();  // keeping track for offset
            printf("Bitmap AFTER Find(): ");
            bitMap->Print();
            if (openFrame != -1 && i == 0)
            {
                startFrame = openFrame;  // for offset
            }
            /* ------------------SHULLAW-------------------------------- */
            pageTable[i].virtualPage = i; // for now, virtual page # = phys page #
            // pageTable[i].physicalPage = i;  // not necessary, since we set valid bit to false
            if (openFrame == -1)   // there is no open frame, for now there should be
                                   // because we're not going to load in the page if there isn't
                                   // enough room for ALL of the pages
            {
                //-------for now, this should not execute-------//
                tid = -1 * (threadID + 1);
                printf("AddrSpace: Initialization failed (openFrame == -1).\n");
                printf("AddrSpace: Error code: %d\n", tid);
                // for now, must quit the program if it does not fit into memory
                if (executable)
                    delete executable; // StartProcess() is not able to run the program
                // printf("Exit(tid): %d\n", tid);
                // Exit(tid);
                //-------for now, this should not execute-------//
            }
            else if (openFrame != i)  // contiguous
            {
                bitMap->Clear(openFrame);
                printf("AddrSpace: Cleared Frame %d.\n", i);
            }
                pageTable[i].physicalPage = i; // set the open frame

                //set true for task 2, bitmap, set offset for mainmem readat
                pageTable[i].valid = TRUE; // set this valid bit to false to cause pageFaultException and handle in exception.cc
                // AGAIN, pageTable[i].valud is only TRUE for TASK 2 according to Taylor // *SHULLAW*------//
                pageTable[i].use = FALSE; // handle page loading later during page fault
                pageTable[i].dirty = FALSE;
                pageTable[i].readOnly = FALSE; // if the code segment was entirely on
                                               // a separate page, we could set its
                                               // pages to be read-only
        }
        printf("AddrSpace: Initialization complete (made pageTable).\n");
        // to zero the unitialized data segment
        // and the stack segment
        // bzero(machine->mainMemory, size);  // needs to be changed to only zero out the pages being currently allocated
        memset(machine->mainMemory, 0, size); // replaces the first "size" addresses of mainMemory with 0
        // then, copy in the code and data segments into memory
        if (noffH.code.size > 0)
        {
            DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
                  noffH.code.virtualAddr, noffH.code.size);
            executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
                               noffH.code.size, noffH.code.inFileAddr);
        }
        if (noffH.initData.size > 0)
        {
            DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
                  noffH.initData.virtualAddr, noffH.initData.size);
            executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
                               noffH.initData.size, noffH.initData.inFileAddr);
        }
        /* ------------------SHULLAW-------------------------------- */
        // Create swapfile
        // Create file name based on threadID
        char swapFileName[20]; // swapfile = 8 + 100000 threads = 5 + 1 = 14...so uhh just being safe with 20
        // Potentially waste .6 MB of space, change later maybe
        // Exit(100);  // for testing
        sprintf(swapFileName, "swapfile%d", threadID);
        printf("SWIZZLE FIZZLE: %s\n", swapFileName);
        fileSystem->Create(swapFileName, UserStackSize); // create swap file and allocate space for it
        fileSystem->Open(swapFileName);                  // open swap file
        // Create a buffer (temporary array of characters) of size equal to noffH.code.size + noffH.initData.size + noffH.uninitData.size
        int sizeOfBuffer = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
        char *buffer = new char[sizeOfBuffer];
        // Copy the code segment into the buffer executable->ReadAt(buffer, sizeOfBuffer, 0);
        executable->ReadAt(buffer, sizeOfBuffer, 0);
        // Delete pointer to buffer and swap files so that program does not consume memory
        delete[] buffer;
        // delete executable;
        // Handle page table
    }
    /* ------------------SHULLAW-------------------------------- */
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    /* ------------------SHULLAW-------------------------------- */
    //    delete pageTable;  // old version, only for one process running at a time
    /* ------------------SHULLAW-------------------------------- */
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

unsigned int AddrSpace::getNumPages()
{
    return numPages;
}