
; �Ӵ��Ժ�Ĵ����ڱ���ģʽ��ִ��
; 32 λ�����. ��ʵģʽ����
; ��������һ�����壬��Ϊ�ҵ������������ƣ�����Ҫһ��ͷ����Ρ�

%include "..\include\system.inc"

extern _setup32
[SECTION .text]
[BITS	32]

LABEL_PM_START:
	call	_setup32	; �������ֻ�����ʼ��
	
	mov	ax, SELECTOR_KERNEL_DS
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov	esp, TOP_STACK
	mov	ax, SELECTOR_GS
	mov	gs, ax

	jmp	SELECTOR_KERNEL_CS:OFFSET_KERNEL	; ϵͳ��ִ�жο�ͷ
