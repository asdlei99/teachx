
#include "_sys_call.h"
#include "ipc.h"
#include "process.h"
#include "kernel.h"
#include "assert.h"

// ����struct _msg_t�ṹ��flag��
#define FLAG_MSG_USEING		1	// ����Ϣ�����ڱ�ʹ�� 
#define FLAG_MSG_COMPLETED	2	// �Ѵ������ 
#define FLAG_MSG_WAIT		4	// ���͸���Ϣ�Ľ���Ҫ�ȴ�����Ϣ�ķ��� 
#define FLAG_MSG_RETVAL		8	// �ȴ�����Ϣ���߳�Ҫ����ط���ֵ 
#define FLAG_MSG_WAITING	0X10// ���߳����ڵȴ�����Ϣ����� 

static struct _msg_t *msgn;
static struct _msg_t *s_free_msgs;

static struct _msg_t* alloc_msg_block()
{
	struct _msg_t *new_block = s_free_msgs;
	assert(new_block);
	new_block->flag = FLAG_MSG_USEING;
	s_free_msgs = s_free_msgs->next;
//  �����ô��룬������������� 
//	new_block->next = NULL;
	return new_block;
}

static void free_msg_block(struct _msg_t *free_msg)
{
//  �����ô��룬������������� 
	free_msg->flag = 0;
	free_msg->next = s_free_msgs;
	s_free_msgs = free_msg;
}

// ��������Ϣ�����Ŷ� 
int send_msg(uint pid_to,struct msg_t *pmsg,bool_t need_wait)
{
	struct process_t *to_proc = get_proc(pid_to);	
	struct thread_t *recv_tail = to_proc->ipc.recv_tail;
	
	pmsg->pid_from = get_cur_pid();

// �����ҵ����շ�����Ҫ��Ϣ��û����Ϣ�����������߳�
// �������ʹ��������̬��û��������
	if(recv_tail != NULL){
		struct thread_t *recv_hdr = recv_tail->ipc.next;
		if(recv_tail == recv_hdr){
			to_proc->ipc.recv_tail = NULL;
		}else{
			recv_tail->ipc.next = recv_hdr->ipc.next;
		}
		set_retval_ex(recv_hdr,-2);	// ���� -2 ��ʾ���߳�֮ǰ��û����Ϣ��������
		to_ready_ex(recv_hdr);		// ����������Ϣ�ˣ�����Ҫ�ٴν��� 
	}

// ��������ʼ����Ϣ�� 
	struct _msg_t *new_msg = alloc_msg_block();
	
	if(need_wait)
		new_msg->flag |= FLAG_MSG_WAIT;
	
	new_msg->msg = *pmsg;

// ����Ϣ����ָ�����̵���Ϣ�Ŷ� 
	if(to_proc->ipc.msg_tail){
		new_msg->next = to_proc->ipc.msg_tail->next;
		to_proc->ipc.msg_tail->next = new_msg;
	}else{
		new_msg->next = new_msg;
	}
	to_proc->ipc.msg_tail = new_msg;

// ���ظ���Ϣid -- mid :) 
	return new_msg - msgn;
}

void _send_msg(uint pid,struct msg_t *pmsg)
{
	send_msg(pid,pmsg,FALSE);
}

void wait_msg(int mid,bool_t need_retval)
{
	assert(mid != INVALID_HMSG);
	if(msgn[mid].flag & FLAG_MSG_COMPLETED){
		if(need_retval)
			set_retval(get_cur_tid(),msgn[mid].retval);
		free_msg_block(&msgn[mid]);
	}else{
		if(need_retval)
			msgn[mid].flag |= FLAG_MSG_RETVAL;
		msgn[mid].tid_wait = get_cur_tid();
		to_block(msgn[mid].tid_wait);
		msgn[mid].flag |= FLAG_MSG_WAITING;
	}
}

void _send_wait_msg(uint pid,struct msg_t *pmsg)
{
	int mid = send_msg(pid,pmsg,TRUE);
	wait_msg(mid,FALSE);
}

void for_wait_msg(int mid,int retval)
{
	assert(mid != INVALID_HMSG);
	if(msgn[mid].flag & FLAG_MSG_WAITING){
		if(msgn[mid].flag & FLAG_MSG_RETVAL)
			set_retval(msgn[mid].tid_wait,retval);
		to_ready(msgn[mid].tid_wait);
	
		free_msg_block(&msgn[mid]);
	}else{
		msgn[mid].flag |= FLAG_MSG_COMPLETED;
		msgn[mid].retval = retval;
	}
}

void recv_msg(struct msg_t *pmsg)
{
	struct process_t *cur_proc = get_cur_proc();
	struct thread_t *cur_thread = get_cur_thread();
	struct _msg_t *msg_tail = cur_proc->ipc.msg_tail;

	if(msg_tail){
		struct _msg_t *msg_head = msg_tail->next;
		
		assert(msg_head);
		
		*pmsg = msg_head->msg;
		
		if(msg_tail == msg_head){
			cur_proc->ipc.msg_tail = NULL;
		}else{
			msg_tail->next = msg_head->next;
		}
		
		if(msg_head->flag & FLAG_MSG_WAIT)
			set_retval_ex(cur_thread,msg_head - msgn);
		else{
			set_retval_ex(cur_thread,INVALID_HMSG);
			free_msg_block(msg_head);
		}
	}else{
		to_block_ex(cur_thread);
		if(cur_proc->ipc.recv_tail){
			cur_thread->ipc.next = cur_proc->ipc.recv_tail->ipc.next;
			cur_proc->ipc.recv_tail->ipc.next = cur_thread;
		}else{
			cur_thread->ipc.next = cur_thread;
		}
		cur_proc->ipc.recv_tail = cur_thread;
	}
}

/*
	RECV��
		����ֵ�� 
			������͸���Ϣ�Ľ�����Ҫ����ֵ���򷵻ظ���Ϣ�ľ��
			�������Ϣ����Ҫ����ֵ���򷵻�-1 
*/
int do_ipc()
{	
	struct thread_t *thread = get_cur_thread();
	uint func = thread->context.pushad.ebx;
	uint pid_to_or_hmsg = thread->context.pushad.ecx;
	struct msg_t *pmsg = (struct msg_t*)thread->context.pushad.edx;
	
// �����ô��룬������������� 
	

	switch(func)
	{
	case IPC_RECV:
		recv_msg(pmsg);
	break;
	case IPC_SEND:
		send_msg(pid_to_or_hmsg,pmsg,FALSE);
		set_retval(get_cur_tid(),INVALID_HMSG);
	break;
	case IPC_SEND | IPC_ATTR_WAIT:
	{
		int mid;
		mid = send_msg(pid_to_or_hmsg,pmsg,TRUE);
		set_retval(get_cur_tid(),mid);
	}
	break;
	case IPC_WAIT:
		wait_msg(pid_to_or_hmsg,TRUE);
	break;
	case IPC_SEND_WAIT:
		pid_to_or_hmsg = send_msg(pid_to_or_hmsg,pmsg,TRUE);
		wait_msg(pid_to_or_hmsg,TRUE);
	break;
	case IPC_FOR_WAIT:
		if(msgn[pid_to_or_hmsg].flag == 0)
			printf("!!!!!%d",pid_to_or_hmsg);
		for_wait_msg(pid_to_or_hmsg,pmsg->p0.iparam);
	break;
	default:
		assert(!"not be here");
	break;
	}
}

void init_ipc()
{
	int i;
	
	msgn = (struct _msg_t*)kvirtual_alloc(NULL,_4K);
	printf("msg_max_count:%d",_4K/sizeof(struct _msg_t));
	for(i = 0;i < _4K/sizeof(struct _msg_t) - 1;i++)
	{
		msgn[i].next = &msgn[i+1];
	}
	msgn[i].next = NULL;
	
	s_free_msgs = msgn;
	
	// ������ ipc.asm �� 
	int ipc_int();
	_set_gate(&idt[0x80],DA_DPL3|DA_386IGate,ipc_int,SELECTOR_KERNEL_CS);
}
