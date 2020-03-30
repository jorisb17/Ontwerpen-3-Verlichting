/*
 * header.h
 *
 * Created: 27-2-2020 14:46:25
 * Author: Joris
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nrf24spiXM2.h"
#include "nrf24L01.h"

// Prototypes
void init(void);
static void init_pwm(void);
void dim_lamp(float percentage);
static void init_adc(void);
static void init_nrf(void);
uint16_t read_sensor(void);
void set_state(void);
void run_state(void);
void lamp_with_pot(void);
void lamp_with_sensor();
uint8_t map(long x, long in_min, long in_max, long out_min, long out_max);
void lamp_on(void);
void lamp_off(void);
uint16_t average_pot(void);
uint16_t read_pot(void);
uint16_t read_sensor(void);

uint8_t pipe[5]="HvA01";											//Pipe adress

/*!Brief initializes the IO-Pins, the ADC and the PWM signals
*
* \return				void 
*/
void init(void)
{
	PORTB.DIRCLR   = PIN2_bm|PIN3_bm|PIN4_bm|PIN5_bm;				// Configure PIN B2, B3, B4 and B5 as input pins 
	PORTB.PIN5CTRL = PORT_OPC_PULLDOWN_gc;							// Use the internal pulldown resistor for PIN B5
	PORTB.PIN4CTRL = PORT_OPC_PULLDOWN_gc;							// Use the internal pulldown resistor for PIN B4
	PORTB.PIN3CTRL = PORT_OPC_PULLDOWN_gc;							// Use the internal pulldown resistor for PIN B3
	PORTB.PIN2CTRL = PORT_OPC_PULLDOWN_gc;							// Use the internal pulldown resistor for PIN B2
	init_nrf();
	init_adc();
	init_pwm();
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	sei();
}

/*!Brief initializes the ADC 
*
* \return				void
*/
static void init_adc(void)
{
	PORTA.DIRCLR		= PIN1_bm;									// PIN A1 as input for ADC
	ADCA.CH0.MUXCTRL	= ADC_CH_MUXPOS_PIN1_gc;					// Bind PIN A1 to channel 0
	ADCA.CH0.CTRL		= ADC_CH_INPUTMODE_SINGLEENDED_gc;			// Configure channel 0 as single-ended
	ADCA.REFCTRL		= ADC_REFSEL_INTVCC_gc;						// Use internal VCC/1.6 reference
	ADCA.CTRLB			= ADC_RESOLUTION_12BIT_gc;					// 12 bits conversion, unsigned, no freerun
	ADCA.PRESCALER		= ADC_PRESCALER_DIV256_gc;					// 32000000/256 = 125kHz		
	ADCA.CTRLA			= ADC_ENABLE_bm;							// Enable ADC

	PORTB.DIRCLR		= PIN1_bm;									// PIN B1 as input for ADC
	ADCB.CH0.MUXCTRL	= ADC_CH_MUXPOS_PIN1_gc;					// Bind PIN B1 to channel 0
	ADCB.CH0.CTRL		= ADC_CH_INPUTMODE_SINGLEENDED_gc;			// Configure channel 0 as single-ended
	ADCB.REFCTRL		= ADC_REFSEL_INTVCC_gc;						// Use internal VCC/1.6 reference
	ADCB.CTRLB			= ADC_RESOLUTION_12BIT_gc;					// 12 bits conversion, unsigned, no freerun
	ADCB.PRESCALER		= ADC_PRESCALER_DIV256_gc;					// 32000000/256 = 125kHz	
	ADCB.CTRLA			= ADC_ENABLE_bm;							// Enable ADC
}

/*!Brief initializes a PWM signal
*
* \return				void
*/
static void init_pwm(void)
{
	PORTD.DIRSET = PIN0_bm;											// PIN D0 as output								
	
	TCD0.CTRLB   = TC0_CCAEN_bm | TC_WGMODE_SS_gc;					// Enable CCA in singleslope mode
	TCD0.CTRLA   = TC_CLKSEL_DIV4_gc;
	TCD0.PER     = 9999;											// Top value
	TCD0.CCA     = 0;												// Duty cycle 0%
}

/*!Brief initializes the NRF module
*
* \return				void
*/
static void init_nrf(void)
{
	nrfspiInit();													// Initialize SPI
	nrfBegin();														// Initialize radio module

	nrfSetRetries(NRF_SETUP_ARD_1000US_gc,							// Auto Retransmission Delay: 1000 us
	NRF_SETUP_ARC_8RETRANSMIT_gc);									// Auto Retransmission Count: 8 retries
	nrfSetPALevel(NRF_RF_SETUP_PWR_6DBM_gc);						// Power Control: -6 dBm
	nrfSetDataRate(NRF_RF_SETUP_RF_DR_250K_gc);						// Data Rate: 250 Kbps
	nrfSetCRCLength(NRF_CONFIG_CRC_16_gc);							// CRC Check
	nrfSetChannel(32);												// Channel: 32
	nrfSetAutoAck(1);												// Auto Acknowledge on
	nrfEnableDynamicPayloads();										// Enable Dynamic Payloads

	nrfClearInterruptBits();										// Clear interrupt bits
	nrfFlushRx();													// Flush fifo's
	nrfFlushTx();
	
	PORTF.INT0MASK |= PIN6_bm;
	PORTF.PIN6CTRL  = PORT_ISC_FALLING_gc;
	PORTF.INTCTRL   = (PORTF.INTCTRL & ~PORT_INT0LVL_gm) | PORT_INT0LVL_LO_gc;
	
	nrfOpenReadingPipe(0, pipe);
	nrfStartListening();
}
