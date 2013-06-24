
/*
	Ŀǰֻ���ǵ��˴�������ʵ�� 
*/
#include "asm.h"
#include "assert.h"
#include "kernel.h"
#include "mm.h"
#include "process.h"
#include "sched.h"
#include "string.h"
#include "type.h" 
#include "intc.h"
#include "_sys_call.h"


struct thread_t *tcbn;	// ָ��������֯���߳̿��ƿ�� 
struct process_t *pcbn;	// ָ��������֯�Ľ��̿��ƿ�� 

void to_block(int tid)
{
	if(tcbn[tid].block_count == 0)
		sched_erase(tid);
	tcbn[tid].block_count ++;
}

void to_ready(int tid)
{
	tcbn[tid].block_count --;
	if(tcbn[tid].block_count == 0)
		sched_insert(tid);
}

void switch_to(struct thread_t *thread)
{
	//������process.asm�� 
	extern void restart(struct cpu_context_t* context);
	
	assert(thread->block_count == 0);
	
	if(thread->process->cr3 != s_cur_process->cr3){
		lcr3(thread->process->cr3);
	}

	s_cur_thread = thread;
	tss.esp0 = (u32)(&(s_cur_thread->context) + 1);
	restart(&s_cur_thread->context);
}

void set_retval(int tid,int retval)
{
	tcbn[tid].context.pushad.eax = retval;
}

static struct process_t *s_free_procs	= NULL;
static struct thread_t *s_cur_thread	= NULL;	//��ǰ�߳� 
static struct thread_t s_dead;					//����̬�����߳��Ѿ����������ǻ���ָ�����ľ�����ڡ� 
static struct thread_t s_hang;					//����̬ 
static struct thread_t *s_free_threads= NULL;	//���е�PCB

extern void clock_int();

#define s_cur_process	(s_cur_thread->process)

uint get_cur_pid()
{
	return s_cur_process - pcbn;
}

// �������̿��ƿ�ģ�� 
static struct process_t* create_proc()
{
/*
struct process_t{
	void *regions;
	void *tls_free;
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
	init_user_space(new_proc-pcbn);
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
	proc->threads->context.ds = SELECTOR_SYS_DS;
	proc->threads->context.fs = SELECTOR_DUMMY;
	proc->threads->context.eip = (u32)proc_addr;
	proc->threads->context.cs = SELECTOR_SYS_CS;
	proc->threads->context.eflags = sflags();
	proc->threads->context.esp = SYS_STACK_TOP;
	proc->threads->context.ss = SELECTOR_SYS_DS;
	
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

struct TSS tss = {0};	//���ù��� 

// ����TSS��ص����� 
void init_tss()
{
	set_gdt(INDEX_TSS_IN_GDT,&tss,sizeof(struct TSS),DA_386TSS|DA_DPL0);
	
	tss.backlink = 0;
	tss.ss0 = SELECTOR_KERNEL_DS;
	tss.esp0 = STACK_TOP;
	tss.iobase = sizeof(g_tss);
	ltr(SELECTOR_TSS);
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
	c_proc->threads->context = p_thread->context;

// �����ӽ����е����̵߳ķ���ֵ 
	c_proc->threads->context.pushad.eax = 0;

// �ӽ���û��ʵ���ϵȴ�����Ϣ�����ֱ�ӽ��ӽ��̷�������Ŷ�
// �丸���������ʹ��for_wait_msg������ 
	to_ready(c_proc->threads);
	
	return c_proc - pcbn;
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
