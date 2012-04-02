#ifndef _SYS_H_GS_2011_
#define _SYS_H_GS_2011_
typedef char                                    SysC8;
typedef unsigned char                           SysU8;
typedef signed char                             SysS8;
typedef unsigned short                          SysU16;
typedef signed short                            SysS16;
typedef unsigned int                            SysU32;
typedef signed int                              SysS32;
typedef float                                   SysF32;

void SysOpen(void);
void SysClose(void);
SysS32 SysLoad(SysU32 Flags,const SysC8 *FileName,SysU32 Offset,SysU32 Size,void *Buffer);
SysS32 SysSave(SysU32 Flags,const SysC8 *FileName,SysU32 Size,void *Buffer);

#ifndef SYS_DEBUG_ODS
#define SYS_DEBUG_ODS
#endif

#ifdef SYS_DEBUG_ODS
void SysODS(const SysC8 *DebugString,...);
#define SysAssert(exp,msg)  if (!(exp)) {SysODS( "SysAssert:%s\n%s File:%s Base File:%s Line:%d\n",msg,#exp, __FILE__, __BASE_FILE__, __LINE__ );exit(1);}
#else
inline void SysODS(const SysC8 *DebugString,...) {};
#define SysAssert(exp,msg)
#endif

#endif

