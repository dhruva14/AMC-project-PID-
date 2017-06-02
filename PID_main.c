#include "lpc17xx.h"
#include "util.h"
#include <stdio.h>

#define AVG_SIZE 2
//declarations of constants and all variables used
float kp = 20, ki = 500, kd = 3, dt = .2; //float kp = 20, ki = 900, kd = 1, dt = .2; // 200, 120, 1 works 2 1500 1    20 900 1 .2
float integral_term = 0, prop_term = 0, derv_term = 0, PID_max = 65535;
float rpm_current = 0, rpm_error = 0, prev_rpm_error = 0, ref_rpm = 50, rpm_max = 120;// calc_rpm = 0, separation;
int PID_op = 0, duty_set = 0, per_set = 200, duty_old = 0;
int encoder_ppr = 20, direction = 1, dirn_reverse = 1;
float avg_rpm = 0; int sample_size, avg_counter = 0;
char input[4] = "aaa";char in = 0;
int txn_enable = 1, len, value, stop = 0, i = 0;
float fl_in = 0;
float ravg_rpm[AVG_SIZE] = {0};

void (*callBack)(void);

char msg[50] = "";
int code = 0;

//calculates and returns the pid output using current and previous error
//pid algorithm implemented
int calc_PID(void)
{ 
	integral_term += ki*dt*rpm_error;
 	prop_term = kp*rpm_error;
	derv_term = kd*(rpm_error - prev_rpm_error)/dt;
	prev_rpm_error = rpm_error;
	
	if(integral_term > PID_max)
		integral_term = PID_max;
	if(integral_term < -PID_max)
		integral_term = -PID_max;
	
	PID_op = (int) (integral_term + prop_term + derv_term);
	
	if(PID_op > PID_max)
		PID_op = PID_max;
	else if(PID_op < -PID_max)
		PID_op = -PID_max;

		return PID_op;
}
//for direction change
void dirn_change_handler(void)
{
	direction *= -1;
}
//this func gets called every time the QEI timer reloads
//cal pid and applies reqd duty cycle to motor
void PID_sequence(void)
{
	rpm_current = (float) QEI_get_RPM();
	
	avg_rpm = 0;
	for(i = AVG_SIZE - 1; i > 0; i--)
	{
			ravg_rpm[i] = ravg_rpm[i - 1];
			avg_rpm += ravg_rpm[i];
	}
	ravg_rpm[0] = rpm_current;
	avg_rpm += ravg_rpm[0];
	avg_rpm /= AVG_SIZE;
	
	rpm_error = ref_rpm - avg_rpm;
	duty_set = (int) (calc_PID());
	
	
	if((duty_set != duty_old))
	{
		MCPWMConfig(per_set, (int) (duty_set * dirn_reverse * 50) / PID_max);
		MCPWMStart();
	}
	
	duty_old = duty_set;
	
	avg_counter = (avg_counter + 1) % sample_size;
	
	if(avg_counter == 0)
	{
		sprintf(msg, "%f,", avg_rpm);
		if(txn_enable != 0 )
			UART0_Send(msg);
	}
}

//this func can be called every time QEI timer reloads
//calculates duty cycle without feedback
void openLoop_sequence(void)
{
	rpm_current = (float) QEI_get_RPM();
	
	duty_set = (int)((ref_rpm * 50)/rpm_max);
	
	if(duty_set != duty_old)
	{
		MCPWMConfig(per_set, dirn_reverse * duty_set);
		MCPWMStart();
	}
	
	duty_old = duty_set;
	
	sprintf(msg, "%f, %d \r\n", avg_rpm, (int) ((duty_set * dirn_reverse * 50) / PID_max));
	
	if(txn_enable != 0)
			UART0_Send(msg);
}

int main(void)
{	
	SystemInit();
	SystemCoreClockUpdate();
	
	UART0_Init();
	UART0_Configure(9600, SERIAL_8N1);
	
	callBack = PID_sequence;		
	
	sample_size = (int) (1 / dt);
	MCPWMInit();
	
	QEI_Init(callBack, dirn_change_handler);
	QEI_set_PPR(encoder_ppr);
	QEI_set_VelTimer(dt*1000);
	
	while(stop != 1)
	{
		ReadTillChar('g', input, 2);
		in = input[0];
		txn_enable = 0;
		
		
		switch(in)
		{
			case '1':		
				sprintf(msg, "\r\n Input New Reference RPM (followed by g) \r\n");
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
					
				len =	ReadTillChar('g', msg, 5);
				value = stopi(msg, len - 1);
				
				if(value >= 0 && value < rpm_max)
				{
					ref_rpm = value;
					avg_counter = 0;
					avg_rpm = 0;
				}	
				break;
				
			case '2':
				sprintf(msg, "\r\n Using Open Loop \r\n");
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
				
				QEI_set_timer_callback(openLoop_sequence);
			
				break;
			
			case '3':
				sprintf(msg, "\r\n Using PID \r\n");
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
				
				QEI_set_timer_callback(PID_sequence);
			
				break;
			
			case '4':
				sprintf(msg, "\r\n Reversing Direction \r\n");
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
				
				dirn_reverse *= -1;
			
				break;
			
			case '5':
				sprintf(msg, "\r\n Input k_p (currently %f) \r\n", kp);
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
			
				len =	ReadTillChar('g', msg, 12);
				fl_in = stopf(msg, len - 1);
				
				if(fl_in >= 0)
					kp = fl_in;
				break;
				
			case '6':
				sprintf(msg, "\r\n Input k_i (currently %f) \r\n", ki);
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
			
				len =	ReadTillChar('g', msg, 12);
				fl_in = stopf(msg, len - 1);
				
				if(fl_in >= 0)
					ki = fl_in;
				break;
				
			case '7':
				sprintf(msg, "\r\n Input k_d (currently %f) \r\n", kd);
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
			
				len =	ReadTillChar('g', msg, 12);
				fl_in = stopf(msg, len - 1);
				
				if(fl_in >= 0)
					kd = fl_in;
				break;
				
			case '0':
				sprintf(msg, "\r\n Stopping Program \r\n");
				while(UART0_Send(msg) == 0);
				while(UART0_TransmitStatus() != 0);
					
				QEI_disable();
				MCPWMDisable();
				stop = 1;
			
				break;
		}
		txn_enable = 1;
	}

	while(1);
}


