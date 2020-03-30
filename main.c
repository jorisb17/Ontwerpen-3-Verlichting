/*
 * Main
 *
 * Created: 25-2-2020 13:02:16
 * Author : Joris Bruinsma
 */ 
#define F_CPU 32000000UL
#include "header.h"

#define STEP 10
#define BOUND 2000
#define UPPER 100
#define BUFFER_SIZE 255
#define POT_SIZE 10

int state = 4;
int lamp  = 0;
int indexArray = 0;

int potArray[POT_SIZE];
int buffer[BUFFER_SIZE];

int main(void)
{
	init();
	while (1) 
	{
		setState();
	}    
}

/*!Brief Set the state according to the inputs
*
* \return				void
*/
void setState()
{
	if(PORTB.IN & PIN2_bm)
	{
		state = 1;
	}
	else if(PORTB.IN & PIN3_bm)
	{
		state = 2;
	}
	else if(PORTB.IN & PIN4_bm)
	{
		state = 3;
	}
	else if(PORTB.IN & PIN5_bm)
	{
		state = 4;
	}
	runState();
}

/*!Brief execute functions according to state
*
* \return				void
*/
void runState()
{
	switch(state)
	{
		case 1:
			lampWithSensor();
			break;
		case 2:
			lampWithPot();
			break;
		case 3:
			lampOn();
			break;
		case 4:
			lampOff();
			break;
		default:
			break;
	}
}

/*!Brief Turn the lamp on
*
* \return				void
*/
void lampOn(void)
{
	TCD0.CCA = 9999;
}

/*!Brief turn the lamp off
*
* \return				void
*/
void lampOff(void)
{
	TCD0.CCA = 0;
}

/*!Brief calculate the average of the last 10 read values of the potentiometer
*
* \return				void
*/
uint16_t averagePot(void)
{
	uint16_t value = read_pot();
	uint16_t sum = 0;
	
	if(indexArray >= POT_SIZE)
	{
		indexArray = 0;
	}
	
	potArray[indexArray] = value;
	indexArray++;
	
	for(int x = 0; x < POT_SIZE; x++)
	{
		sum += potArray[x];
	}
	
	return sum/POT_SIZE;
}

/*!Brief Use the avveragePots value to control the brightens level of the Lamp
*
* \return				void
*/
void lampWithPot(void)
{
	uint16_t value = averagePot();
	TCD0.CCA = map(value, 0, 4095, 0 , 9999);
}

/*!Brief Use the light sensors value to keep the brightens level of the Lamp the same
*
* \return				void
*/
void lampWithSensor()
{
	int value = read_sensor();
	if(value < BOUND - UPPER){
		lamp += STEP;
		if(lamp > 9999)
		{
			lamp = 9999;
		}
		TCD0.CCA = lamp;
	}
	else if(value > BOUND + UPPER)
	{
		lamp -= STEP;
		if(lamp < 0)
		{
			lamp = 0;
		}
		TCD0.CCA = lamp;
	}
}

/*! Brief Re-maps a number from one range to another
*
* \Param x				the number to map
* \Param in_min			the lower bound of the value’s current range
* \Param in_max			the upper bound of the value’s current range
* \Param out_min		the lower bound of the value’s target range
* \param out_max		the upper bound of the value’s target range
*
* \return				mapped value
*
* From: https://www.arduino.cc/reference/en/language/functions/math/map/
*/
uint8_t map(long x, long in_min, long in_max, long out_min, long out_max) 
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*!Brief reads the value of the light sensor connected to pin
*
* \return				result of light sensor
*/
uint16_t read_sensor(void)
{
	uint16_t res;

	ADCA.CH0.CTRL |= ADC_CH_START_bm;								//Start ADC conversion
	while(!(ADCA.CH0.INTFLAGS));									//Wait for conversion to finish
	res = ADCA.CH0.RES;												//Store result in res
	ADCA.CH0.INTFLAGS |= ADC_CH_CHIF_bm;							//Reset interrupt register
	
	return res;
}

/*!Brief reads the value of the potentiometer connected to pin
*
* \return				result of potentiometer
*/
uint16_t read_pot(void)
{
	uint16_t res;
	
	ADCB.CH0.CTRL |= ADC_CH_START_bm;								//Start ADC conversion
	while(!(ADCB.CH0.INTFLAGS));									//Wait for conversion to finish
	res = ADCB.CH0.RES;												//Store result in res
	ADCB.CH0.INTFLAGS |= ADC_CH_CHIF_bm;							//Reset interrupt register
	
	return res;
}

/*!Brief Interrupt that triggers when a messages is received
*/
ISR(PORTF_INT0_vect)
{
	uint8_t tx_ds, max_rt, rx_dr;
	
	nrfWhatHappened(&tx_ds, &max_rt, &rx_dr);						//Check what happened
	
	if(rx_dr)														//messages received correctly
	{																
		nrfRead(buffer, nrfGetDynamicPayloadSize());				//read first byte
		uint8_t res = buffer[0];									//store first byte				
		if(res == 'c')												//Store is 'c'
		{															
			state = 3;												//Lamp on
		}
	}
}