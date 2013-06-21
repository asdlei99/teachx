
#ifndef _ERRNO_H_
#define _ERRNO_H_
#include "type.h"

void panic(u32 error_code,u32 param);

//����һ�����̰߳󶨵Ĵ������ 
u32* _errno();
#define errno (*_errno())


/* ���������ں˴������ */

//The operation completed successfully
#define ERROR_SUCCESS	0

//Unable to send byte to FDC
#define ERROR_FDC		1

//ָ���ڴ��޷���ȡ 
#define ERROR_MEM_READ	2

//ָ���ڴ��޷�д�� 
#define ERROR_MEM_WRITE	3 

//û���㹻�ľ���ռ���
#define	EMFILE		24	/* Too many open files */

//��Ч���ֵ
#define	EBADF		9	/* Bad file descriptor */

#endif
