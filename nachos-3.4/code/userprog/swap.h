#ifndef SWAP_H
#define SWAP_H
#include "system.h"
#include "ProcessTable.h"
#include "copyright.h"
#include "filesys.h"
#include <strings.h>

class Swap{
public:
    OpenFile** swapFile;
    int ** entry;
    int * sizes;
    Swap(int numpros)
    {
        numpros++;
        swapFile = new OpenFile*[numpros];
        entry = new int*[numpros];
        sizes = new int[numpros];

        for( int i=0;i<numpros;i++)
        {
            swapFile[i]=NULL;
            entry[i]=NULL;
            sizes[i]= -1;
        }
    }
    ~Swap()
    {
        delete swapFile;
        delete entry;
    }

    void StoreIntoSwap(int vpn,int prosid,int pysical)
    {
        if(swapFile[prosid]==NULL)
        {
            printf("OPENING NEW SWAP FILE \n");
            Thread * thread = (Thread *)processtable->Get(prosid);
            int tsize= thread->space->numPages;
            entry[prosid]= new int[tsize];
            for( int n=0; n<tsize;n++) entry[prosid][n]=0;
            sizes[prosid]=tsize;
            char ss[20];
            // ss << prosid;
            // std :: string s = ss.str();
            sprintf(ss,"%d",prosid);
            fileSystem->Create(ss, tsize*PageSize+8);
            swapFile[prosid] = fileSystem->Open(ss);
            printf("DONE OPENING \n");



        }
        swapFile[prosid]->WriteAt(machine->mainMemory + pysical * PageSize, PageSize, vpn * PageSize);
        entry[prosid][vpn]=1;
    }

    bool ispresent(int vpn, int prosid)
    {
        if(swapFile[prosid]==NULL) return false;
        //printf("swap status\n");
        //for( int n=0;n<sizes[prosid];n++) printf("%d ->",entry[prosid][n] );
        //printf("\n");
        if( entry[prosid][vpn]==0){

            return false;
        } 
        else return true;
    }

    void loadtomemory(int vpn, int prosid, int physical)
    {
        bzero( machine->mainMemory + physical * PageSize, PageSize );
        swapFile[prosid]->ReadAt(machine->mainMemory + physical * PageSize, PageSize, vpn * PageSize);
    }

    void removeswap(int prosid)
    {
        if(swapFile[prosid]==NULL) return;
        delete swapFile[prosid];
        delete entry[prosid];
        swapFile[prosid]=NULL;
        char ss[20];
        // ss << prosid;
        sprintf(ss, "ss%d", prosid);
        // std::string s = ss.str();
        fileSystem->Remove(ss);
    

    //     //---------------------------//
    //       // Create swapfile
    // // Create file name based on threadID
    // char swapFileName[20]; // swapfile = 8 + 100000 threads = 5 + 1 = 14...so uhh just being safe with 20
    // // Potentially waste .6 MB of space, change later maybe
    // // Exit(100);  // for testing
    // sprintf(swapFileName, "swapfile%d", threadID);
    // printf("SWIZZLE FIZZLE: %s\n", swapFileName);
    // fileSystem->Create(swapFileName, UserStackSize); // create swap file and allocate space for it
    // fileSystem->Open(swapFileName);                  // open swap file
    // // Create a buffer (temporary array of characters) of size equal to noffH.code.size + noffH.initData.size + noffH.uninitData.size
    // int sizeOfBuffer = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
    // char *buffer = new char[sizeOfBuffer];
    // // Copy the code segment into the buffer executable->ReadAt(buffer, sizeOfBuffer, 0);
    // //executable->ReadAt(buffer, sizeOfBuffer, 0); -----Ryan--------
    // // Delete pointer to buffer and swap files so that program does not consume memory
    // delete[] buffer;

    }
};

extern Swap* swapspace;
#endif // SWAP_H