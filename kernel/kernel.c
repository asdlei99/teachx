
#include "init.h"
#include "kernel.h"
#include "asm.h"
#include "stdio.h"
#include "setup.h"

// ���ܣ�
// ��֤�κ�ʱ��ϵͳ�ж�������һ���Ƕ������� 
// ��CPU��Ϣ��Ϣ 
static void sys_idle_proc()
{
	while(1)
		hlt();
}

static void init_proc()
{
	create_sys_proc(mm_process);
	create_sys_proc(hd_process);
	create_sys_proc(sys_idle_proc);
}

void kernel_start()
{
	struct setup_info_t system_info = *g_sys_info;
	
	init_trap();
	init_8259A();
	sti();
	//	printf("first:%x,last:%x\n",g_sys_info->first_page_addr,g_sys_info->last_page_addr);
	init_mm(g_sys_info->first_page_addr,g_sys_info->last_page_addr);
	init_keyboard();
	init_process_ctrl();
	init_proc();
	schedule();
}
