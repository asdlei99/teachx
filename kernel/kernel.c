
#include "init.h"
#include "kernel.h"
#include "asm.h"
#include "stdio.h"
#include "setup.h"
#include "vga.h"

// ���ܣ�
// ��֤�κ�ʱ��ϵͳ�ж�������һ���Ƕ������� 
// ��CPU��Ϣ��Ϣ 
static void sys_idle_proc()
{
	int i,j;
	
	sti();
	
	for(i=0;;i++)
	{
		for(j=0;j<1000000;j++)
			nop();
		printf("%d ",i);
		for(j=0;j<1000000;j++)
			nop();
	}
	while(1)
		hlt();
}

static void init_proc()
{
//	create_sys_proc(mm_process);
//	create_sys_proc(hd_process);
	create_sys_proc(sys_idle_proc);
}

void kernel_start()
{
	struct setup_info_t system_info = *g_sys_info;
	u32 flags;
	
	init_trap();
	init_8259A();
	//	printf("first:%x,last:%x\n",g_sys_info->first_page_addr,g_sys_info->last_page_addr);
	init_mm(g_sys_info->first_page_addr,g_sys_info->last_page_addr);
	printf("end of mm\nstart keyboard\n");
	init_keyboard();
	printf("end of keyboard\nstart process\n");
	init_process_ctrl();
	printf("end of process\nstart proc\n");
	
	flags = sflags();
	flags |= 0x1000;
	lflags(flags);
	
	init_proc();
	printf("end of proc\n");
	
	// �Ӵ�ϵͳ��ʼ����ϣ�ʱ���жϺ󼴿ɽ���ϵͳ���̡� 
	sti();
}
