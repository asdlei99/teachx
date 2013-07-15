
#ifndef _KERNEL_H_
#define _KERNEL_H_
/*
setup�еĳ�ʼ��Ҫ��ɵ�����
1��IDT:
IDTռ���ڴ�2K����256���ʼ����ȫ�����ڴ����� 
ָ�� lidt ����ȫ����256�ֻ�����������

2��GDT:
GDTռ���ڴ�2K����256���ʼ����ȫ�����ڴ�����
ָ�� lgdt ����ȫ����256���ʼ���
0 ��������
1 GVA�� 
2 ϵͳ��ִ�ж�
3 ϵͳ���ݶ�
4 �û���ִ�ж�
5 �û����ݶ�
6 TSSA�� 
7 ϵͳ���̿�ִ�ж�
8 ϵͳ�������ݶ� 
��ʹ�õ�������

֮���Լ������ǣ�IDT,GDT�����������Ϊ���Ժ����ڼ������ǣ��õ���ʱ��ֱ��ʹ����Ч���ɡ�
���ں˶�ջ������û���ں˶�ջ�Ρ� 

ҳĿ¼&ҳ��ռ��4M����Ϊ�ں���Ҫ��������ӳ�亯�� 


gs	��ȫ�̱�����Ƶ��������ѡ���� 
*/
#include "i386.h"
#include "config.h" 

/* ѡ���� */
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
