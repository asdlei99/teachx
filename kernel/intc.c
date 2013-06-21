
/*
�жϿ����� 
*/
#include "asm.h"
#include "intc.h"

/*�жϿ�������*/ 
#define ICW1	0X11	/*�����ش�����������Ҫд��ICW4*/
#define ICW2_M	INT_VECTOR_IRQ
#define ICW2_S	INT_VECTOR_IRQ + 8
#define ICW3_M	0X4		/*IR2Ϊ������*/
#define ICW3_S	0X2
#define ICW4	0X1		/*���Զ������������߲�����*/ 

void init_8259A()
{
	out(INT_M_CTRL,ICW1);
	out(INT_S_CTRL,ICW1);
	out(INT_M_CTRLMASK,ICW2_M);
	out(INT_S_CTRLMASK,ICW2_S);
	out(INT_M_CTRLMASK,ICW3_M);
	out(INT_S_CTRLMASK,ICW3_S);
	out(INT_M_CTRLMASK,ICW4);
	out(INT_S_CTRLMASK,ICW4);
	out(INT_M_CTRLMASK,0XFF);	/*���������ж�*/ 
	out(INT_S_CTRLMASK,0XFF);
}
