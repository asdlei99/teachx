
; ����ֻ��һЩ��������

BaseOfSetup		equ	 08000h	; SETUP.BIN �����ص���λ�� ----  �ε�ַ
OffsetOfSetup		equ	  0100h	; SETUP.BIN �����ص���λ�� ----  ƫ�Ƶ�ַ
OffsetOfSetup16		equ	  0100h ; SETUP16.BIN ���ڴ��е�ƫ��
OffsetOfSetup32		equ	  0300h ; SETUP32.BIN ���ڴ��е�ƫ��

BaseOfSetupPhyAddr	equ	BaseOfSetup * 10h	; SETUP.BIN �����ص���λ�� ---- ������ַ (= BaseOfSetup * 10h)