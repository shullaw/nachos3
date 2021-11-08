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
#include "memoryManager.h"
MemoryManager *memorymanager;
ProcessTable *processtable;
Swap *swapspace;

#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

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
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    if (noffH.noffMagic != NOFFMAGIC)
    {
        printf("Not a noff file: %d\n", noffH.noffMagic);
        return;
    }

    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize; // we need to increase the size
                                                                                          // to leave room for the stack

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    swapFileName = new char[100];
    sprintf(swapFileName, "%d.swap", threadID);
    bool success = fileSystem->Create(swapFileName, size);
    if (!success)
    {
        // handle error with swapfile creation
    }

    OpenFile *swapFile = fileSystem->Open(swapFileName);
    if (swapFile == NULL)
    {
        // handle error withg swapfile opening
    }

    // Copy contents of executable into swapfile.
    char *buffer = new char[size];
    executable->ReadAt(buffer, size, sizeof(noffH));
    swapFile->WriteAt(buffer, size, 0);
    delete[] buffer;
    delete swapFile;

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);

    // first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++)
    {
        pageTable[i].virtualPage = i;
        pageTable[i].valid = FALSE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;
    }
    // ASSERT(numPages <= NumPhysPages);		// check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory
    printf("AddrSpace: Initializing address space, num pages %d, size %d\n",
           numPages, size);
    printf("AddrSpace: Number of pages: %d\n", numPages);
    printf("AddrSpace: Number of physical pages: %d\n", NumPhysPages);
    printf("AddrSpace: threadID: %d\n", threadID);
    if (virtualOption == 0)
    {
        printf("AddrSpace: Paging Option: Demand Paging\n");
    }
    if (virtualOption == 1)
    {
        printf("AddrSpace: Paging Option: FIFO (not implemented!)\n");
    }
    if (virtualOption == 2)
    {
        printf("AddrSpace: Paging Option: Rando (not implemented!)\n");
    }
    printf("#--------------------------------------------------------#\n");

    int numAvailPages = bitMap->NumClear();
    // if (numPages > numAvailPages)
    // {
    //     tid = -1 * (threadID + 1);
    //     printf("AddrSpace: Initialization failed (numPages > NumPhysPages) %d.\n", numAvailPages);
    //     printf("AddrSpace: Error code: %d\n", tid);
    //     // for now, must quit the program if it does not fit into memory
    //     if (executable)
    //         delete executable; // StartProcess() is not able to run the program
    //     printf("Exit(tid): %d\n", tid);
    //     Exit(tid);
    // }

    // // first, set up the translation
    // if (numPages <= numAvailPages) // commented out 11/7, needs to be number of remaining pages
    // {
    //     pageTable = new TranslationEntry[numPages];
    //     for (i = 0; i < numPages; i++)
    //     {
    //         // printf("Bitmap BEFORE Find(): ");
    //         // bitMap->Print();
    //         // openFrame = bitMap->Find(); // keeping track for offset
    //         // printf("Bitmap AFTER Find(): ");
    //         // bitMap->Print();
    //         pageTable[i].virtualPage = i;  // for now, virtual page # = phys page #
    //                                        // pageTable[i].physicalPage = i;  // not necessary, since we set valid bit to false
    //         pageTable[i].physicalPage = i; // to not load into mainMemory, but into swap file
    //         //set true for task 2, bitmap, set offset for mainmem readat
    //         pageTable[i].valid = FALSE; //---Ryan---- set this valid bit to false to cause pageFaultException and handle in exception.cc
    //         // AGAIN, pageTable[i].valud is only TRUE for TASK 2 according to Taylor // *SHULLAW*------//
    //         pageTable[i].use = FALSE; // handle page loading later during page fault
    //         pageTable[i].dirty = FALSE;
    //         pageTable[i].readOnly = FALSE;        // if the code segment was entirely on
    //                                               // a separate page, we could set its
    //                                               // pages to be read-only
    //         memset(machine->mainMemory, 0, size); // replaces the first "size" addresses of mainMemory with 0
    //         // machine->DumpState();
    // }
    // printf("AddrSpace: Initialization complete (made pageTable).\n");
    /* ------------------SHULLAW-------------------------------- */
    // Create swapfile
    // Create file name based on threadID
    // char swapFileName[20]; // swapfile = 8 + 100000 threads = 5 + 1 = 14...so uhh just being safe with 20
    // sprintf(swapFileName, "swapfile%d", threadID);
    // printf("SWIZZLE FIZZLE: %s\n", swapFileName);
    // int sizeOfBuffer = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
    // // Create a buffer (temporary array of characters) of size equal to noffH.code.size + noffH.initData.size + noffH.uninitData.size
    // char *buffer = new char[sizeOfBuffer];
    // fileSystem->Create(swapFileName, sizeOfBuffer); // create swap file and allocate space for it
    // fileSystem->Open(swapFileName);                 // open swap file
    // // Copy the code segment into the buffer executable->ReadAt(buffer, sizeOfBuffer, 0);
    // //executable->ReadAt(buffer, sizeOfBuffer, 0); -----Ryan--------
    // // Delete pointer to buffer and swap files so that program does not consume memory
    // delete[] buffer;
    // // delete executable;
    // // Handle page table
}
/* ------------------SHULLAW-------------------------------- */

void AddrSpace::demandPage(int vpn)
{
    if (pageTable[vpn].valid == TRUE)
    {
        printf("AddrSpace: demandPage: Page %d is already valid.\n", vpn);
        return;
    }
    else
    {
        printf("AddrSpace: demandPage: Bitmap BEFORE Find(): ");
        bitMap->Print();
        openFrame = bitMap->Find(); // keeping track for offset
        printf("AddrSpace: demandPage: Bitmap AFTER Find(): ");
        bitMap->Print();
        if (openFrame == -1)
        {
            printf("No available frames for thread %d!!", currentThread->getID());
            Exit(currentThread->getID()); // for offset
        }
        else
        {
            pageTable[vpn].physicalPage = vpn;
            machine->mainMemory[vpn];
            pageTable[vpn].valid = TRUE;
            this->loadPage(vpn);
        }
    }
}

int AddrSpace::whichSeg(int virtAddr, Segment *segPtr)
{

    if (noffH.code.size > 0)
    {
        if ((virtAddr >= noffH.code.virtualAddr) &&
            (virtAddr < noffH.code.virtualAddr + noffH.code.size))
        {
            (*segPtr) = noffH.code;
            return 0;
        }
    }
    if (noffH.initData.size > 0)
    {
        if ((virtAddr >= noffH.initData.virtualAddr) &&
            (virtAddr < noffH.initData.virtualAddr + noffH.initData.size))
        {
            (*segPtr) = noffH.initData;
            return 1;
        }
    }
    if (noffH.uninitData.size > 0)
    {
        if ((virtAddr >= noffH.uninitData.virtualAddr) &&
            (virtAddr < noffH.uninitData.virtualAddr + noffH.uninitData.size))
        {
            (*segPtr) = noffH.uninitData;
            return 2;
        }
    }
    return 3;
}

int AddrSpace::pageFault(int vpn)
{
    stats->numPageFaults++;
    // pageTable[vpn].physicalPage = mm->AllocPage(this,vpn);
    if (pageTable[vpn].physicalPage == -1)
    {
        printf("Error: run out of physical memory\n");
        //to do://should yield and wait for memory space and try again?
        ASSERT(FALSE); //panic at this time
    }

    if (this->PageIn(&pageTable[vpn]) == -1)
        loadPage(vpn);

    pageTable[vpn].valid = TRUE;
    pageTable[vpn].use = FALSE;
    pageTable[vpn].dirty = FALSE;

    return 0;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    for (int j = 0; j < NumPhysPages; j++)
    {
        bitMap->Clear(j);
    }
    // printf("Page after deallocating\n");
    // bitMap->Print();
    // printf("-------------------------------------------\n");
    if (pageTable != NULL)
    {
        delete pageTable;
        printf("AddrSpace: Deallocated pageTable.\n");
    }
    if (swapFileName)
    {
    // printf("swapfilename: %s\n", swapFileName);
    fileSystem->Remove(swapFileName);
    printf("AddrSpace: Deallocated %s.\n", swapFileName);
    }
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

// int AddrSpace::virtToPhys(int virtAddr)
// {
//     int pagelocation = pageTable[virtAddr / PageSize].physicalPage * PageSize;
//     return pagelocation + (virtAddr % PageSize);
// }
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

// unsigned int AddrSpace::getNumPages()
// {
//     return numPages;
// }

int AddrSpace::loadPage(int vpn)
{
    int readAddr, physAddr, size, segOffs;
    int virtAddr = vpn * PageSize;
    int offs = 0;
    Segment seg;
    bool readFromFile = FALSE;

    pageTable[vpn].readOnly = FALSE;
    do
    {
        physAddr = pageTable[vpn].physicalPage * PageSize + offs;
        switch (whichSeg(virtAddr, &seg))
        {
        case 0: //code
        {
            segOffs = virtAddr - seg.virtualAddr;
            readAddr = segOffs + seg.inFileAddr;
            size = min(PageSize - offs, seg.size - segOffs);
            executable->ReadAt(&(machine->mainMemory[physAddr]), size, readAddr);
            readFromFile = TRUE;
            if (size == PageSize)
            {
                pageTable[vpn].readOnly = TRUE;
            }
            if (vpn == 1)
                ASSERT(machine->mainMemory[physAddr] == 7);
            break;
        }
        case 1: //initData
        {
            segOffs = virtAddr - seg.virtualAddr;
            readAddr = segOffs + seg.inFileAddr;
            size = min(PageSize - offs, seg.size - segOffs);
            executable->ReadAt(&(machine->mainMemory[physAddr]), size, readAddr);
            readFromFile = TRUE;
            break;
        }
        case 2: //uninitData
        {
            size = min(PageSize - offs, seg.size + seg.virtualAddr - virtAddr);
            bzero(&(machine->mainMemory[physAddr]), size);
            break;
        }
        case 3: //stack or others
        {
            bzero(&(machine->mainMemory[physAddr]), PageSize - offs);
            return 0; //don't use break
        }
        }
        offs += size;
        virtAddr += size;
    } while (offs < PageSize);
    if (readFromFile)
        pagesReadIn++;
    return 0;
}

/* Read the virtual page referenced by PTE from the backing store */
int AddrSpace::PageIn(TranslationEntry *pte)
{
    if (valid[pte->virtualPage])
    {
        int offset = pte->virtualPage * PageSize;
        int physAddr = pte->physicalPage * PageSize;

        executable->ReadAt(&machine->mainMemory[physAddr], PageSize, offset);

        // 		char buffer[PageSize];
        // 		bsFile->ReadAt(buffer, PageSize, offset);
        // 		// Write buffer to memory
        // 		for (int i = 0; i < PageSize; i++) {
        // 			machine->mainMemory[physAddr + i] = (char)buffer[i];
        // 		}

        pagesReadIn++;
        return 0;
    }
    else
    {
        return -1;
    }
}

void AddrSpace::loadFromFile(int virtPageNum, int physPageNum)
{
    int numInitPages = divRoundUp((noffH.code.size + noffH.initData.size), PageSize);
    int baseAddress = noffH.code.inFileAddr;
    if (numInitPages < virtPageNum)
    {
        int k = executable->ReadAt(&(machine->mainMemory[physPageNum * PageSize]), PageSize, baseAddress + virtPageNum * PageSize);
    }
}

// void AddrSpace::FreeMem(int vpn)
// {

// }

// int AddrSpace::pageFault(int vpn)
// {
//     stats->numPageFaults++;
//     // pageTable[vpn].physicalPage = mm->AllocPage(this,vpn);
//     if (pageTable[vpn].physicalPage == -1)
//     {
//         printf("Error: run out of physical memory\n");
//         //to do://should yield and wait for memory space and try again?
//         ASSERT(FALSE); //panic at this time
//     }

//     //if(backingStore->PageIn(&pageTable[vpn])==-1)
//     //loadPage(vpn);

//     pageTable[vpn].valid = TRUE;
//     pageTable[vpn].use = FALSE;
//     pageTable[vpn].dirty = FALSE;
//     //pageTable[vpn].readOnly is modified in loadPage()

//     return 0;
// }
