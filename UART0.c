#include<LPC17xx.h>
#include "UART0.h"
#include<stdio.h>
#include<string.h>

#define BFSZ 128 												// Buffer Size

extern unsigned char recv_data, recv_index;

unsigned char rx0_flag=0, tx0_flag=0, tbuf_flag = 0;		//rx0_flag, tx0_flag are set when data is received or transmitted
																												//tbuf_flag is 0xff when the transimitter is busy and 0 if it is slack
char tbuf[BFSZ] = {0};									//Transmit buffer. Implemented to act as a circular buffer
char rbuf[BFSZ] = {0};									//Receive buffer. Implemented to act as a circular buffer 

int th = 1, tl = 0, rh = 0, rl = 0, rf = 0; 		//Indices pointing to head and tail of tbuf and rbuf. 
																								// rf points to last filled location until it is modified by WaitForChar()
																								// after which it points to rh (so that the same location won't be checked
																								// by WaitForChar() once more when it is called again

void UART0_Init(void)
{
	LPC_SC->PCONP |= 0x00000008;					//UART0 peripheral enable
	LPC_PINCON->PINSEL0 &= ~0x000000F0;
	LPC_PINCON->PINSEL0 |= 0x00000050;
	LPC_UART0->LCR = 0x00000083;					//enable divisor latch, parity disable, 1 stop bit, 8bit word length
	LPC_UART0->DLM = 0x00; 
  LPC_UART0->DLL = 0x13;      					//select baud rate 9600 bps
	LPC_UART0->LCR = 0x00000003;
	LPC_UART0->FCR = 0x07;
	LPC_UART0->IER = 0X03;	   						//select Transmit interrupt

	NVIC_EnableIRQ(UART0_IRQn);						//Assigning channel
}

void UART0_RecvDisable(void)
{
	LPC_UART0->IER = 2;
}

void UART0_RecvEnable(void)
{
	LPC_UART0->IER = 3;
}

signed char UART0_Send(char * transData)
{
		int n, k = strlen(transData);
	
		if(transData == 0)									//Check if input string is not null
			return -3;
		
		if(k >= BFSZ)												//Check if input string is too large for the buffer 
			return -2;
		
		if(tbuf[(th + k + 1) % BFSZ] != 0)	//Check if the buffer can store input without overwriting untransmitted data 
			return -1;		
		
		for(n = 0; n < k; n++)							//Load string onto the buffer and update the 'head' of the buffer
		{
			tbuf[th] = transData[n];
			th = (th + 1) % BFSZ;
		}
		
		if(tbuf_flag == 0)										//If nothing is being transmitted, start transmission, return 2
		{
			tl = (tl + 1) % BFSZ;					
			LPC_UART0->SCR = tbuf[tl];
			tbuf[tl] = 0;
			tbuf_flag = 0xff;
			LPC_UART0->THR = LPC_UART0->SCR;
			
			return 2;
		}
		
		return 1;														//If transmission is currently in progress, return 1
}

void UART0_IRQHandler(void)
{
	unsigned long Int_Stat;
	Int_Stat = LPC_UART0->IIR;						//Read data from interrupt identification register
	
	if((Int_Stat & 0x02)== 0x02)					//transmit interrupt
	{	
		tx0_flag = 0xff;
		
		if((tl + 1) % BFSZ != th)						//If more data is to be transmitted, set flag, update 'tail', and transmit the data
		{
			tl = (tl + 1) % BFSZ;			
			LPC_UART0->SCR = tbuf[tl];					
			tbuf[tl] = 0;							 
			tbuf_flag = 0xff;
			LPC_UART0->THR = LPC_UART0->SCR;
		}
		else																//else, all data has been transmitted. Reset tbuf_flag to 0
		{
			tbuf_flag = 0;
		}
	}
	else if((Int_Stat & 0x04) == 0x04)  	//Receive interrupt
	{
		rbuf[rh] = LPC_UART0->RBR;					//Load data onto rbuf, update head index
		rf = rh;		
		rh = (rh + 1) % BFSZ;								
		rx0_flag = 0xff;										//Set rx0_flag 
	}
}

unsigned char UART0_TransmitStatus(void)
{
	if(tbuf_flag == 0xff)									//If currently transmitting
	{
		if((tl + 1) % BFSZ == th)						//Return 2 if this is the last byte to be transfered
			return 2;
		
		return 1;														//Return 1 if the buffer has multiple bytes yet to be transmitted
	}
	return 0;															//Return 0 if not currently transmitting
}

void UART0_Configure(unsigned long int speed, enum SerialConfig configure)    //Configures UART0 for the given speed (baud rate) and data frame
{
	float FRe, q, err, min = 2; 
	unsigned long int DIV = 0x13;
	unsigned int BITS = 0 , NUM = 0, DEN = 1;;
	
	BITS = (configure / 6) + ((configure % 2) << 2);		//Find bits to be written to LCR for the given data frame configuration
	
	switch( (configure / 2) % 3)
	{
		case 1: BITS += 0x08;
						break;
		case 2: BITS += 0x18;
						break;
	}
	
	LPC_UART0->LCR = (LPC_UART0->LCR & (0xd0)) + BITS;	//Write the bits to LCR
	LPC_UART0->LCR |= 0x80;															//Set divisor latch enable 
	
	if(speed < 100 || speed > 62500)					//If the given speed is beyond the limits, use a default baud rate of 9600
		speed = 9600;
	
	
	/* Procedure, as mentioned in the User Manual, to find the values to be loaded onto the Divisor Latch for the given baud rate */
	
	if((long)((3000000.0/(16 * speed)) + 0.5) == (3000000.0/(16 * speed)))		//If the speed can be set without using the fractional divider
	{
		DIV = 3000000/(16 * speed);
	}
	else																											//else, find values of DLL, DLH, DIVADDVAL and MULVAL
	{
		for(q = 1.1; q < 2; q += 0.1)
		{
			DIV = (unsigned long) (3000000/(16 * speed * q));
			FRe = 3000000.0/(16 * speed * DIV);
			if(FRe < 1.9 && FRe > 1.1)
				break;			
		}
		
		for(NUM = 1; NUM < 15; NUM++)										//find closest fraction to FRe
		{
			for(DEN = 1; DEN < 16; DEN++)
			{
				err = FRe - (1 + (float)NUM/DEN);
				if(err < 0)
					err = -1*err;
				if(err < min)
				{
					min = err;
					LPC_UART0->FDR = NUM + (DEN << 4);				//Set DIVADDVAL and MULVAL
				}
			}
		}
	}
	
	LPC_UART0->DLL = (DIV % 256);											//Set DLL, DLM
	LPC_UART0->DLM = (DIV / 256); 
	
	LPC_UART0->LCR &= 0x7f;
		
}

int UART0_Read(char *buffer, int length)	//Reads characters from the receive buffer onto the supplied string and null terminates
{
	int n = 0;
	
	if(buffer == 0)
		return -1;
	
	while(rl != rh && n < length)		//Read characters till the head is reached or till 'length' characters are read
	{
		buffer[n] = rbuf[rl];		
		rl = (rl + 1) % BFSZ;
		n++;
	}
	buffer[n] = 0;

	return n;			//Return number of characters read	
}

int UART0_ReadAll(char* buffer)
{
	return UART0_Read(buffer, rh - rl);
}

int UART0_RecvAck(void)
{
	if(rx0_flag == 0xff)
	{
		rx0_flag = 0;
		return 1;
	}
	return 0;
}

char UART0_WaitForChar(void)
{
	int rh_initial = rh;
	
	UART0_RecvEnable();
	while(rx0_flag == 0);
	UART0_RecvDisable();
	rx0_flag = 0;
	rl = rh;
	
	return rbuf[rh_initial];
}

void UART0_WaitForLength(int length)
{
	while(length-- > 0)
	{
		while(UART0_RecvAck() == 0);
	}	
}

int ReadTillChar(char c, char* read_msg, uint8_t msg_limit)
{
	int charsRead = 0; 
	int initrf = rh;
	
	UART0_RecvEnable();
	
	rf = rh;
	while(rbuf[rf] != c)
	{
//		while(UART0_Send("\r\n Waiting \r\n") == 0);
//		while(UART0_TransmitStatus() != 0);		
	};
	rx0_flag = 0;
	
	UART0_RecvDisable();
	
	charsRead = rh - initrf;
	if(charsRead < 0)
		charsRead += BFSZ;
	
	if(charsRead > msg_limit)
	{
		charsRead = msg_limit;
		rl = rh - msg_limit;
		if(rl < 0)
			rl += BFSZ;
	}
	else
		rl = initrf;
	
	UART0_Read(read_msg, charsRead);
	
	return charsRead;
}
