
#include "ipc.h"
#include "process.h"

// ����struct _msg_t�ṹ��flag��
#define FLAG_MSG_USEING		1	// ����Ϣ�����ڱ�ʹ�� 
#define FLAG_MSG_COMPLETED	2	// �Ѵ������ 
#define FLAG_MSG_WAIT		4	// ���͸���Ϣ�Ľ���Ҫ�ȴ�����Ϣ�ķ��� 
#define FLAG_MSG_RETVAL		8	// �ȴ�����Ϣ���߳�Ҫ����ط���ֵ 
#define FLAG_MSG_WAITING	0X10// ���߳����ڵȴ�����Ϣ����� 

struct _msg_t *msgn;

#define SIZE_MSG_BUFFER		_4K
#define COUNT_MSG			(SIZE_MSG_BUFFER/sizeof(struct _msg_t))

static struct _msg_t *s_free_msgs;

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
	struct _msg_t *new_msg = s_free_msgs;
	s_free_msgs = s_free_msgs->next;
	
	new_msg->flag = FLAG_MSG_USEING;
	if(need_wait)
		new_msg->flag |= FLAG_MSG_WAIT;
	new_msg->msg = *pmsg;

// ����Ϣ����ָ�����̵���Ϣ�Ŷ� 
	if(to_proc->ipc.msg_tail){
		new_msg->next = to_proc->ipc.msg_tail;
		to_proc->ipc.msg_tail->next = new_msg;
	}else{
		new_msg->next = new_msg;
	}
	to_proc->ipc.msg_tail = new_msg;

// ���ظ���Ϣid -- mid :) 
	return need_wait? new_msg - msgn : INVALID_HMSG;
}

void wait_msg(int mid,bool_t need_retval)
{
	if(msgn[mid].flag & FLAG_MSG_COMPLETED){
		msgn[mid].flag = 0;
		msgn[mid].next = s_free_msgs;
		s_free_msgs = &msgn[mid];
		if(need_retval)
			set_retval(get_cur_tid(),msgn[mid].retval);
	}else{
		if(need_retval)
			msgn[mid].flag |= FLAG_MSG_RETVAL;
		msgn[mid].tid_wait = get_cur_tid();
		to_block(msgn[mid].tid_wait);
	}
}

void for_wait_msg(int mid,int retval)
{
	if(msgn[mid].flag & FLAG_MSG_WAITING){
		if(msgn[mid].flag & FLAG_MSG_RETVAL)
			set_retval(msgn[mid].tid_wait,retval);
		to_ready(msgn[mid].tid_wait);
		
		msgn[mid].flag = 0;
		msgn[mid].next = s_free_msgs;
		s_free_msgs = &msgn[mid];
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
		
		*pmsg = msg_head->msg;
		if(msg_head->flag & FLAG_MSG_WAIT)
			set_retval_ex(cur_thread,msg_head - msgn);
		else
			set_retval_ex(cur_thread,INVALID_HMSG);

		if(msg_tail == msg_head){
			cur_proc->ipc.msg_tail = NULL;
		}else{
			msg_tail->next = msg_head->next;
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
