
#ifndef __MM_H_
#define __MM_H_
#include "type.h"

// û�ж��� FLAG_PAGE_READ ��������Ϊ������û���Ϊ��������ǲ����øñ�־���Ǹ����ξͲ��ɶ���
// Ȼ�����Ǵ������Ϊ�� 
#define FLAG_PAGE_WRITE		0X02
#define FLAG_PAGE_USER		0X04
#define ATTR_PAGE_MASK		0XF000
#define ATTR_PAGE_UNKNOW	0X0000		// δ֪ 
#define ATTR_PAGE_GNL		0X1000		// �ύ��ͨҳ�� 
#define ATTR_PAGE_ZERO		0x2000		// �ύ���������ҳ�� 
#define ATTR_PAGE_FMAP		0x3000		// �ļ�ӳ�� 

void* virtual_alloc(void *addr,size_t size,uint flag);
bool_t virtual_reset(void *addr,uint flag);
bool_t virtual_free(void *addr);
bool_t mapview_file(void *addr,handle_t handle,int offset);

#endif
