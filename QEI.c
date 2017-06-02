#include "lpc17xx.h"
#include "util.h"

void (*cb_timer) (void);
void (*cb_dirn) (void);

int tim_millisec = 250, Edges = 4, PPR = 20, RPM = 0, interrupt = 2, config = 4; 
	
void QEI_Init(void (*CB_on_timer_ellapse) (void), void (*CB_on_dirn_change) (void))
{
	cb_timer = CB_on_timer_ellapse;
	cb_dirn = CB_on_dirn_change;
	
	LPC_SC->PCONP |= (0x1 << 18);	 // 3 MHz as PCLK
	LPC_SC->PCLKSEL1 |= 0x1;
	LPC_SC->PCLKSEL1 &= ~(0x2);
	
	LPC_PINCON->PINSEL3 |= (1<<8) + (1<<14) + (1<<16);       	// Using MCI0, 1, 2
	LPC_PINCON->PINSEL3 &= ~((1<<9) + (1<<15) + (1<<17));			
	
	LPC_QEI->FILTER = 3*500; 									// pulse must remain for at least 500us to be recognized
	
	LPC_QEI->QEILOAD = 3000 * tim_millisec;	// timer reloads every tim_millisec ms
	LPC_QEI->QEICONF = config;
	LPC_QEI->QEIIES = interrupt; 									
	
	NVIC_EnableIRQ(QEI_IRQn);
}

void QEI_IRQHandler(void)
{
	uint32_t Status;
	Status = LPC_QEI->QEIINTSTAT;
	
	if(Status & 2)
	{
		RPM = (1000 * LPC_QEI->QEICAP * 60) / (tim_millisec * PPR * Edges);
		LPC_QEI->QEICLR = 2;
		
		LPC_QEI->QEICON = 4;
		cb_timer();
	}
	else if(Status & 8)
	{
		LPC_QEI->QEICLR = 8;
		cb_dirn();
	}	
}

int QEI_get_RPM(void)
{
	return RPM;
}

void QEI_set_PPR(int ppr_val)
{
	PPR = ppr_val;
}

void QEI_set_VelTimer(uint32_t tim_ms)
{
	tim_millisec = tim_ms;
	
	LPC_QEI->QEIIES = 0;
	LPC_QEI->QEILOAD = 3000*tim_millisec;
	LPC_QEI->QEIIES = 2;
}

void QEI_set_timer_callback(void (*CB_on_timer_ellapse) (void))
{
	LPC_QEI->QEIIES = 0;
	if(CB_on_timer_ellapse != 0)
		cb_timer = CB_on_timer_ellapse;
	LPC_QEI->QEIIES = interrupt;
}

void QEI_set_dirn_callback(void (*CB_on_dirn_change) (void))
{
	LPC_QEI->QEIIES = 0;
	if(CB_on_dirn_change != 0)
		cb_dirn = CB_on_dirn_change;
	LPC_QEI->QEIIES = interrupt;
}

void QEI_set_config(int config_val)
{
	LPC_QEI->QEIIES = 0;
	config = config_val;
	LPC_QEI->QEIIES = interrupt;
}

void QEI_select_interrupts(int int_val)
{
	LPC_QEI->QEIIES = 0;
	interrupt = int_val;
	LPC_QEI->QEIIES = interrupt;
}

void QEI_disable(void)
{
	QEI_select_interrupts(0);
	LPC_SC->PCONP &= ~(0x1 << 18);		
}
