
#include "sched.h"
#include "assert.h"

// ���ɵ����߳���֯��˫ָ�����ѭ������ 
// ���� idle_head Ϊ��ͷ����ָ����н��̵��߳� 
static struct thread_t *idle_head;
static struct thread_t *next_thread;


static void schedule()
{
	next_thread = next_thread->sched.next;
	if(next_thread == idle_head)
		next_thread = next_thread->sched.next;
}

void sched_init(struct thread_t *idle_thread)
{
	idle_head = idle_thread;
	next_thread = idle_thread;
	idle_head->sched.prev = idle_head;
	idle_head->sched.next = idle_head;
	idle_head->sched.life = 0;
} 

void sched_insert(struct thread_t *thread)
{
	thread->sched.prev = idle_head->sched.prev;
	thread->sched.next = idle_head;
	idle_head->sched.prev->sched.next = thread;
	idle_head->sched.prev = thread;
	thread->sched.life = MAX_THREAD_LIFE;
}

void sched_erase(struct thread_t *thread)
{
	// ���Ⱥ�����Ҫ next_thread ����˱�����ɾ���ýڵ�֮ǰ���á� 
	if(next_thread == thread){
		schedule();
		if(next_thread == thread)
			next_thread = idle_head;
	}
	
	thread->sched.prev->sched.next = thread->sched.next;
	thread->sched.next->sched.prev = thread->sched.prev;
}

struct thread_t* do_iret()
{	
	if(next_thread->sched.life <= 0){
		next_thread->sched.life += MAX_THREAD_LIFE;
		schedule();
	}
	
	return next_thread;
}

void do_sched_clock_int()
{
	next_thread->sched.life --;
}
