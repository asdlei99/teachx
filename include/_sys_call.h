
#ifndef __SYS_CALL_
#define __SYS_CALL_
/*
   �������������ϵͳ���õ����� 
   ���ļ������ý�����Ϊ��һ���ԣ���ϵͳ������ӿ�֮��
   Ȼ�����������ļ��е����ݽ�ͳͳ���� 
*/

#define PID_MM	01	// Memory Manage
#define PID_IO	02	// Input Output
#define PID_PM	03	// Process Manage
#define PID_HD	04	// Hard Disk

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

#endif
