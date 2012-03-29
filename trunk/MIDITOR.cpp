//MIDITOR Copyright G.Symons 2012

#include "MIDITOR.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const SysF32 Bias=0.0001f;

MIDITOR::MIDITOR(SysU32 MaxE,SysU32 MaxEventDataLen)
{
    MaxEvents=MaxE;
    MaxData=MaxEvents*MaxEventDataLen;
    Event=new MIDITOREVENT[MaxData];
    Data=new SysU8[MaxData];
    Flags=0;
    EventIndex=0;
    DataIndex=0;
    MacroIndex=0;
    SavedTimeIndex=0;
    SavedTime[SavedTimeIndex]=0;
    RLevel=0;
    Channel=0;
    Key=69;
    Time=0;
    BarTime=1;
    for(SysU32 i=0; i<MaxRemaps; i++) Remap[i]=i;
    VelocityDown=VelocityUp=127;
    NoteCallBack=DefaultNoteCallBack;
    IsNoteCallBack=DefaultIsNoteCallBack;
}
MIDITOR::~MIDITOR()
{
    delete [] Data;
    delete [] Event;
}
void MIDITOR::Add(const SysU8 *s,SysU32 l,SysF32 t)
{
    if(t<0) t=Time;
    memcpy(&Data[DataIndex],s,l);
    Event[EventIndex].Time=t;
    Event[EventIndex].DataIndex=DataIndex;
    Event[EventIndex].DataLen=l;
    EventIndex++;
    DataIndex+=l;
    SysAssert(EventIndex<MaxEvents);
    SysAssert(DataIndex<MaxData);
}
void MIDITOR::Add3(SysU8 a,SysU8 b,SysU8 c,SysF32 t)
{
    if(t<0) t=Time;
    SysU8 d[3]= {a,b,c};
    Add(d,3,t);
}
void MIDITOR::Add2(SysU8 a,SysU8 b,SysF32 t)
{
    if(t<0) t=Time;
    SysU8 d[2]= {a,b};
    Add(d,2,t);
}
static const SysU8 MIDINoteOff=0x80;
void MIDITOR::NoteOff(SysU8 n,SysU8 v,SysF32 t)
{
    if(t<0) t=Time;
    Add3(MIDINoteOff|Channel,n,v,t);
}
static const SysU8 MIDINoteOn=0x90;
void MIDITOR::NoteOn(SysU8 n,SysU8 v,SysF32 t)
{
    if(t<0) t=Time;
    Add3(MIDINoteOn|Channel,n,v,t);
}
static const SysU8 MIDIPolyAfterTouch=0xa0;
void MIDITOR::PolyAfterTouch(SysU8 n,SysU8 p,SysF32 t)
{
    if(t<0) t=Time-Bias;
    Add3(MIDIPolyAfterTouch|Channel,n,p,t);
}
static const SysU8 MIDICtrlModeChange=0xb0;
void MIDITOR::CtrlModeChange(SysU8 c,SysU8 v,SysF32 t)
{
    if(t<0) t=Time-Bias;
    Add3(MIDICtrlModeChange|Channel,c,v,t);
}
static const SysU8 MIDIProgChange=0xc0;
void MIDITOR::ProgChange(SysU8 p,SysF32 t)
{
    if(t<0) t=Time-Bias;
    Add2(MIDIProgChange|Channel,p,t);
}
static const SysU8 MIDIChannelAfterTouch=0xd0;
void MIDITOR::ChannelAfterTouch(SysU8 p,SysF32 t)
{
    if(t<0) t=Time-Bias;
    Add2(MIDIChannelAfterTouch|Channel,p,t);
}
static const SysU8 MIDIPitchWheel=0xe0;
void MIDITOR::PitchWheel(SysU8 m,SysU8 l,SysF32 t)
{
    if(t<0) t=Time;
    Add3(MIDIPitchWheel|Channel,l,m,t);
}
void MIDITOR::Sort(void)
{
    SysU32 s=1;
    while(s)
    {
        s=0;
        for(SysS32 i=0; i<(EventIndex-1); i++)
        {
            if(Event[i].Time>Event[i+1].Time)
            {
                s=1;
                MIDITOREVENT t=Event[i];
                Event[i]=Event[i+1];
                Event[i+1]=t;
            }
        }
    }
}

void DBGEvent(SysF32 t,SysU8 *m)
{
    const SysC8 *s=0;
    SysU32 n=0;
    switch(m[0]&0xf0)
    {
    case MIDINoteOn:
        s="NoteOn";
        n=m[1];
        break;
    case MIDINoteOff:
        s="NoteOff";
        n=-m[1];
        break;
    case MIDIProgChange:
        s="ProgChg";
        break;
    case MIDIPitchWheel:
        s="PitchWheel";
        break;
    }
    if(n)
        SysODS("%d ",n);
    else
        SysODS("\n%f:%s %02x %02x(%d) %02x(%d)\n",t,s,m[0],m[1],m[1],m[2],m[2]);
}

struct MidHdrS
{
    SysU8 HeaderID[4];
    SysU8 HeaderSize[4];
    SysU8 Type[2],Tracks[2],TimeDivision[2];
    SysU8 TrackID[4];
    SysU8 TrackSize[4];
};

SysU32 MIDITOR::SToTicks(SysF32 t)
{
    return t*MidBPM*MidTicksPerBeat/60.0f;
}

SysF32 MIDITOR::TicksToS(SysU32 t)
{
    return t*60.0f/(MidBPM*MidTicksPerBeat);
}

SysU32 MIDITOR::DeltaTime(SysU8 *m,SysU32 j,SysU32 t)
{
    static SysU32 l=0;
    SysU32 i=0;
    SysU8 b[8];
    l+=t;
    //SysODS("%fS:%d 0x%08x",TicksToS(l),t,t);
    do
    {
        b[i]=t&0x7f;
        t>>=7;
        i++;
        SysAssert(i<4);
    }
    while(t);

    while(i)
    {
        i--;
        if(i) b[i]|=0x80;
        m[j++]=b[i];
        //SysODS(" 0x%02x",b[i]);
    }
    //SysODS("\n");
    return j;
}

void MIDITOR::Render(const SysC8 *MIDIFileName)
{
    SysU8 *wm=new SysU8 [MaxWorkMemory];
    SysU32 j=0;
    MidHdrS h=
    {
        {'M','T','h','d'},
        {0,0,0,6},
        {0,0},
        {0,1},
        {(MidTicksPerBeat>>8)&0xff,MidTicksPerBeat&0xff},
        {'M','T','r','k'},
        {0,0,0,0},
    };
    SysAssert(sizeof(h)==(4*4+3*2));
    memcpy(&wm[j],&h,sizeof(h));
    j+=sizeof(h);
    SysS32 i=0;
    SysU32 l=0;
    while(i<EventIndex)
    {
        //SysODS("%f:   ",Event[i].Time);
        SysU32 d=SToTicks(Event[i].Time)-l;
        SysAssert(d>=0);
        j=DeltaTime(wm,j,d);
        for(SysU32 k=0; k<Event[i].DataLen; k++)
        {
            wm[j]=Data[Event[i].DataIndex+k];
            j++;
        }
        l=SToTicks(Event[i].Time);
        SysAssert(l>=0);
        i++;
    }
    wm[j++]=0;
    wm[j++]=0xff;
    wm[j++]=0x2f;
    wm[j++]=0;
    SysU32 s=j-sizeof(h);
    h.TrackSize[0]=(s>>24)&0xff;
    h.TrackSize[1]=(s>>16)&0xff;
    h.TrackSize[2]=(s>>8 )&0xff;
    h.TrackSize[3]=(s>>0 )&0xff;
    memcpy(&wm[0],&h,sizeof(h));
    SysSave(0,MIDIFileName,j,wm);
    delete [] wm;
    SysODS("MIDITOR Rendered MIDI To File:%s %d Bytes Completed\n",MIDIFileName,j);
}

SysS32 MIDITOR::Next(const SysC8 *b,SysS32 i,SysC8 c)
{
    SysS32 n=0;
    while(b[i+n]!=c)
    {
        SysAssert(b[i+n]);//Unexpected end!
        n++;
    }
    return n;
}

SysS32 MIDITOR::NoteValue(SysC8 c)
{
    if((c>='0')&&(c<='9')) return (c-'0');
    switch(c)
    {
    case 'T':
        return 10;
    case 'E':
        return 11;
    case '.':
        return -1;
    case '-':
        return -2;
    }
    SysAssert(0);//Unrecognised note element!
    return 0;
}

SysS32 MIDITOR::IsElement(SysC8 c)
{
    return (IsNoteCallBack(c)|(c=='|'));
}

SysS32 MIDITOR::SkipBracket(const SysC8 *b,SysS32 i)
{
    if(b[i]=='[') return Next(b,i,']');
    else return 0;
}

SysS32 MIDITOR::NextElement(const SysC8 *b,SysS32 i)
{
    SysS32 n=i;
    while(!IsElement(b[i]))
    {
        i+=SkipBracket(b,i);
        if(!b[i]) return i-n;
        i++;
    }
    return i-n;
}

SysS32 MIDITOR::NotesInBar(const SysC8 *b,SysS32 i)
{
    SysS32 n=0;
    i++;
    while((b[i]!='|')&&(b[i]))
    {
        i+=SkipBracket(b,i);
        SysC8 c=toupper(b[i]);
        if(IsNoteCallBack(c)) n++;
        i++;
    }
    return n;
}

SysS32 MIDITOR::HoldsAfterNote(const SysC8 *b,SysS32 i)
{
    SysS32 h=0;
    i++;
    while(b[i])
    {
        i+=NextElement(b,i);
        if(b[i]=='-')
            h++;
        else if(b[i]!='|')
            break;
        i++;

    }
    //SysODS("%d Holds\n",h);
    return h;
}

SysU32 GetHex1(const SysC8 *s)
{
    SysU32 m;
    sscanf(s,"(%x)",&m);
    return m;
}

void GetHex2(const SysC8 *s,SysU8 *a)
{
    SysU32 m[2];
    sscanf(s,"(%x)(%x)",&m[0],&m[1]);
    a[0]=m[0];
    a[1]=m[1];
}

void GetHex3(const SysC8 *s,SysU8 *a)
{
    SysU32 m[3];
    sscanf(s,"(%x)(%x)(%x)",&m[0],&m[1],&m[2]);
    a[0]=m[0];
    a[1]=m[1];
    a[2]=m[2];
}

void GetHex4(const SysC8 *s,SysU8 *a)
{
    SysU32 m[4];
    sscanf(s,"(%x)(%x)(%x)(%x)",&m[0],&m[1],&m[2],&m[3]);
    a[0]=m[0];
    a[1]=m[1];
    a[2]=m[2];
    a[3]=m[3];
}

SysF32 GetArg(const SysC8 *s)
{
    SysF32 r;
    sscanf(s,"%f ",&r);
    return r;
}

SysF32 GetRelOrAbsArg(const SysC8 *s,SysF32 c)
{

    if(s[0]=='+')
        return c+GetArg(&s[1]);
    else if(s[0]=='-')
        return c-GetArg(&s[1]);
    else
        return GetArg(&s[0]);
}

void MIDITOR::BarCommands(const SysC8 *b,SysS32 j)
{
    SysU8 a16[4];

    j++;
    while(b[j]&&(b[j]!='|'))
    {
        if(b[j+1]=='(')
            switch(b[j])
            {
            case 'M':
                if(GetHex1(&b[j+1])==2)
                {
                    GetHex3(&b[j+1],a16);
                    //SysODS("M: %x %x\n",a16[1],a16[2]);
                    Add(&a16[1],2);
                }
                else
                {
                    SysAssert(GetHex1(&b[j+1])==3);//Hex arguments to MIDI message must define either 2 or 3 arguments!
                    GetHex4(&b[j+1],a16);
                    //SysODS("M: %x %x %x\n",a16[1],a16[2],a16[3]);
                    Add(&a16[1],3);
                }
                break;
            case 'T':
                Time=GetRelOrAbsArg(&b[j+2],Time);
                //SysODS("Time:%f\n",Time);
                break;
            case 'D':
                BarTime=GetRelOrAbsArg(&b[j+2],BarTime);
                //SysODS("Bar Time:%f\n",BarTime);
                break;
            case 'K':
                Key=GetRelOrAbsArg(&b[j+2],Key);
                //SysODS("Key:%d\n",Key);
                break;
            case 'C':
                Channel=GetRelOrAbsArg(&b[j+2],Channel);
                //SysODS("Channel:%d\n",Channel);
                break;
            case 'S':
                if(b[j+1]=='+') SavedTimeIndex++;
                SysAssert(SavedTimeIndex<MaxSavedTimes);
                SavedTime[SavedTimeIndex]=Time;
                break;
            case 'R':
                Time=SavedTime[SavedTimeIndex];
                if(b[j+1]=='-') SavedTimeIndex--;
                SysAssert(SavedTimeIndex>=0);
                break;
            }
        j++;
    }
}

void MIDITOR::Bars(const SysC8 *ob,SysS32 r)
{
    SysC8 *b=new SysC8[MaxWorkMemory];
    SysS32 bl=PreProcess(ob,strlen(ob),b,MaxWorkMemory);
    SysAssert(bl>=0);
    while(r)
    {
        SysAssert(Channel<16);//Only 16 MIDI channels!
        SysS32 l=strlen(b);
        SysS32 i=0;
        NoteTime=1;

        while(b[i])
        {
            while(b[i]<=' ') i++;
            if(!IsElement(b[i])) SysODS("Not recognised [%d:%c %d 0x%x]\n",i,b[i],b[i],b[i]);
            SysAssert(IsElement(b[i]));//Unrecognised note element!
            if(IsNoteCallBack(b[i]))
            {
                //SysODS("%d:Note:%c %f %f\n",i,b[i],Time,NoteTime);
                NoteCallBack(this,b,i);
                Time+=NoteTime;
            }
            else
            {
                //SysODS("%d:|:%c %f %f\n",i,b[i],Time,NoteTime);
                SysAssert(b[i]=='|');//Expecting bar!
                BarCommands(b,i);
                SysS32 n=NotesInBar(b,i);
                if(n) NoteTime=BarTime/n;
            }
            SysAssert(i<l);//Out of range!
            i++;
            i+=NextElement(b,i);
        }
        r--;
    }
    delete [] b;
}

SysS32 MIDITOR::DefaultIsNoteCallBack(SysC8 c)
{
    c=toupper(c);
    return (((c>='0')&&(c<='9'))||(c=='T')||(c=='E')||(c=='.')||(c=='-'));
}

SysS32 HexVal(SysC8 a)
{
    SysS32 a16=0;
    if((a>='0')&&(a<='9')) a16=a-'0';
    if((a>='a')&&(a<='f')) a16=a-'a'+10;
    if((a>='A')&&(a<='F')) a16=a-'A'+10;
    return a16;
}

void MIDITOR::DefaultNoteCallBack(MIDITOR *p,const SysC8 *b,SysS32 i)
{
#define SetPitchWheel(O,T) p->PitchWheel((((SysU32)(O))>>7)&((1<<7)-1),((SysU32)(O))&((1<<7)-1),(T))
    SysS32 n=p->NoteValue(b[i]);
    SysS32 NoteEnabled=(n>=0);

    if(!NoteEnabled)
        n=0;
    else
        n+=p->Key;

    SysS32 h=p->HoldsAfterNote(b,i);
    SysS32 l=p->NextElement(b,i+1);
    SysF32 d=p->NoteTime*(1+h)-Bias;
    SysF32 r=0;
    SysF32 a[4];
    SysU8 a16[4];

    for(SysS32 j=(i+1); j<(i+1+l); j++)
    {
        switch(b[j])
        {
        case '\'':
            n+=12;
            break;
        case ',':
            n-=12;
            break;
        case '"':
            n+=24;
            break;
        case ';':
            n-=24;
            break;
        case 'v':
            a[0]=GetArg(&b[j+1]);
            if(a[0]<0)
                p->VelocityDown=a[0];
            else
                p->VelocityUp  =a[0];
            break;
        case 'b':
            a[0]=GetArg(&b[j+1]);
            //SysODS("b %f\n",a[0]);
            for(SysU32 i=1; i<16; i++) SetPitchWheel(0x2000+((i*a[0])/15.0f)*0x1fff,p->Time+(d*i/15.0f));
            SetPitchWheel(0x2000,p->Time+d);
            break;
        case '+':
            a[0]=GetArg(&b[j+1]);
            //SysODS("+ %f\n",a[0]);
            p->NoteOn(n+a[0],p->VelocityDown,p->Time+r+0);
            p->NoteOff(n+a[0],p->VelocityUp,p->Time+r+d);
            break;
        case 'd':
            a[0]=GetArg(&b[j+1]);
            d=(a[0]*p->BarTime);
            break;
        case 'r':
            a[0]=GetArg(&b[j+1]);
            r=(a[0]*p->BarTime);
            break;
        case 'x':
            NoteEnabled=0;
            break;
        case '/':
            a[0]=GetArg(&b[j+1]);
            for(SysU32 i=1; i<a[0]; i++)
            {
                SysF32 t=d/(a[0]-1);
                SysF32 o=(t*i);
                p->NoteOn(n+i,p->VelocityDown,p->Time+r+o);
                p->NoteOff(n+i,p->VelocityUp,p->Time+r+o+t);
            }
            break;
        case '\\':
            a[0]=GetArg(&b[j+1]);
            for(SysU32 i=1; i<a[0]; i++)
            {
                SysF32 t=d/(a[0]-1);
                SysF32 o=(t*i);
                p->NoteOn(n-i,p->VelocityDown,p->Time+r+o);
                p->NoteOff(n-i,p->VelocityUp,p->Time+r+o+t);
            }
            break;
        case 'm':
            if(GetHex1(&b[j+1])==2)
            {
                GetHex3(&b[j+1],a16);
                //SysODS("m %x %x\n",a16[1],a16[2]);
                p->Add(&a16[1],2);
            }
            else
            {
                SysAssert(GetHex1(&b[j+1])==3);//MIDI message within note modifier must have 2 or 3 arguments defined!
                GetHex4(&b[j+1],a16);
                //SysODS("m %x %x %x\n",a16[1],a16[2],a16[3]);
                p->Add(&a16[1],3);
            }
            break;
        }
    }
    if(NoteEnabled)
    {
        p->NoteOn(n,p->VelocityDown,p->Time+r+0);
        p->NoteOff(n,p->VelocityUp,p->Time+r+d);
    }
}

void  MIDITOR::Display(const SysC8 *t,const SysC8 *s,const SysC8 *e)
{
    SysODS("* %s",t);
    while(s<e) SysODS("%c",*s++);
    SysODS(" *\n");
}

SysS32 MIDITOR::MacroNameLen(SysU32 i)
{
    return ((Macro[i][1]-1)-Macro[i][0]);
}

SysS32 MIDITOR::MacroDefLen(SysU32 i)
{
    return (Macro[i][2]-Macro[i][1]);
}

static void Copy3(const SysC8 **d,const SysC8 **s)
{
    d[0]=s[0];
    d[1]=s[1];
    d[2]=s[2];
}

void MIDITOR::MacrosSort(void)
{
    SysF32 f=1;
//    SysODS("\n\nSorting\n");
    while(f)
    {
        f=0;
        for(SysU32 i=0; (i+1)<MacroIndex; i++)
        {
//            SysODS("NL:%d DL:%d ",MacroNameLen(i),MacroDefLen(i));
//            Display("Sorting:",Macro[i][0],&Macro[i][0][16]);
            if(!MacroDefLen(i))
            {
//                Display("Undefined1",Macro[i][0],&Macro[i][0][16]);
                Macro[i][1]=Macro[i][0]+1;
            }
            if(MacroNameLen(i)<MacroNameLen(i+1))
            {
                const SysC8 *t[3];
                Copy3(&t[0],Macro[i]);
                Copy3(Macro[i],Macro[i+1]);
                Copy3(Macro[i+1],t);
                f=1;
            }
        }
    }
    while((MacroIndex>0)&&(!MacroNameLen(MacroIndex-1)))
    {
//        Display("Undefined2",Macro[MacroIndex-1][0],&Macro[MacroIndex-1][0][16]);
        MacroIndex--;
    }
}

SysS32 MIDITOR::MacroSame(const SysC8 *a,const SysC8 *b)
{
    SysAssert(a);
    SysAssert(b);
    SysU32 i=0;
    while(a[i]!=']')
    {
        if(a[i]!=b[i]) return 0;
        i++;
    }
    return 1;
}

SysS32 MIDITOR::MacroFind(const SysC8 *m)
{
    for(SysU32 i=0; i<MacroIndex; i++)
        if(MacroSame(Macro[i][0],m)) return i;
    return -1;
}

SysS32 MIDITOR::EndOfMacro(const SysC8 *b)
{
    SysS32 i=0;
    SysS32 InMBrackets=1;
    while(InMBrackets)
    {
        i++;
        SysAssert(b[i]);
        if(b[i]=='<') InMBrackets++;
        if(b[i]=='>') InMBrackets--;
    }
    return i;
}
void MIDITOR::MacroDefine(const SysC8 *m)
{
    SysU32 i=0;
    Macro[MacroIndex][0]=m;
    while(m[i])
    {
        if(m[i]==']')
        {
            i++;
            Macro[MacroIndex][1]=&m[i];
            i+=EndOfMacro(&m[i-1])-1;
            Macro[MacroIndex][2]=&m[i];
            //Display("Define:",Macro[MacroIndex][0],Macro[MacroIndex][2]);
            SysS32 f=MacroFind(Macro[MacroIndex][0]);
            if(f>=0)
            {
                if(MacroNameLen(MacroIndex)!=MacroNameLen(f)) f=-1;
            }
            if(f>=0)
            {
                Copy3(Macro[f],Macro[MacroIndex]);
                //Display("Overwrite:",Macro[MacroIndex][0],Macro[MacroIndex][2]);
            }
            else
            {
                MacroIndex++;
                SysAssert(MacroIndex<MaxMacros);
            }
            MacrosSort();
            return;
        }
        i++;
    }
    SysAssert(0);//Macro definition error!
}

SysC8 MIDITOR::Remapping(SysC8 c)
{
    if(RLevel!=1) return c;
    SysAssert(c>32);
    SysAssert(c<MaxRemaps);
    //if(c!=Remap[c&0x7f]) SysODS("[Level %d]:Remapped %c %c\n",RLevel,c,Remap[c&0x7f]);
    return Remap[(c&0x7f)];
}

SysS32 MIDITOR::PreProcess(const SysC8 *b,SysU32 bl,SysC8 *wm,SysU32 wl)
{
    RLevel++;
    SysS32 InBrackets=0;
    SysS32 InComments=0;
    SysC8 *WorkMem=wm;
    SysU32 i=0,j=0;
    while(i<bl)
    {
        if(b[i]<=' ')
        {
            i++;
            continue;
        }
        if((b[i]=='<')&&(b[i+1]=='*'))
        {
            InComments++;
            i+=2;
            continue;
        }
        if((b[i]=='*')&&(b[i+1]=='>'))
        {
            InComments--;
            i+=2;
            if(InComments>=0)continue;
        }
        if(InComments)
        {
            if(InComments>0)
            {
                i++;
                continue;
            }
            SysODS("Comments <**> nested error:%d %d\n",i,InComments);
            exit(1);
        }
        if(b[i]=='[') InBrackets++;
        if(b[i]==']') InBrackets--;
        SysAssert(InBrackets>=0);//] specified with no previous [!
        if(!InBrackets)
        {
            if(b[i]=='<')
            {
                if(b[i+1]=='[')
                {
                    //Display("Macro Defining:",&b[i],&b[i+8]);
                    MacroDefine(&b[i+2]);
                }
                else if(b[i+1]==']')
                {
                    SysAssert(b[i+3]=='[');
                    SysAssert(b[i+5]=='>');
                    SysAssert(b[i+3]>32);
                    SysAssert(b[i+3]<MaxRemaps);
                    SysAssert(b[i+5]>32);
                    SysAssert(b[i+5]<MaxRemaps);
                    Remap[b[i+2]&0x7f]=(b[i+4]&0x7f);
                }
                i+=EndOfMacro(&b[i]);
                i++;
                continue;
            }
            SysS32 f=MacroFind(&b[i]);
            //Display("Search:",&b[i],&b[i+1]);
            if(f>=0)
            {
                SysS32 r=1;
                SysS32 o=i;
                if((b[o-1]=='>')&&(b[o-2]=='X'))
                {
                    while(b[o]!='<')
                    {
                        SysAssert(o>=0);//No preceding < for macro multiple!
                        o--;
                    }
                    sscanf(&b[o+1],"%d",&r);
                }
                while(r)
                {
                    const SysC8 *s=Macro[f][1];
                    SysS32 l=PreProcess(s,Macro[f][2]-Macro[f][1],&WorkMem[j],MaxWorkMemory-j);
                    SysAssert(l>=0);
                    while(l)
                    {
                        WorkMem[j]=Remapping(WorkMem[j]);
                        j++;
                        l--;
                    }
                    r--;
                }
                //SysODS("(%d)",RLevel);
                //Display("Found:",Macro[f][0],Macro[f][1]-1);
                i+=MacroNameLen(f);
            }
            else
            {
                WorkMem[j++]=Remapping(b[i++]);
            }
        }
        else
        {
            WorkMem[j++]=Remapping(b[i++]);
        }
        SysAssert(j<wl);
    }
    WorkMem[j]=0;
    //SysODS("[Level %d]: %s[End]\n",RLevel,WorkMem);
    RLevel--;
    return j;
}




