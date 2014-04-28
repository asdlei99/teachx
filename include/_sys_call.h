
#ifndef __SYS_CALL_
#define __SYS_CALL_
/*
   �������������ϵͳ���õ����� 
   ���ļ������ý�����Ϊ��һ���ԣ���ϵͳ������ӿ�֮��
   Ȼ�����������ļ��е����ݽ�ͳͳ���� 
*/


#define IPC_RECV		0X01	// ������Ϣ
#define IPC_SEND		0X02	// ������Ϣ 
#define IPC_WAIT		0X03	// �ȴ�
#define IPC_SEND_WAIT	0X04	// ������Ϣ�����ȴ�����Ϣ���� 
#define IPC_FOR_WAIT	0X05	// 

#define IPC_ATTR_WAIT	0X10	// ֻ������ IPC_WAIT ���� 


#define PID_MM	01	// Memory Manage
#define PID_IO	02	// Input Output
#define PID_HD	03	// Hard Disk
#define PID_PM	04	// Process Manage
#define PID_CLK	04	// Clock

// ģ���ڲ�ʹ�õ���Ϣ����ֵ������ڴ�ֵ 
// ģ����⿪�ŵ���Ϣ����ֵ����С�ڴ�ֵ 
#define SC_USER					1000 

// ���̹�����̵���Ϣ���� 
#define SC_PTHREAD_KEY_CREATE	01
#define SC_PTHREAD_KEY_DELETE	02
#define SC_FORK					03

#define SC_CLOSE				01
#define SC_EOF					02
#define SC_FILELENGTH			03
#define SC_LSEEK				04
#define SC_READ					05
#define SC_TELL					06
#define SC_WRITE				07


#define SC_HD_READ				01
#define SC_HD_WRITE				02


#define SC_CLK_CLOCK			01
#define SC_CLK_INIT				02
#define SC_CLK_SLEEP			03
#define SC_CLK_TIME				04


#endif
