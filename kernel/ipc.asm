
[SECTION .text]
[BITS 32]

global	_ipc_int
extern	_do_ipc		; ��Ӧ�жϵ�C����
extern	save_restart

; �жϺ���
_ipc_int:
	push	0
	push	_do_ipc
	jmp	save_restart
