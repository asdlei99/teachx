
#ifndef _VASRM_H_
#define _VASRM_H_
// virtual address space region management
/*
��ģ�������������ķ��䡢�ͷźͱ�־���������Ա�־���н��͡� 
��ģ�����ӵ��һЩ��̬���������������create_vasr(...);���á� 
֮��ռ䲻��ʱ���Ե���kvirtual_alloc(...);����ȡ�ռ䣬�ú���������mmģ���С� 
*/

#include "type.h"

// ʹ����mallocͬ����˼����������ڴ� 
// ��ʹ�ÿ�[begin,end)
// ���п�  [end,end2) 
struct region_t{
	u32 flag;
	void *begin;
	void *end;
	void *end2; 

	struct region_t *prev;
	struct region_t *next;
};
typedef struct region_t* region_hdr_t;

void init_vasm();
region_hdr_t create_vasr(void *begin,void *end,void *end2);
region_hdr_t vasm_do_fork(region_hdr_t hdr);
void* alloc_region(region_hdr_t hdr,void *addr,size_t size,uint flag);
bool_t reset_region(region_hdr_t hdr,const void *addr,uint flag);
bool_t free_region(region_hdr_t hdr,void *addr);
uint region_flag(region_hdr_t hdr,const void *addr);

#endif
