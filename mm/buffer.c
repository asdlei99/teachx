
#include "kernel.h"

#define VIEW_PAGES ((VIEW_BUFFER_END-VIEW_BUFFER_BEGIN)>>12)
#define VIEW_SIZE	(_1K * 512)		// һ����ͼ��Ĵ�С 
#define VIEW_PAGES	(VIEW_SIZE/_4K)	// һ����ͼ���ҳ���� 

typedef u8 view_buf_t[VIEW_SIZE];

#define view_bufs ((view_buf_t*)VIEW_BUFFER_BEGIN)
#define vcb_head

struct vcb_t{	//view control block
	u32 offset; //or sector
	size_t size;
	struct vcb_t *prev;
	struct vcb_t *next;
};

// ���ؿ��ƿ��Ӧ����ͼ 
u8* get_buf(struct vcb_t *vcb)
{
	return view_bufs + (vcb_head - vcb);
}

void init_buffer()
{
	
	VIEW_PAGES
}
