#include <stdio.h>
#include <stdarg.h>
#include "Sys.h"

void SysOpen(void)
{
}
void SysClose(void)
{
}

SysS32  SysLoad(SysU32 Flags,const SysC8 *FileName,SysU32 Offset,SysU32 Size,void *Buffer)
{
    FILE *f;
    SysS32 l;

    f=fopen(FileName,"rb");
    if(f==NULL)
    {
        return 0;
    }
    if(Size && Buffer)
    {
        fseek(f,Offset,SEEK_SET);
        l=fread(Buffer,1,Size,f);
    }
    else
    {
        fseek(f,0,SEEK_END);
        l=ftell(f);
    }
    fclose(f);
    return l;
}

SysS32 SysSave(SysU32 Flags,const SysC8 *FileName,SysU32 Size,void *Buffer)
{
    FILE *f=fopen(FileName,"wb");
    if(Size) fwrite(Buffer,sizeof(char),Size,f);
    fclose(f);
    return 0;
}

#ifdef SYS_DEBUG_ODS
void SysODS(const SysC8 *DebugString,...)
{
    va_list args;
    va_start (args, DebugString);
    vprintf (DebugString, args);
    va_end (args);
}
#endif

