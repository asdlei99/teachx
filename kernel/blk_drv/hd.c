
/*
win��������˹��(Winchester)Ӳ�� 
*/
#include "intc.h"
#include "setup.h"
#include "asm.h"
#include "vga.h"
#include "process.h"
#include "_sys_call.h"

#define PORT_HD_DATA	0X1F0	//���ݼĴ��� 
#define PORT_HD_ERROR	0X1F1	//����Ĵ���
#define PORT_HD_PRECOMP	0X1F1 
#define PORT_HD_NSECTOR	0X1F2	//�������Ĵ��������һ�������Ĳ����üĴ������Զ���һ 
#define PORT_HD_SECTOR	0X1F3	//�����żĴ��� 
#define PORT_HD_LCYL	0X1F4	//����żĴ���LOW 
#define PORT_HD_HCYL	0X1F5	//����żĴ���HIGH
#define PORT_HD_CURRENT	0X1F6	// 
#define PORT_HD_STATUS	0X1F7
#define PORT_HD_COMMAND	0X1F7 
#define PORT_HD_CMD		0X3F6	

#define WIN_RESTORE	0X10	//Ĭ�ϲ������� 35us 
#define WIN_READ	0X20	//����ECC�룬��������
#define WIN_WRITE	0X30	//����ECC�룬��������
#define WIN_VERIFY	0X40
#define WIN_FORMAT	0X50	//��ʽ���ŵ���
#define WIN_INIT	0X60	//��������ʼ��
#define WIN_SEEK	0X70
#define WIN_DIAGNOSE 0X90	//������������� 
#define WIN_SPECIFY	0X91 

#define HD_STATUS_ERROR	0X01
#define HD_STATUS_INDEX 0X02
#define HD_STATUS_ECC	0X04
#define HD_STATUS_DRQ	0X08
#define HD_STATUS_SEEK	0X10
#define HD_STATUS_WRERR	0X20
#define HD_STATUS_READY	0X40
#define HD_STATUS_BUSY	0X80
#define HD_STATUS_OK	(HD_STATUS_READY|HD_STATUS_SEEK)

static u8  s_ctl;
static u8  s_pcom;
static u32 s_sec_per_trk;
static u32 s_heads;

static void *s_cur_buf;

void hd_bad_int();				//������жϵ��� 
void (*do_hd)() = hd_bad_int;
void (*do_floppy) = hd_bad_int;	//��ʱ��������һ�¡� 


static u32 hd_ctrl_ready()
{
	u32 retries=10000;

	while (--retries && (in_p(PORT_HD_STATUS)&HD_STATUS_BUSY))
		;	//���������д��nop 
	return (retries);
}

//FALSE ��ʾ ����������
//TRUE  ��ʾ ������æ
static bool_t hd_ctrl_busy()
{
	u32 i;
	for(i=0;i<10000;i++)
		if(HD_STATUS_OK == (in_p(PORT_HD_STATUS)&(HD_STATUS_BUSY|HD_STATUS_OK)))
			return FALSE;
	return TRUE;
}

//����ֵ��
//0��ʾ������1��ʾ���� 
static u32 win_result()
{
	const u8 status = in(PORT_HD_STATUS);
	
	if((status&(HD_STATUS_BUSY|HD_STATUS_READY|HD_STATUS_WRERR|	
		HD_STATUS_SEEK|HD_STATUS_ERROR))==(HD_STATUS_SEEK|HD_STATUS_READY)){
		
		return 0;
	}
	
	if(status & HD_STATUS_ERROR)
		in(PORT_HD_ERROR);
	
	return 1;
}

//hard disk out
static void hd_out(u32 sector,u8 nsect,u8 cmd)
{
	u32 trk = sector / s_sec_per_trk;
	u32 cyl = trk / s_heads;
	
	if(!hd_ctrl_ready()){
		end_req(FALSE);
	}
	
	out_p(PORT_HD_CMD,s_ctl);
	out_p(PORT_HD_PRECOMP,s_pcom);
	out_p(PORT_HD_NSECTOR,nsect);
	out_p(PORT_HD_SECTOR,(sector % s_sec_per_trk) + 1);
	out_p(PORT_HD_LCYL,cyl&0xff);
	out_p(PORT_HD_HCYL,(cyl>>8)&0xff);
	out_p(PORT_HD_CURRENT,0xa0 | (trk % s_heads));	//��0�������� 
	out_p(PORT_HD_COMMAND,cmd);
}

void hd_bad_int()
{
	
}

static void hd_read_int()
{
	port_readw(PORT_HD_DATA,s_cur_buf,256);

	if(in(PORT_HD_NSECTOR)){	//ʣ�������� 
		s_cur_buf += 512;
	}else{
		end_req(TRUE); 
	}
}

static void hd_write_int()
{
	if(in(PORT_HD_NSECTOR)){	//ʣ�������� 
		s_cur_buf += 512;
		port_writew(PORT_HD_DATA,s_cur_buf,256);
	}else{
		end_req(TRUE); 
	}
}

void do_hd_int()
{
	if(!win_result()){
		do_hd();
	}else{
		end_req(FALSE);
	}

	eoi_m();
	eoi_s();
}

void hd_read(void *buf,u32 sector,u8 nsect)
{
	s_cur_buf = buf;
	do_hd = hd_read_int;
	hd_out(sector,nsect,WIN_READ);
}

//��Ӳ�̷��Ͷ�ȡ���� 
void hd_write(void *buf,u32 sector,u8 nsect)
{
	u32 i;
	u8 r;
	
	s_cur_buf = buf;
	do_hd = hd_write_int;
	hd_out(sector,nsect,WIN_WRITE);
	for(i=0;i<3000 && !(r=in_p(PORT_HD_STATUS)&HD_STATUS_DRQ);i++)
		;
	if(!r)
		end_req(FALSE);
	else
		port_writew(PORT_HD_DATA,s_cur_buf,256);
}

void init_hd()
{
	s_ctl = g_sys_info->hd0[8];
	s_pcom = (g_sys_info->hd0[6]<<6) | (g_sys_info->hd0[5]>>2);
	s_sec_per_trk = g_sys_info->hd0[0x0e];
	s_heads = g_sys_info->hd0[2];
	
	extern void hd_int();	// ������ fdhd.asm �� 
	set_8259a_idt(INT_AT,hd_int);
	enable_irq(2);		//�򿪼����ж� 
	enable_irq(INT_AT);
}
