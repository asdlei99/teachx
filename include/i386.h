
#ifndef _I386_H_
#define _I386_H_
#include "type.h"

/*==============================================================================
�������� 
================================================================================ 

   ������������8�ֽ�
   �����ڲ��ṹ�����ҡ��������Բ�������ڲ��ṹ��û��ʲô�ô��ġ� 
   | BYTE7 | BYTE6 | BYTE5 | BYTE4 | BYTE3 | BYTE2 | BYTE1 | BYTE0 |
   | 31~24 |               |         23~0          |     15~0      | 
   |�λ�ַ |               |        �λ�ַ         |    �ν���     | 
   _______/                \________________________________________
   | 7 | 6 | 5 | 4 | 3     ~     0 | 7 | 6 | 5 | 4 | 3     ~     0 | 
   | G |D/B| 0 |AVL|  �ν���19~16  | P |  DPL  | S |     TYPE      |
*/
struct DESCRIPTOR{
	u32 d0;
	u32 d1; 
};

//Ĭ��4K���롢32λ������ 
//                DESCRIPTOR* void* u32    u32
#define _set_desc(desc_addr , base, limit, attr)	\
do{											\
	(desc_addr)->d0 = ((limit)&0xffff) | (((u32)(base)&0xffff)<<16);	\
	(desc_addr)->d1 = (((u32)(base)>>16)&0xff) | ((attr) /*&0x00f0ff00*/ ) | ((u32)(base)&0xff000000) | ((limit)&0xf0000);	\
}while(0)

#pragma pack (2)
struct GDTPTR{
	u16 limit;				/*�߽�*/
	struct DESCRIPTOR *BaseAddr;	/*��ַ*/ 
};
#pragma pack ()
typedef struct GDTPTR _GDTPTR;
AssertSizeof(_GDTPTR,6);

/*==============================================================================
�������� 
================================================================================ 

   ������������8�ֽ�
   �����ڲ��ṹ�����ҡ��������Բ�������ڲ��ṹ��û��ʲô�ô��ġ� 
   | BYTE7 | BYTE6 | BYTE5 | BYTE4 | BYTE3 | BYTE2 | BYTE1 | BYTE0 |
   |  ƫ��(31~16)  |               |    ѡ����     |  ƫ��(15~0)   |
   ________________/               \________________________________ 
   | 7 | 6 | 5 | 4 | 3     ~     0 | 7 | 6 | 5 | 4       ~       0 | 
   | P |  DPL  | S |     TYPE      | 0   0   0 |       ����        |
*/

//               desc*      u32  p    u32
#define _set_gate(gate_addr,attr,func,selector)	\
do{		\
	(gate_addr)->d0 = ((u32)(func)&0xffff) | ((selector)<<16);	\
	(gate_addr)->d1 = ((u32)(func)&0xffff0000) | (attr);		\
}while(0)

#pragma pack (2)
struct IDTPTR{
	u16 limit;		/*�߽�*/
	struct DESCRIPTOR *BaseAddr;	/*��ַ*/ 
};
#pragma pack ()
typedef struct IDTPTR _IDTPTR;
AssertSizeof(_IDTPTR,6);

struct TSS{
	u32	backlink;
	u32	esp0;		/* stack pointer to use during interrupt */
	u32	ss0;		/*   "   segment  "  "    "        "     */
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;
	u32	ss;
	u32	ds;
	u32	fs;
	u32	gs;
	u32	ldt;
	u16	trap;
	u16	iobase;	/* I/Oλͼ��ַ���ڻ����TSS�ν��ޣ��ͱ�ʾû��I/O���λͼ */
	/*t_8	iomap[2];*/
};


/* ����������ֵ˵�� */
#define	DA_32			0x400000	/* 32 λ��				*/
#define	DA_LIMIT_4K		0x800000	/* �ν�������Ϊ 4K �ֽ�			*/
#define	DA_DPL0			0x0000
#define	DA_DPL1			0x2000
#define	DA_DPL2			0x4000
#define	DA_DPL3			0x6000
/* �洢������������ֵ˵�� */
#define	DA_DR			0x9000	/* ���ڵ�ֻ�����ݶ�����ֵ			*/
#define	DA_DRW			0x9200	/* ���ڵĿɶ�д���ݶ�����ֵ			*/
#define	DA_DRWA			0x9300	/* ���ڵ��ѷ��ʿɶ�д���ݶ�����ֵ	*/
#define	DA_C			0x9800	/* ���ڵ�ִֻ�д��������ֵ			*/
#define	DA_CR			0x9A00	/* ���ڵĿ�ִ�пɶ����������ֵ		*/
#define	DA_CCO			0x9C00	/* ���ڵ�ִֻ��һ�´��������ֵ		*/
#define	DA_CCOR			0x9E00	/* ���ڵĿ�ִ�пɶ�һ�´��������ֵ	*/
/* ϵͳ������������ֵ˵�� */
#define	DA_LDT			0x8200	/* �ֲ��������������ֵ		*/
#define	DA_TaskGate		0x8500	/* ����������ֵ				*/
#define	DA_386TSS		0x8900	/* ���� 386 ����״̬������ֵ*/
#define	DA_386CGate		0x8C00	/* 386 ����������ֵ			*/
#define	DA_386IGate		0x8E00	/* 386 �ж�������ֵ			*/
#define	DA_386TGate		0x8F00	/* 386 ����������ֵ			*/

/* ѡ��������ֵ˵�� */
/* ����, SA_ : Selector Attribute */
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TIG		0
#define	SA_TIL		4

/* �ж����� */
#define	INT_VECTOR_DIVIDE		0x0
#define	INT_VECTOR_DEBUG		0x1
#define	INT_VECTOR_NMI			0x2
#define	INT_VECTOR_BREAKPOINT	0x3
#define	INT_VECTOR_OVERFLOW		0x4
#define	INT_VECTOR_BOUNDS		0x5
#define	INT_VECTOR_INVAL_OP		0x6
#define	INT_VECTOR_COPROC_NOT	0x7
#define	INT_VECTOR_DOUBLE_FAULT	0x8
#define	INT_VECTOR_COPROC_SEG	0x9
#define	INT_VECTOR_INVAL_TSS	0xA
#define	INT_VECTOR_SEG_NOT		0xB
#define	INT_VECTOR_STACK_FAULT	0xC
#define	INT_VECTOR_PROTECTION	0xD
#define	INT_VECTOR_PAGE_FAULT	0xE
#define	INT_VECTOR_COPROC_ERR	0x10

#endif
