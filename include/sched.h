
#ifndef _SCHED_H_
#define _SCHED_H_
#include "type.h"

// ʹָ���߳̿ɱ����� 
void sched_insert(int tid);
// ʹָ���̲߳��ɱ����� 
void sched_erase(int tid);
// ���жϷ��ص�����ʱ�����ã�Ƕ�׵��жϷ��ز�����øú��� 
void do_iret();

#endif
