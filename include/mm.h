
#ifndef _MM_H_
#define _MM_H_
/*
ҳ���4M�ռ�ȫ������ʹ��
����û���ȷʹ�ã������ú�Ȼ��ʹ�ã�
����û���������ڴ棬��ʹ��û�����úõı�����յ��´���
�ļ��������ڴ棩�ں˶�����Թ�������ӳ������򲻿��ԣ�
��Ϊÿ��������ܽ���ͬ���ļ��������ڴ棩ӳ�䵽��ͬ�������ڴ��ַ�С� 

�ļ�ӳ����ƣ� 
���ļ�
ӳ���ļ� ����ӳ���ں˶��� 
ӳ����ͼ ֱ��ӳ�䵽�ڴ�Ϳ����ˣ���ʹ��ָ��ӳ����� 
*/
#include "type.h"

u32 create_vas();

void do_mm_fork(int p_pid,int c_pid);

void* kvirtual_alloc(void *addr,size_t size);
bool_t kvirtual_reset(void *addr);
bool_t kvirtual_free(void *addr);

#endif
