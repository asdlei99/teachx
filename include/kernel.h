
#ifndef _KERNEL_H_
#define _KERNEL_H_
/*
setup中的初始化要完成的任务：
1、IDT:
IDT占用内存2K，共256项，初始化就全部在内存中了 
指令 lidt 加载全部的256项，只是所有项都置零

2、GDT:
GDT占用内存2K，共256项，初始化就全部在内存中了
指令 lgdt 加载全部的256项，初始化项：
0 空描述符
1 GVA段 
2 系统可执行段
3 系统数据段
4 用户可执行段
5 用户数据段
6 TSSA段 
7 系统进程可执行段
8 系统进程数据段 
不使用的项置零

之所以加载它们（IDT,GDT）的所有项，是为了以后不用在加载它们，用到的时候直接使其有效即可。
有内核堆栈，但是没有内核堆栈段。 

页目录&页表占用4M是因为内核需要管理整个映射函数 


gs	将全程保存视频缓冲区段选择子 
*/
#include "i386.h"
#include "config.h" 

/* 选择子 */
#define	SELECTOR_DUMMY		0
#define SELECTOR_VIDEO		(0x8 | SA_RPL3)
#define	SELECTOR_KERNEL_CS	0x10
#define	SELECTOR_KERNEL_DS	0x18
#define SELECTOR_USER_CS	(0x20 | SA_RPL3)
#define SELECTOR_USER_DS	(0x28 | SA_RPL3)

#define SELECTOR_SYS_CS		(0X38 | SA_RPL1)
#define SELECTOR_SYS_DS		(0X40 | SA_RPL1)

#define INDEX_TSS_IN_GDT	6 
#define SELECTOR_TSS		(INDEX_TSS_IN_GDT<<3)

#define gdt ((struct DESCRIPTOR*)GDT)
#define idt ((struct DESCRIPTOR*)IDT)

#define set_gdt(vector,base,limit,attr)	\
	_set_desc(&gdt[vector],base,limit,attr)
#define set_sys_idt(vector,offset)	\
	_set_gate(&idt[vector],DA_DPL0|DA_386IGate,offset,SELECTOR_KERNEL_CS)

#endif
