#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "Sys.h"
#include "MIDITOR.h"

MIDITOR Mid((1<<20));
void MidiMIDITOR(const SysC8 *s,const SysC8 *f)
{
    MIDITOR &m=Mid;
    Mid.Bars(s);
    m.Sort();
    m.Render(f);
}

void Preprocess(const SysC8 *s)
{
    SysC8 *b=new SysC8[Mid.MaxWorkMemory];
    Mid.PreProcess(s,strlen(s),b,Mid.MaxWorkMemory);
    SysODS("%s",b);
    delete [] b;
}

//#define DEBUGARGS
SysS32 main(SysS32 argc,SysC8** argv)
{
    SysODS("MIDITOR Copyright G.Symons 2012\n");
#ifndef DEBUGARGS
    if(argc!=3)
    {
        SysODS("Usage: MIDITOR FileName.MTR FileName.MID \n");
        SysODS("       MIDITOR FileName.MTR -D   Displays Preprocessed File\n");
        exit(1);
    }
    const SysC8 *a[2]= {argv[1],argv[2]};
#else
    const SysC8 *a[2]= {"MIDITOR.txt","Test.MID"};
#endif
    SysS32 l=SysLoad(0,a[0],0,0,0);
    SysC8 *m=new SysC8 [l+1];
    SysLoad(0,a[0],0,l,m);
    m[l]=0;
    SysOpen();
    if(argv[2][0]=='-')
        Preprocess(m);
    else
        MidiMIDITOR(m,a[1]);
    SysClose();
    delete [] m;;
    return 0;
}


