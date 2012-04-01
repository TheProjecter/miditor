//MIDITOR Copyright G.Symons 2012

#ifndef _MIDITOR_H_GS_2011_
#define _MIDITOR_H_GS_2011_
#include "Sys.h"

struct MIDITOR
{
    SysS32 Channel,Key,VelocityUp,VelocityDown;
    SysF32 Time,BarTime;
    void (*NoteCallBack)(MIDITOR *p,const SysC8 *b,SysS32 i);
    SysS32 (*IsNoteCallBack)(SysC8 c);

    enum {UseInternalTime=-1,MaxWorkMemory=(1<<24),MaxMacros=(1<<20),MaxSavedTimes=(1<<16),MaxRemaps=128,MidBPM=120,MidTicksPerBeat=960};
    void Add(const SysU8 *s,SysU32 l,SysF32 t=UseInternalTime);
    void Add3(SysU8 a,SysU8 b,SysU8 c,SysF32 t=UseInternalTime);
    void Add2(SysU8 a,SysU8 b,SysF32 t=UseInternalTime);
    void Sort(void);
    void Render(const SysC8 *MIDIFileName);
    void Bars(const SysC8 *b,SysS32 r=1);

    void NoteOff(SysU8 n,SysU8 v,SysF32 t=UseInternalTime);
    void NoteOn(SysU8 n,SysU8 v,SysF32 t=UseInternalTime);
    void PolyAfterTouch(SysU8 n,SysU8 p,SysF32 t=UseInternalTime);
    void CtrlModeChange(SysU8 c,SysU8 v,SysF32 t=UseInternalTime);
    void ProgChange(SysU8 p,SysF32 t=UseInternalTime);
    void ChannelAfterTouch(SysU8 p,SysF32 t=UseInternalTime);
    void PitchWheel(SysU8 m,SysU8 l,SysF32 t=UseInternalTime);

    struct MIDITOREVENT
    {
        SysU32 DataIndex,DataLen;
        SysF32 Time;
    } *Event;
    SysU8 *Data;

    SysS32 EventIndex,DataIndex;
    SysS32 MaxEvents,MaxData;
    SysU32 Flags,RLevel;

    MIDITOR(SysU32 MaxEvents,SysU32 MaxEventDataLen=4);
    ~MIDITOR();
    SysF32 NoteTime;
    SysF32 SavedTime[MaxSavedTimes];
    SysS32 SavedTimeIndex;
    SysC8 Remap[MaxRemaps];
    void BarCommands(const SysC8 *b,SysS32 i);
    SysS32 Next(const SysC8 *b,SysS32 i,SysC8 c);
    SysS32 SkipBracket(const SysC8 *b,SysS32 i);
    SysS32 NoteValue(SysC8 c);
    SysS32 IsElement(SysC8 c);
    SysS32 NextElement(const SysC8 *b,SysS32 i);
    SysS32 NotesInBar(const SysC8 *b,SysS32 i);
    SysF32 DurationAfterNote(const SysC8 *b,SysS32 i);
    static void DefaultNoteCallBack(MIDITOR *p,const SysC8 *b,SysS32 i);
    static SysS32 DefaultIsNoteCallBack(SysC8 c);
    const SysC8 *Macro[MaxMacros][3];
    SysU32 MacroIndex;
    SysS32 MacroSame(const SysC8 *a,const SysC8 *b);
    SysS32 MacroFind(const SysC8 *m);
    void MacroDefine(const SysC8 *m);
    SysS32 PreProcess(const SysC8 *b,SysU32 bl,SysC8 *wm,SysU32 wl);
    void Display(const SysC8 *t,const SysC8 *s,const SysC8 *e);
    SysS32 MacroNameLen(SysU32 i);
    SysS32 MacroDefLen(SysU32 i);
    SysS32 EndOfMacro(const SysC8 *b);
    void MacrosSort(void);
    SysU32 SToTicks(SysF32 t);
    SysF32 TicksToS(SysU32 t);
    SysU32 DeltaTime(SysU8 *m,SysU32 j,SysU32 t);
    SysC8 Remapping(SysC8 c);
};

#endif
