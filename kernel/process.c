
/*
	Ŀǰֻ���ǵ��˴�������ʵ�� 
*/
#include "asm.h"
#include "assert.h"
#include "kernel.h"
#include "mm.h"
#include "process.h"
#include "string.h"
#include "type.h" 
#include "intc.h"
#include "_sys_call.h"

/*
	i386=Intel 80386����ʵi386ͨ����������Ϊ��Intel 32λ΢��������ͳ�ơ� 
*/

/* pushad will push */
struct pushad_t{
	u32	edi;		/* �� Low		*/
	u32	esi;
	u32	ebp;
	u32	esp;		/* <- 'popad' will ignore it	*/
	u32	ebx;	
	u32	edx;
	u32	ecx;
	u32	eax;		/* ��Hig		*/
};

/*
���û�����ִ��ʱ���жϷ��ͽ��Դ�ѹջ��ss,esp,eflags�� 
ע�⣺�ڱ���ģʽ�£���BITS 32����pop push iret�ȶ�Ĭ��Ϊ32λָ� 

����Ȩ���仯���жϣ�CPU���ss,espѹջ
�û�����ֻ����cs��ds�μĴ�����gsΪȫ�ֶμĴ��� 
*/
struct stack_frame_t{	/*	proc_ptr points here	�� Low		*/
	u32	ds;			/* ��						��			*/
	u32 fs;
	struct pushad_t pushad;
	u32 return_addr;
	u32 error_code;
	u32	eip;		/* ��						��			*/
	u32	cs;			/* ��						��			*/
	u32	eflags;		/* �� these are pushed by CPU during interrupt		*/
	u32	esp;		/* ��						��			*/
	u32	ss;			/* ��						��High		*/
};

struct _msg_t;

struct process_t{
	u32 cr3;					// ���̵�ҳĿ¼��������ַ
	struct _msg_t *msg_head;	// �ý��̵Ĵ�������Ϣ�Ŷ� 
	struct thread_t *threads;	// �����е��߳�
	void *regions;
	void *tls_free;
	void* pls[MAX_PLS];
	struct thread_t *recv_next;	// �������Ӹý�������recv���������߳��� 
	struct process_t *child;	// ָ���һ���ӽ��� 
	struct process_t *sibling;	// ������������ͬһ�����̵��ӽ��� 
};

struct thread_t{
	struct stack_frame_t regs;	/* process' registers saved in stack frame */
#define STATUS_THREAD_FREE	0
#define STATUS_THREAD_READY	1
#define STATUS_THREAD_RECV	2	// �������Ϣ���������
#define STATUS_THREAD_WAIT	3	// ��ȴ����ض�������� 
#define STATUS_THREAD_DEAD	4
#define STATUS_THREAD_HANG	8	// ����һ������λ����������״̬�½������̬ 
	uint status;
#define MAX_THREAD_LIFE		20
	int life;
	u32 exit_code;
	struct process_t *process;	// ָ�������߳������Ľ���
	struct thread_t *sibling;	// ָ���Լ����ֵ��̣߳��������Ӹý����е������̡߳� 
	struct thread_t *recv_next;	// �������Ӹý�������recv���������߳��� 
	struct thread_t *prev;
	struct thread_t *next;
};

static struct process_t *s_free_procs	= NULL;
static struct thread_t *s_cur_thread	= NULL;	//��ǰ�߳� 
static struct thread_t s_ready;				//����̬ 
static struct thread_t s_block;				//����̬
static struct thread_t s_dead;				//����̬�����߳��Ѿ����������ǻ���ָ�����ľ�����ڡ� 
static struct thread_t s_hang;				//����̬ 
static struct thread_t *s_free_threads= NULL;	//���е�PCB

#define SIZE_THREAD_BUFFER	_4K
#define COUNT_THREAD		(SIZE_THREAD_BUFFER/sizeof(struct thread_t))
static struct thread_t s_thread[COUNT_THREAD];
#define BEGIN_THREAD_BUFFER	s_thread
#define END_THREAD_BUFFER	(BEGIN_THREAD_BUFFER + COUNT_THREAD)

#define SIZE_PROC_BUFFER	_4K
#define COUNT_PROC			(SIZE_PROC_BUFFER/sizeof(struct process_t))
static struct process_t s_process[COUNT_PROC];
#define BEGIN_PROC_BUFFER	s_process
#define END_PROC_BUFFER		(BEGIN_THREAD_BUFFER + COUNT_THREAD)

extern void restart(struct thread_t*);	//��process.asm�� 
extern void clock_int();

#define s_cur_process	(s_cur_thread->process)

uint get_cur_pid()
{
	return s_cur_process - BEGIN_PROC_BUFFER;
}

static void t_insert(struct thread_t *header,struct thread_t *thread)
{
	thread->prev = header->prev;
	thread->next = header;
	header->prev->next = thread;
	header->prev = thread;
}

static void t_erase(struct thread_t *thread)
{
	thread->prev->next = thread->next;
	thread->next->prev = thread->prev;
}

// ��ָ���߳��������̬ 
static void to_block(struct thread_t *thread,uint status)
{
	thread->status = status; 
	t_erase(thread);
	t_insert(&s_block,thread);
}

static void to_dead(struct thread_t *thread)
{
	thread->status = STATUS_THREAD_DEAD;
	t_erase(thread);
	t_insert(&s_dead,thread);
}

// ��ָ���߳��������̬ 
static void to_ready(struct thread_t *thread)
{
	thread->status = STATUS_THREAD_READY;
	t_erase(thread);
	t_insert(&s_ready,thread);
} 

static void switch_to(struct thread_t *pthread)
{
	if(pthread->process->cr3 != s_cur_process->cr3)
		lcr3(pthread->process->cr3);
	s_cur_thread = pthread;
	g_tss.esp0 = (u32)(&(pthread->regs) + 1);
	restart(pthread);
}

void do_clock_int()
{
	eoi_m();
	s_cur_thread->life --;
}

void schedule()
{
	if(s_cur_thread){
		if(s_cur_thread->life<0){
			s_cur_thread->life += 20;
			to_ready(s_cur_thread);
			s_cur_thread = NULL;
		}else if(s_cur_thread->status != STATUS_THREAD_READY){
			s_cur_thread = NULL;
		}
	}
	
	if(s_cur_thread == NULL){
		if(s_ready.next != &s_ready){
			switch_to(s_ready.next);
		}else
			assert(!"no a process!");
	}
}

/* ************************************************************************** */ 

struct _msg_t{
	struct thread_t *t_from;// ֻ��
#define ATTR_MSG_COM	1	// �Ѵ������ 
#define ATTR_MSG_WAIT	2	// ���͸���Ϣ�Ľ���Ҫ�ȴ�����Ϣ�ķ��� 
#define ATTR_MSG_RETVAL	4	// ���ط���ֵ 
	uint status;
	int retval;
	struct msg_t msg;
	
	struct _msg_t *next;
	struct _msg_t *prev;
};

#define SIZE_MSG_BUFFER		_4K
#define COUNT_MSG			(SIZE_MSG_BUFFER/sizeof(struct _msg_t))
static struct _msg_t s_msg[COUNT_MSG];
#define BEGIN_MSG_BUFFER	s_msg
#define END_MSG_BUFFER		(BEGIN_MSG_BUFFER+COUNT_MSG)

static struct _msg_t s_msg_deal_header;	// �ȴ�����ֵ����Ϣ�����ͷ 
static struct _msg_t *s_free_msgs;

static void m_insert(struct _msg_t *header,struct _msg_t *pmsg)
{
	pmsg->prev = header->prev;
	pmsg->next = header;
	header->prev->next = pmsg;
	header->prev = pmsg;
}

static void m_erase(struct _msg_t *pmsg)
{
	pmsg->next->prev = pmsg->prev;
	pmsg->prev->next = pmsg->next;
}

// ��������Ϣ�����Ŷ� 
int send_msg(uint pid_to,struct msg_t *pmsg,bool_t need_wait)
{
	struct process_t *pproc	= BEGIN_PROC_BUFFER + pid_to;
	struct thread_t *pthread= pproc->recv_next;
	
	pmsg->pid_from = get_cur_pid();

// �����ҵ����շ�����Ҫ��Ϣ��û����Ϣ�����������߳�
// �������ʹ��������̬��û��������
	if(pthread != NULL){
		pthread->regs.pushad.eax = -2;	// ���� -2 ��ʾ���߳�֮ǰ��û����Ϣ��������
		to_ready(pthread);				// ����������Ϣ�ˣ�����Ҫ�ٴν��� 
	}

// ����Ϣ�����Ŷ� 
	struct _msg_t *_pmsg = s_free_msgs;

	s_free_msgs = s_free_msgs->next;
	
	_pmsg->status = need_wait? ATTR_MSG_WAIT:0;
	_pmsg->msg = *pmsg;

// �������������
	m_insert(pproc->msg_head,_pmsg);
	
// ���ظ���Ϣid -- mid :) 
	return _pmsg - BEGIN_MSG_BUFFER;
}

int wait_msg(int hmsg,bool_t need_retval)
{
	struct _msg_t *_pmsg;

	if(hmsg<0 || hmsg>=COUNT_MSG)
		return -1;

	_pmsg = BEGIN_MSG_BUFFER + hmsg;
	
	if(_pmsg->t_from != NULL || _pmsg->msg.pid_from != get_cur_pid())
		return -1;

	if(_pmsg->status & ATTR_MSG_COM){
		m_erase(_pmsg);
		_pmsg->next = s_free_msgs;
		s_free_msgs = _pmsg;
		return _pmsg->retval;
	}else{
		if(need_retval){
			_pmsg->status |= ATTR_MSG_RETVAL;
		}
		_pmsg->t_from = s_cur_thread;
		to_block(s_cur_thread,STATUS_THREAD_WAIT);
	}
}

void for_wait_msg(int hmsg,int retval)
{
	struct _msg_t *_pmsg = BEGIN_MSG_BUFFER + hmsg;

	if(_pmsg->t_from == NULL){
		_pmsg->status |= ATTR_MSG_COM;
		_pmsg->retval = retval;
	}else{
		if(_pmsg->status & ATTR_MSG_RETVAL)
			_pmsg->t_from->regs.pushad.eax = retval;
		to_ready(_pmsg->t_from);
	}
}

int recv_msg(struct msg_t *pmsg)
{
	struct process_t *pproc = s_cur_process;
	struct thread_t *pthread = s_cur_thread;

	struct _msg_t *_pmsg = pproc->msg_head->next;
	if(_pmsg == pproc->msg_head){
		t_erase(pthread);
		pthread->recv_next = pproc->recv_next;
		pproc->recv_next = pthread;
		pthread->status = STATUS_THREAD_RECV;
	}else{
		*pmsg = _pmsg->msg;
		m_erase(_pmsg);
		if(_pmsg->status & ATTR_MSG_WAIT){
			m_insert(&s_msg_deal_header,_pmsg);
			return _pmsg - BEGIN_MSG_BUFFER;
		}else{
			_pmsg->next = s_free_msgs;
			s_free_msgs = _pmsg;
			return INVALID_HMSG;
		}
	}
}

/*
	����������뾡�췵�� �� ������� 
	
	RECV��
		����ֵ�� 
			������͸���Ϣ�Ľ�����Ҫ����ֵ���򷵻ظ���Ϣ�ľ��
			�������Ϣ����Ҫ����ֵ���򷵻�-1 
*/
/* ��ֵ���ں��еĶ������һ�£�Ҳ��˵������������ϡ� */
#define IPC_RECV		0X01	// ������Ϣ
#define IPC_SEND		0X02	// ������Ϣ 
#define IPC_WAIT		0X03	// �ȴ�
#define IPC_SEND_WAIT	0X04	// ������Ϣ�����ȴ�����Ϣ���� 
#define IPC_FOR_WAIT	0X05	// 

#define IPC_ATTR_WAIT	0X10	// ֻ������ IPC_WAIT ���� 

int do_ipc(uint func,uint pid_to_or_hmsg,struct msg_t *pmsg)
{
	switch(func)
	{
	case IPC_RECV:
		return recv_msg(pmsg);
	break;
	case IPC_SEND:
		return send_msg(pid_to_or_hmsg,pmsg,FALSE);
	break;
	case IPC_WAIT:
		return wait_msg(pid_to_or_hmsg,TRUE);
	break;
	case IPC_SEND_WAIT:
		pid_to_or_hmsg = send_msg(pid_to_or_hmsg,pmsg,TRUE);
		return wait_msg(pid_to_or_hmsg,TRUE);
	break;
	case IPC_FOR_WAIT:
		for_wait_msg(pid_to_or_hmsg,pmsg->p0.iparam);
	break;
	case IPC_SEND | IPC_ATTR_WAIT:
		return send_msg(pid_to_or_hmsg,pmsg,TRUE);
	break;
	default:
		
	break;
	}
}

void init_ipc()
{
	int i;

	s_msg_deal_header.next = s_msg_deal_header.prev = &s_msg_deal_header;
	s_free_msgs = NULL;
	for(i=0;i<COUNT_MSG;i++)
	{
		BEGIN_MSG_BUFFER[i].next = s_free_msgs;
		s_free_msgs = &BEGIN_MSG_BUFFER[i];
	}
	set_sys_idt(0x80,ipc_int);
}

// �������̿��ƿ�ģ�� 
static struct process_t* create_proc()
{
/*
struct process_t{
	void *regions;
	void *tls_free;
	void *pls_free;
	void* pls[MAX_PLS];
};
*/
static bool_t created = FALSE;	// ��¼�Ƿ��ѵ��ù������� 
	struct _msg_t *new_msg = s_free_msgs;
	struct thread_t *new_thread = s_free_threads;
	struct process_t *new_proc = s_free_procs;

	if(	s_free_msgs == NULL ||
		s_free_threads == NULL ||
		s_free_procs == NULL)
		return NULL;

	s_free_procs = s_free_procs->child;
	s_free_threads = s_free_threads->next;
	s_free_msgs = s_free_msgs->next;

	new_msg->next = new_msg->prev = new_msg;
// ��ʼ����������
	if(created == FALSE){
		new_proc->cr3 = scr3();
		created = TRUE;
	}else{
		new_proc->cr3 = create_vas();
	}
	new_proc->msg_head = new_msg;
	new_proc->threads = new_thread;
	init_user_space(new_proc-BEGIN_PROC_BUFFER);
	new_proc->recv_next = NULL;
	new_proc->child = NULL;
	new_proc->sibling = NULL;	// �ɿ�¡������д���ٴ�ֻ�ǳ�ʼ�� 
	
	new_thread->status = STATUS_THREAD_HANG;
	new_thread->life = MAX_THREAD_LIFE;
	new_thread->process = new_proc;
	new_thread->sibling = NULL;
	new_thread->recv_next = NULL;
	
	t_insert(&s_hang,new_thread);
	
	return new_proc;
}

void create_sys_proc(void (*proc_addr)())
{
	struct process_t *proc = create_proc();
	
	// ��ʼ�����߳����� 
	// ��ʼ���Ĵ������ݣ�����ʼ��ͨ�üĴ����� 
	proc->threads->regs.ds = SELECTOR_SYS_DS;
	proc->threads->regs.fs = SELECTOR_DUMMY;
	proc->threads->regs.eip = (u32)proc_addr;
	proc->threads->regs.cs = SELECTOR_SYS_CS;
	proc->threads->regs.eflags = sflags();
	proc->threads->regs.esp = SYS_STACK_TOP;
	proc->threads->regs.ss = SELECTOR_SYS_DS;
	
	reserve_region(proc-BEGIN_PROC_BUFFER,SYS_STACK_BASE,SYS_STACK_TOP-SYS_STACK_BASE,ATTR_RT_WRITE);
	to_ready(proc->threads);
}

void init_process_ctrl(void (*init_proc)())
{
	int i;
	
	s_free_procs = &BEGIN_PROC_BUFFER[0];
	for(i=0;i<COUNT_PROC-1;i++)
	{
		BEGIN_PROC_BUFFER[i].child = &BEGIN_PROC_BUFFER[i+1];
	}
	BEGIN_PROC_BUFFER[COUNT_PROC-1].child = NULL;
	
	s_free_threads = &BEGIN_THREAD_BUFFER[0];
	for(i=0;i<COUNT_THREAD-1;i++)
	{
		BEGIN_THREAD_BUFFER[i].next = &BEGIN_THREAD_BUFFER[i+1];
	}
	BEGIN_THREAD_BUFFER[i].next = NULL;
	
	s_free_msgs = &BEGIN_MSG_BUFFER[0];
	for(i=0;i<COUNT_MSG;i++)
	{
		BEGIN_MSG_BUFFER[i].next = &BEGIN_MSG_BUFFER[i+1];
	}
	BEGIN_MSG_BUFFER[i].next = NULL;
	
	s_ready.prev = s_ready.next = &s_ready;
	s_block.prev = s_block.next = &s_block;
	s_dead.prev = s_dead.next = &s_dead;
	s_hang.prev = s_hang.next = &s_hang;
	
	init_ipc();
	
	create_proc(init_proc);
	
	set_8259a_idt(INT_CLOCK,clock_int);
	enable_irq(INT_CLOCK);
}

static int do_fork(struct thread_t *p_thread)
{
	struct process_t *c_proc = create_proc();
	
	if(c_proc == NULL)
		return -1;

// ȷ�����ӹ�ϵ 
	c_proc->sibling = p_thread->process->child;
	p_thread->process->child = c_proc;

// ��¡���� 
	c_proc->cr3 = p_thread->process->cr3;
	c_proc->threads->regs = p_thread->regs;

// �����ӽ����е����̵߳ķ���ֵ 
	c_proc->threads->regs.pushad.eax = 0;

// �ӽ���û��ʵ���ϵȴ�����Ϣ�����ֱ�ӽ��ӽ��̷�������Ŷ�
// �丸���������ʹ��for_wait_msg������ 
	to_ready(c_proc->threads);
	
	return c_proc - BEGIN_PROC_BUFFER;
}

void pm_process()
{
	int hmsg;
	int ret_val;
	struct msg_t msg;
	while(1)
	{
		hmsg = ipc_recv(&msg);
		switch(msg.type)
		{
		case SC_FORK:
			ret_val = do_fork();
		break;
		default:
		
		break;
		}
		if(INVALID_HMSG != hmsg)
			ipc_for_wait(hmsg,ret_val);
	}
}

void* get_proc_regions(uint pid)
{
	return BEGIN_PROC_BUFFER[pid].regions;
}

void set_proc_regions(uint pid,void *regions)
{
	BEGIN_PROC_BUFFER[pid].regions = regions;
}
