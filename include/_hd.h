
#ifndef _HD_H_
#define _HD_H_
#include "type.h"

// ������Ϣ��� 
int _hd_read_msg(void *buf,u32 sector,u8 nsect);
int _hd_write_msg(const void *buf,u32 sector,u8 nsect);
// ���ط���ֵ 
bool_t hd_read_msg(void *buf,u32 sector,u8 nsect);
bool_t hd_write_msg(const void *buf,u32 sector,u8 nsect);

#endif
