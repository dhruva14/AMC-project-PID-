#include "lpc17xx.h"

void MCPWMInit(void)
{
	LPC_SC->PCONP |= (1<<17);        // Enable Motor Control PWM
	LPC_SC->PCLKSEL1 &= 0x3fffffff;  // PCLK for MCPWM as CCLK / 4;
	LPC_PINCON->PINSEL3 = 0x00045540; // Enable mcoa0, b0, mc0, 1, 2, and mcoabort
	
}

void MCPWMConfig(int period, int duty) // duty should lie between -50 and 50
{
	LPC_MCPWM->MCCON_SET = 0;
	LPC_MCPWM->MCPER0 = period;
	LPC_MCPWM->MCPW0 = ((duty + 50) * period )/ 100;   // mcob0 is on from  0 to duty 
}

void MCPWMStart(void)
{
	LPC_MCPWM->MCCON_SET = 1;
}

void MCPWMDisable(void)
{
	MCPWMConfig(100, 0);
	LPC_SC->PCONP &= ~(1<<17);
}

