#ifdef CODE32
__asm__(".code32 \n\t");
#include "IOProgramer.h"
#include "libx2.h"
#include "def.h"

#ifdef __I_REALLY_NEED_THIS
const unsigned int IO_8259A::ICW1_EDGE_MODE_OFF=0b0,
                   IO_8259A::ICW1_ELEC_MODE_ON=0b1000,
                    IO_8259A::ICW1_SINGLE_CHIP_ON=0b10,
                    IO_8259A::ICW1_MULTI_CHIPS_OFF=0b0,
                    IO_8259A::ICW1_REQUIRE_ICW4_ON=0b1,
                    IO_8259A::ICW1_NO_ICW4_OFF=0b0,
                    IO_8259A::ICW1_BASE_ON = 0b0001000,
                    IO_8259A::ICW1_BASE_OFF = 0b00011011;
                    
const unsigned int 	IO_8259A::ICW4_SPECIAL_COMPLETE_NEST_ON=0b00010000,
                    IO_8259A::ICW4_COMMON_COMPLETE_NEST_OFF=0b0,
                    IO_8259A::ICW4_BUFFER_ON=0b00001000,
                    IO_8259A::ICW4_NONBUFFER_OFF=0b0,
                    IO_8259A::ICW4_AUTO_END_ON=0b00000010,
                    IO_8259A::ICW4_EOI_OFF=0b0,
                    IO_8259A::ICW4_FOR_80x86_ON=0b00000001,
                    IO_8259A::ICW4_FOR_MCS_OFF=0b0
                    IO_8259A::ICW4_BASE_ON=0b0,
                    IO_8259A::ICW4_BASE_OFF=0b00011111;

const unsigned int 	IO_8259A::OCW2_EOI_COMPLETE_NEST=0x20,
                    IO_8259A::OCW2_BASE_ON=0b0,
                    IO_8259A::OCW2_BASE_OFF=0b11100111;
#endif

const int IO_8259A::PORTS[4]={0x20,0xA0,0x21,0xA1};


IO_8259A::IO_8259A()
{

}

IO_8259A::~IO_8259A()
{

}
void IO_8259A::sendICW1(int chip,int electTriggerMode,int singleChip,int requireICW4)
{
    
    Util::insertMark(0x7788991);
    __asm__ __volatile__(
    "outb %%al,%%dx \n\t"
    :
    :"a"(0x10 | 
        ((electTriggerMode & 0x1) << 3) |
        ((singleChip & 0x1) << 1) |
        ((requireICW4 & 0x1))),
     "d"(IO_8259A::PORTS[chip])
    :
    );
}
void IO_8259A::sendICW2(int chip,int base)
{
    
    Util::insertMark(0x7788992);
        __asm__ __volatile__(
    "outb %%al,%%dx \n\t"
    :
    :"a"(base),
     "d"(IO_8259A::PORTS[chip+2])
    :
    );
}
void IO_8259A::sendICW3(int chip,int linkage)
{
    
    Util::insertMark(0x7788993);
    __asm__ __volatile__(
    "outb %%al,%%dx \n\t"
    :
    :"a"(linkage),
     "d"(IO_8259A::PORTS[chip+2])
    :
    );    
}
void IO_8259A::sendICW4(int chip,int specCompNest,int buf,int autoEndEOI,int useFor80x86)
{
    
    Util::insertMark(0x7788994);
    __asm__ __volatile__(
    "outb %%al,%%dx \n\t"
    :
    :"a"(((specCompNest & 0x1) << 4) |
        ((buf & 0x1) << 3) |
        ((autoEndEOI & 0x1) << 1)|
        ((useFor80x86 & 0x1))),
     "d"(IO_8259A::PORTS[chip+2])
    :
    );   
}
void IO_8259A::sendOCW1(int chip,int mask)
{
    __asm__ __volatile__(
    "outb %%al,%%dx \n\t"
    :
    :"a"(mask),
     "d"(IO_8259A::PORTS[chip+2])
    :
    );    
}
void IO_8259A::sendOCW2(int chip,int eoi)
{
    Util::insertMark(0x7788997);
    __asm__ __volatile__(
    "outb %%al,%%dx \n\t"
    :
    :"a"(eoi & 0b11100111),//D3,D4恒为0
     "d"(IO_8259A::PORTS[chip])
    :
    );    
}
const int IO_8253::PORTS[4]={0x40,0x41,0x42,0x43};
const unsigned int IO_8253::MAXNUM=1193180; 
const unsigned int IO_8253::MAX_MICRO=3599;
IO_8253::IO_8253()
{
    
    
}
IO_8253::~IO_8253()
{
    
    
}    
void IO_8253::setTimeMill(int tunel,unsigned int millsecs)
{
    int count = IO_8253::MAXNUM/1000 * millsecs;
    //int count=11930;-->10ms
    this->sendControlByte(0,0b11,0x3,0);
    Util::outb(IO_8253::PORTS[tunel],count);
    Util::outb(IO_8253::PORTS[tunel],count >> 8);
}
void IO_8253::setTimeMicro(int tunel,unsigned int microsecs)
{
    if(microsecs > IO_8253::MAX_MICRO)
    {
        microsecs = IO_8253::MAX_MICRO;
    }
    unsigned int count = IO_8253::MAXNUM * microsecs / 1000000;
    this->sendControlByte(0,0b11,0x3,0);
    Util::outb(IO_8253::PORTS[tunel],count);
    Util::outb(IO_8253::PORTS[tunel],count >> 8);
 }
void IO_8253::sendControlByte(int tunel,int writeOrder,int workingMode,int BCDMode)
{
    Util::outb(IO_8253::PORTS[3],
                    ((tunel & 0b11) << 6) |
                    ((writeOrder & 0b11) << 4) |
                    ((workingMode & 0b111) << 1) |
                    (BCDMode & 0b1));
}

//=========class: Keyboard
const int Keyboard::PORT_DATA=0x60,
            Keyboard::PORT_CONTROL=0x64,
            Keyboard::PORT_PPI=0x61;
const int Keyboard::NO_DATA_ERROR=0x10000;

const char* Keyboard::KEY_MAP_STD[]={
    "UNUSED","ESC","1","2","3","4","5","6","7","8","9","0","_","=","BS","TAB","q","w","e","r","t",
    "y","u","i","o","p","[","]","Enter","CNTL","a","s","d","f","g","h","j","k","l",";","'","`","LSHFT",
    "\\","z","x","c","v","b","n","m",",",".","/","RSHFT","*","ALT"," ","CAP","F1","F2","F3","F4","F5","F6",
    "F7","F8","F9","F10","NUML","CtrlBreak","Home","Up","PgUp","-","Left","Center","Right","+","End","Down","PgDn","Ins","Del","Unknown","Unknown","Unknown","F11","F12","Unkown","Unknown","Windows","Unknown","Menu"
};
//e0 x e0 x -- up down left right  _  表示需要接受两个e0
//长按无用
const int Keyboard::KEY_MAP_STD_LEN=sizeof(Keyboard::KEY_MAP_STD)/sizeof(char*);
Keyboard::Keyboard()
{
    
    
}

Keyboard::~Keyboard(){
    
}
void Keyboard::waitToWrite()
{
    while(this->isBusy());
}
const char* Keyboard::getAsciiChar(unsigned char code)
{
    if(code<=Keyboard::KEY_MAP_STD_LEN)
        return Keyboard::KEY_MAP_STD[code];
    else
        return "Exceed.";
}

#endif