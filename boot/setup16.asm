
; ���ܣ�
; ��ʾSetup ...
; ���ж�
; ������ʱgdt
; ����A20��ַ����
; ���뱣��ģʽ
; ����setup32����


%include "pm.inc"	; ���ڱ���ģʽ�ĳ������ꡢ�Լ�һЩ˵��
%include "load.inc"	; ���ڸ�ģ�鱻���ص��ڴ�����

org OffsetOfSetup

BaseOfStack:
LABEL_START_CODE:	; �����������ʵ��ţ�ջ���ʹ����￪ʼ

	jmp LABEL_START

[SECTION .gdt]
ALIGN 8

GdtPtr  dw  GdtLen				; �α߽�
        dd  BaseOfSetupPhyAddr + LABEL_GDT	; ����ַ

ALIGN 8

;				    �λ�ַ,  �ν���, ����
; ��������
LABEL_GDT:              Descriptor       0,       0, 0
; 0 ~ 4G �Ŀ�ִ�ж�
LABEL_DESC_FLAT_C:      Descriptor       0, 0fffffh, DA_CR|DA_32|DA_LIMIT_4K
; 0 ~ 4G �Ŀɶ�д��
LABEL_DESC_FLAT_RW:     Descriptor       0, 0fffffh, DA_DRW|DA_32|DA_LIMIT_4K

GdtLen	equ $ -LABEL_GDT


; ѡ����
SelectorFlatC   equ LABEL_DESC_FLAT_C   - LABEL_GDT
SelectorFlatRW  equ LABEL_DESC_FLAT_RW  - LABEL_GDT

[SECTION .data]
Message	db	"Setup ..." ,0dh,0ah,00

[SECTION .s16]
[BITS 16]
LABEL_START:
	mov	ax, cs
	mov	ds, ax
	
	mov	si, Message
	call	DisplayStr

	cli
	lgdt [GdtPtr]

	in  al, 92h
	or  al, 00000010b
	out 92h, al
	
	mov eax, cr0
	or  eax, 1
	mov cr0, eax

	mov	ax, SelectorFlatRW	; ֻ�����������μĴ������� 
	mov	ds, ax
	mov ss, ax 
	mov	esp, BaseOfStack + BaseOfSetupPhyAddr
	
	jmp	dword SelectorFlatC:(BaseOfSetupPhyAddr+OffsetOfSetup32)

;-------------------------------------------------------------------	
;������������ʾ�ַ���,�ַ�����'00h'��β
;����˵��: ds:si ָ���ַ����׵�ַ
;����ֵ: �� 
DisplayStr:
	pushf
	push ax
	cld				;�ַ���������ߵ�ַ
	
	mov  ah, 0eh
CONTINE:	
	lodsb
	cmp al,0
	jz  EXIT 
	int 10h
	jmp CONTINE
EXIT:
	pop ax
	popf
	ret
