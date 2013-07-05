
/*
	Ŀǰֻ���ǵ��˴�������ʵ�� 
*/
#include "asm.h"
#include "assert.h"
#include "config.h"
#include "kernel.h"
#include "mm.h"
#include "process.h"
#include "sched.h"
#include "string.h"
#include "stdio.h"
#include "type.h" 
#include "intc.h"
#include "ipc.h"
#include "_sys_call.h"


static struct thread_t *tcbn;	// ָ��������֯���߳̿��ƿ�� 
static struct process_t *pcbn;	// ָ��������֯�Ľ��̿��ƿ�� 
static struct thread_t *s_free_threads= NULL;	//���е�PCB
static struct process_t *s_free_procs	= NULL;
static struct thread_t *s_cur_thread	= NULL;	//��ǰ�߳� 
#define s_cur_process	(s_cur_thread->process)
static struct TSS tss = {0};

struct process_t* get_cur_proc()
{
	return s_cur_process;
}

struct thread_t* get_cur_thread()
{
	return s_cur_thread;
}

struct process_t* get_proc(int pid)
{
	return &pcbn[pid];
} 

void to_block_ex(struct thread_t *thread)
{	
	if(thread->block_count == 0)
		sched_erase(thread);
	thread->block_count ++;
}

void to_block(int tid)
{
	to_block_ex(&tcbn[tid]);
}

void to_ready_ex(struct thread_t *thread)
{	
	thread->block_count --;
	if(thread->block_count == 0)
		sched_insert(thread);
}

void to_ready(int tid)
{
	to_ready_ex(&tcbn[tid]);
}

void switch_to(struct thread_t *thread)
{
	//������process.asm�� 
	extern void restart(struct cpu_context_t* context);
	
	assert(thread->block_count == 0);
	
	assert(thread->process && s_cur_process);
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

void set_retval_ex(struct thread_t *thread,int retval)
{
	thread->context.pushad.eax = retval;
}

static struct process_t* alloc_pcb()
{
	struct process_t *new_proc;
	
	new_proc = s_free_procs;
	s_free_procs = s_free_procs->sibling;
	
	return new_proc;
}

static void free_pcb(struct process_t *pcb)
{
	pcb->sibling = s_free_procs;
	s_free_procs = pcb;
}

static struct thread_t* alloc_tcb()
{
	struct thread_t *new_thread;
	
	new_thread = s_free_threads;
	s_free_threads = s_free_threads->sibling;
	
	return new_thread;
}

static void free_tcb(struct thread_t *tcb)
{
	tcb->sibling = s_free_threads;
	s_free_threads = tcb;
}

uint get_cur_pid()
{
	return s_cur_process - pcbn;
}

uint get_cur_tid()
{
	return s_cur_thread - tcbn;
}

// �������̿��ƿ�ģ�� 
static struct process_t* create_proc()
{
/*
struct process_t{
	void *tls_free;
	void* pls[MAX_PLS];
};
*/
static bool_t created = FALSE;	// ��¼�Ƿ��ѵ��ù������� 
	struct thread_t *new_thread = alloc_tcb();
	struct process_t *new_proc = alloc_pcb();

// ��ʼ����������
	if(created == FALSE){
		new_proc->cr3 = scr3();
		created = TRUE;
	}else{
		new_proc->cr3 = create_vas();
	}

	new_proc->threads = new_thread;
	new_proc->mm.region_hdr = create_vasr(NULL,(void*)_4K,(void*)KERNEL_SEG);
	new_proc->child = NULL;
	new_proc->sibling = NULL;	// �ɿ�¡������д���ٴ�ֻ�ǳ�ʼ�� 
	
	new_thread->status = STATUS_THREAD_HANG;
	new_thread->block_count = 1;
	new_thread->process = new_proc;
	new_thread->sibling = NULL;
	new_thread->recv_next = NULL;
	
	return new_proc;
}

struct thread_t* create_sys_proc(void (*proc_addr)())
{
	struct process_t *proc = create_proc();
	
	// ��ʼ�����߳����� 
	// ��ʼ���Ĵ������ݣ�����ʼ��ͨ�üĴ����� 
	proc->threads->context.ds_es = SELECTOR_SYS_DS;
	proc->threads->context.fs = SELECTOR_DUMMY;
	proc->threads->context.eip = (u32)proc_addr;
	proc->threads->context.cs = SELECTOR_SYS_CS;
	proc->threads->context.eflags = sflags();
	proc->threads->context.esp = SYS_STACK_TOP;
	proc->threads->context.ss = SELECTOR_SYS_DS;
	
	alloc_region(proc->mm.region_hdr,
		(void*)SYS_STACK_BASE,
		SYS_STACK_TOP-SYS_STACK_BASE,
		FLAG_PAGE_USER|FLAG_PAGE_WRITE|ATTR_PAGE_GNL);
	
	proc->threads->block_count = 0;
	s_cur_thread = proc->threads;
	return proc->threads;
}

// ����TSS��ص����� 
static void init_tss()
{
	set_gdt(INDEX_TSS_IN_GDT,&tss,sizeof(struct TSS),DA_386TSS|DA_DPL0);
	
	tss.backlink = 0;
	tss.ss0 = SELECTOR_KERNEL_DS;
	tss.esp0 = STACK_TOP;
	tss.iobase = sizeof(g_tss);
	ltr(SELECTOR_TSS);
}

void do_clock_int()
{
	do_sched_clock_int();
	eoi_m();
}

void init_process_ctrl()
{
	int i;
	
	init_tss();
	init_ipc();
	
	pcbn = kvirtual_alloc(NULL,MAX_PROC_COUNT * sizeof(struct process_t));
	s_free_procs = pcbn;
	for(i=0;i<MAX_PROC_COUNT - 1;i++)
	{
		pcbn[i].sibling = &pcbn[i+1];
	}
	pcbn[i].sibling = NULL;
	
	tcbn = kvirtual_alloc(NULL,MAX_THREAD_COUNT * sizeof(struct thread_t));
	s_free_threads = tcbn;
	for(i=0;i<MAX_THREAD_COUNT - 1;i++)
	{
		tcbn[i].sibling = &tcbn[i+1];
	}
	tcbn[i].sibling = NULL;
	
	extern void clock_int();
	set_8259a_idt(INT_CLOCK,clock_int);
	enable_irq(INT_CLOCK);
}

static int do_fork(struct thread_t *p_thread)
{
	struct process_t *c_proc = create_proc();
	
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
	to_ready(c_proc->threads - tcbn);
	
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
//			ret_val = do_fork();
		break;
		default:
		
		break;
		}
		if(INVALID_HMSG != hmsg)
			ipc_for_wait(hmsg,ret_val);
	}
}
