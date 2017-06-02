#include <LPC17xx.h>

#ifndef __UART0_H
#define __UART0_H

enum SerialConfig
{
	
 SERIAL_5N1, SERIAL_5N2, SERIAL_5O1, SERIAL_5O2, SERIAL_5E1, SERIAL_5E2, 
 SERIAL_6N1, SERIAL_6N2, SERIAL_6O1, SERIAL_6O2, SERIAL_6E1, SERIAL_6E2, 
 SERIAL_7N1, SERIAL_7N2, SERIAL_7O1, SERIAL_7O2, SERIAL_7E1, SERIAL_7E2, 
 SERIAL_8N1, SERIAL_8N2, SERIAL_8O1, SERIAL_8O2, SERIAL_8E1, SERIAL_8E2, 
	
};

void UART0_Init(void);
void UART0_IRQHandler(void);
signed char UART0_Send(char * transData);
unsigned char UART0_TransmitStatus(void);
void UART0_Configure(unsigned long int speed, enum SerialConfig configure);
int UART0_Read(char *buffer, int length);
int UART0_ReadAll(char* buffer);
int UART0_RecvAck(void);
void UART0_WaitForLength(int length);
char UART0_WaitForChar(void);
void UART0_RecvEnable(void);
void UART0_RecvDisable(void);
int ReadTillChar(char c, char* read_msg, uint8_t msg_limit);


extern unsigned char recv_buf[50], recv_data, recv_index;
extern unsigned char rx0_flag, tx0_flag;

#endif
