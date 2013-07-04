
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

/* ��̬�ڴ�ռ䲼�� */ 
#define HANDLE_TABLE	0X7FFFE000		//������λ�� 
#define KERNEL_SEG		(1u<<31)		/*2G*/
#define PTT 			KERNEL_SEG
#define PDT				(PTT + (PTT>>10))
#define VGA				(PTT + _4M)
#define IDT				(VGA + 8*_4K)
#define GDT				(IDT + _2K)

#define STACK_BOTTOM	(GDT + _2K)
#define STACK_TOP		(STACK_BOTTOM + _1M)
#define kernel_entry	STACK_TOP				//0x509000
#define KERBEL_END		0X81000000
#define PTE_BEGIN		(KERNEL_SEG+4*_4M)
#define PTE_END			(KERNEL_SEG + 512*_1M)
#define VIEW_BUFFER_BEGIN	(0x80c00000)		//��ͼ������λ�� 
#define VIEW_BUFFER_END		(0xa0c00000)	

extern struct TSS g_tss;

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
