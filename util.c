#include "util.h"

int stopi(char * string_conv, int length)
{
	int radix = 1;
	int conv_num = 0;
	
	if(string_conv == 0)
		return -1;
	
	while(length--)
	{
		if(string_conv[length] >= '0' && string_conv[length] <= '9')
		{
			conv_num += (string_conv[length] - '0') * radix;
			radix *= 10;
		}
		else
			return -1;
	}
	
	return conv_num;
}

int pitos(int num_conv, char* conv_string)
{
	int d[10] = {0};
	int i = 0, n = 0;
	
	if(conv_string == 0)
		return -1;
	
	if(num_conv < 0)
		return -1;
	
	while(num_conv != 0)
	{
		d[i] = num_conv % 10;
		num_conv /= 10;
		i++;
	}
	
	n = i;
	
	while(--i >= 0)
	{
		*conv_string = d[i] + '0';
		conv_string++;
	}
	
	*conv_string = 0;
	conv_string -= n;
	
	return 1;
	
}
int StrLength(char * string_meas)
{
	int n = 0;
	
	if(string_meas == 0)
		return -1;
	
	while(string_meas[n] != 0)
		n++;
	
	return n;
}
	
float stopf(char * string_conv, int length)
{
	int i, j = length, k = 0;
	int integer_part = 0, fractional_part = 0;
	float multiplier = 1;
	
	if(string_conv == 0)
		return -1;
	
	if(length == 0)
		return 0;
	
	length--;
	
	for(i = length; i >= 0; i--)
	{
		if(string_conv[i] == '.')
		{
			j = i;		
			k++;
			if(k > 1)
				return -1;
		}
		else if(!(string_conv[i] >= '0' && string_conv[i] <= '9'))
				return -1;
		
		if(k == 0)
			multiplier *= 0.1;
	}
	
		integer_part = stopi(string_conv, j);
		fractional_part = stopi(&(string_conv[j + 1]), length - j);
	
	return integer_part + multiplier * fractional_part;
}
