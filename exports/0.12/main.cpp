#ifdef CODE16
__asm__(
".text \n\t"
".code16gcc \n\t"
"STARTSEG = 0x7c0 \n\t"
"STACKSIZE = 512*2 \n\t"
"TEMP_SEG = 0xa00 \n\t"
);
extern int READSEG[];

#include "libx2.h"
#include "PMLoader.h"
#include "Descriptor.h"
#include "def.h"

//开始引导
__asm__(
"ljmp $STARTSEG,$HERE \n\t"
"HERE: \n\t"
"cli \n\t"
"mov $STARTSEG,%ax \n\t"
"mov %ax,%ds \n\t"
"mov %ax,%ss \n\t"
"mov %ax,%es \n\t"
"mov $STACKSIZE,%sp \n\t"
"#mov %ax,%sp \n\t"
"call _readLaterSectors\n\t"
"pushw $TEMP_SEG \n\t"
"pushw $__Z8theEntryv \n\t"  //先选择符，选择符在高位
"ljmpw *(%esp) \n\t" //不能使用main，gcc对main有特殊处理
);

//突破1扇区的限制先
//将后面的扇区读到起始地址 0xa000处
extern "C" void readLaterSectors()
{
    __asm__(
        "READLEN =  16 -2  \n\t"
        "push %es\n\t"
        "movw $TEMP_SEG,%ax \n\t"  //这些来自PMLoader的参数尚未加载
        "mov %ax,%es\n\t"
        "mov $STACKSIZE,%bx\n\t" //cx=start-sector -->es:bx+READLEN
        "xor %dx,%dx \n\t"
        "mov $0x0003,%cx \n\t"
        "mov $0x200+READLEN,%ax \n\t"
        "int $0x13 \n\t"
        "pop %es \n\t"
    );
}

__attribute__((section(".test_section"))) int theEntry() //this is placed in .test_section,which is placed at 0x7c0:0x400
{
    __asm__(
    "movl $TEMP_SEG,%eax \n\t"
    "mov %eax,%ds\n\t"
    "mov %eax,%es \n\t"
    "mov %eax,%ss \n\t"  //栈指针 保持不变
    ); //到此开始，ds可用，PMLoader的数据才有效
    //调整，将0x7c0:处的1个扇区复制到此(正在进行的栈不能复制，复制的主要是字符串等数字)
    Util::memcopy(0x7c0,0,PMLoader::TEMP_SEG,0,PMLoader::SECSIZE);
    
    //===========Class Test Start============
//    Test t;//c++的默认构造函数不能加括号，否则可能被看成是函数声明
    Util::printStr("Hello World!");//测试默认参数
    //堆栈不足以支持这么多函数，因此需要适时扩张堆栈
    
    //测试类的构造、析构、成员函数
    Util t;
    t.test();
    //===========Class Test End============
    
    //===========Util Test Start============
    int readBase=PMLoader::TEMP_SEG*16;
    Util::memcopy(PMLoader::TEMP_SEG,512,PMLoader::TEMP_SEG,0,512);//将第一个扇区清空
    Util::insertMark(0x555);
    if(Util::readSectors(PMLoader::TEMP_SEG,0,0,0,1))
    {
        Util::printStr("Load Tested.\n");
    }
    SegmentDescriptor sd1(readBase,1024);
    char saver[8];
    sd1.writeToMemory(0x10000,saver);
    SegmentDescriptor sd2;
    SegmentDescriptor::fromMemory(&sd2,0x10000,saver);
    char saver2[8];
    sd2.writeToMemory(0x10000,saver2);
    if(sd1.equals(sd2))
    {
        Util::printStr("SegmentDescriptor Right!!!\n");
    }
    Util::printStr("Util The End.\n");
    //===========Util Test End============
    
    //使用PMLoader的主过程加载
    //加载配置在编译期已经配置好
    PMLoader::mainProcess();

}

__asm__(
".section .last_section \n\t"
".org 2 \n\t"
".word 0xaa55 \n\t"
);

#endif