
#ifndef _INTC_H_
#define _INTC_H_
#include "kernel.h" 

/* 8259A�������˿� */
#define INT_M_CTRL		0X20
#define INT_M_CTRLMASK	0X21
#define INT_S_CTRL		0XA0
#define INT_S_CTRLMASK	0XA1

/* �����趨8259a�ж�������� */
#define	INT_VECTOR_IRQ	0x20

/* �жϺ� */
#define INT_CLOCK	0
#define INT_KEY		1
#define INT_FLOPPY	6
#define INT_AT		14 

/* �ֶ�֪ͨ�����ж� */
#define eoi_m()	out(INT_M_CTRL,0X20)
#define eoi_s()	out(INT_S_CTRL,0X20)

/* ����\�ر�ĳ�ж� */ 
#define enable_irq(irq) do{	\
	if(irq<8)				\
		out(INT_M_CTRLMASK,in(INT_M_CTRLMASK) & (u8)(~(1<<irq)));			\
	else																	\
		out(INT_S_CTRLMASK,in(INT_S_CTRLMASK) & (u8)(~((1<<(irq))>>8)));	\
}while(0)

#define disable_irq(irq) do{	\
	if(irq<8)					\
		out(INT_M_CTRLMASK,in(INT_M_CTRLMASK) | (u8)(1<<irq));			\
	else																\
		out(INT_S_CTRLMASK,in(INT_S_CTRLMASK) | (u8)((1<<(irq))>>8));	\
}while(0)

/* ����\�ر������ж� */
#define enable_all_irq()		\
do{								\
	out(INT_M_CTRLMASK,0X0);	\
	out(INT_S_CTRLMASK,0X0);	\
}while(0)

#define set_8259a_idt(vector,offset) set_sys_idt((vector)+INT_VECTOR_IRQ,offset)

#endif
