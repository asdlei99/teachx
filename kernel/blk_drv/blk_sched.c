
#include "blk.h"
#include "process.h"
#include "_sys_call.h"

#define MAX_REQUEST	20

struct request_t{
	u32 sector;
	u8 nsect;
	u8 cmd;
	void *buf;
	int mid;
	struct request_t *prev;	// Ϊ�˵����㷨 
	struct request_t *next;
};

static struct request_t *s_cur_req = NULL;
static struct request_t *s_free_reqs;


static void hd_do_request()
{
	if(s_cur_req)switch(s_cur_req->cmd)
	{
	case SC_HD_READ:
		hd_read(s_cur_req->buf,s_cur_req->sector,s_cur_req->nsect);
	break;
	case SC_HD_WRITE:
		hd_write(s_cur_req->buf,s_cur_req->sector,s_cur_req->nsect);
	break;
	default:
		
	break;
	}
} 

// ��һ��������������������Ƴ� 
static void hd_do_next_req()
{
	#define INSIDE	0
	#define OUTSIDE	1
	static int direction = INSIDE;
	struct request_t *req = s_cur_req;
	
	if(req->next == s_cur_req){
		s_cur_req = NULL;
	}else{
		req->next->prev = req->prev;
		req->prev->next = req->next;
		s_cur_req = req->next;
		hd_do_request();
	}
	
	req->next = s_free_reqs;
	s_free_reqs = req;
}

// ��һ���������������������
// ��ʱ��ʹ�õ����㷨������FIFO 
static void req_insert(struct request_t *preq) 
{
	if(s_cur_req){
		preq->prev = s_cur_req->prev;
		preq->next = s_cur_req;
		
		s_cur_req->prev->next = preq;
		s_cur_req->prev = preq;
	}else{
		s_cur_req = preq;
		s_cur_req->prev = s_cur_req->next = s_cur_req;
		hd_do_request();
	}
}

#define REQUEST_COM		1001

void end_req(bool_t b)
{
	struct msg_t msg;
	
	msg.type = REQUEST_COM;
	msg.p0.iparam = s_cur_req->mid;
	msg.p1.iparam = b;
	_send_msg(PID_HD,&msg);
	
	hd_do_next_req();
}

void init_blk_sched()
{
	int i;
	
	s_free_reqs = (struct request_t*)kvirtual_alloc(NULL,_4K);
	for(i=0;i<MAX_REQUEST - 1;i++)
		s_free_reqs[i].next =  &s_free_reqs[i];
	s_free_reqs[i].next = NULL;
}

void hd_process()
{
	struct msg_t msg;
	int mid;
	
	init_blk_sched();
	
	while(1)
	{
		mid = ipc_recv(&msg);
		switch(msg.type)
		{
		case SC_HD_READ:
		case SC_HD_WRITE:
		{
			if(s_free_reqs == NULL){
				for_wait_msg(mid,FALSE);
			}else{
				struct request_t *request = s_free_reqs;
				
				s_free_reqs = s_free_reqs->next;
				
				request->sector = msg.p1.uiparam;
				request->nsect = msg.p2.uiparam;
				request->cmd = msg.type;
				request->buf = msg.p0.pparam;
				request->mid = mid;
				
				req_insert(request);
			}
		}
		break;
		case REQUEST_COM:
			if(msg.p0.iparam != INVALID_HMSG)
				for_wait_msg(msg.p0.iparam,msg.p1.iparam);
		break;
		default:
			
		break;
		}
	}
}

