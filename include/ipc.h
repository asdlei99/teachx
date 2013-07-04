
#ifndef _IPC_H_
#define _IPC_H_
#include "_process.h" 

struct _msg_t{
	int tid_wait;			// ֻ��
	uint flag;
	int retval;
	struct msg_t msg;
	struct _msg_t *next;
};

void init_ipc();
// Ϊ�ж������ṩ�Ľӿ�
// ֮������һ����Ϣ���Ҳ��ȴ����� 
void _send_msg(uint pid,struct msg_t*);

// ֮������һ����Ϣ�����ȴ����������ط���ֵ 
void _send_wait_msg(uint pid,struct msg_t*);

// �����жϵ�ipc�����������ǲ�������̶߳��� 
void for_wait_msg(int mid,int retval); 

#endif
