
; ��Ϊ���ǻᾡ�����ٵ�ʹ�û�࣬���Ի����������С�λ�����
; �����Ƿֱ�����ڲ�ͬ�ļ���ȷʵ�е��鷳��
; ���ÿ��ģ��ֻ����һ����ģ����ͬ���Ļ���ļ������ڴ��������ػ�����

[SECTION .text]
[BITS 32]

extern save_restart
extern _kernel_start

; ��һ�δ����������ǰ��
; ��������ת���ں˳�ʼ������
push	0
push	_kernel_start
jmp	save_restart


; IPC ģ��
global	_ipc_int
extern	_do_ipc		; ��Ӧ�жϵ�C����
extern	save_restart

; �жϺ���
_ipc_int:
	push	0
	push	_do_ipc
	jmp	save_restart


; keyboard ģ��
extern	_do_int_key
extern	save_restart
global	_int_key

; IRQ1
_int_key:
	push	0
	push	_do_int_key
	jmp	save_restart


; ʱ��ģ��
global	_clock_int
extern	_do_clock_int
extern	_do_sched_clock_int

do_clock_int:
	call	_do_sched_clock_int
	call	_do_clock_int
;	out(INT_M_CTRL,0X20)
	ret

_clock_int:
	sub	esp, 4
	push	do_clock_int
	jmp	save_restart
