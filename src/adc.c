#include "adc.h"
#include "main.h"
#include "utils.h"
#include "lpc824.h"

volatile struct ADC_Data adc_data;

void ADC_Init(void) {
   ADCSEQA_CTRL &= (~(1u<<31)); //disable sequence A
   ADCSEQB_CTRL &= (~(1u<<31)); //disable sequence B
   PIO0_14 = 0; //the most important here is disabling pd/pu resistors
   PINENABLE0 &= (~(1<<15)); //ADC_2 enabled on PIO0_14
   ADCTHR0_LOW = ADCTHR1_LOW = ADCTHR0_HIGH = ADCTHR1_HIGH = 0; //threshold intervals not used
   ADCCHAN_THRSEL = 0; //threshold intervals not used, but set THR0 (not THR1) to be used for all channels
   ADCTRM &= (~(1<<5)); //VDD = 2.7V to 3.6V
   ADCCTRL = ((CLOCK*2)<<0 | 0<<10 | 1<<30); //divider for 500kHz, low power mode, calibration mode
   while(ADCCTRL&(1<<30)); //wait for calibration cycle to complete
   ADCCTRL = (0<<0 | 1<<10); //divider 0+1, low power mode
   ADCSEQA_CTRL = (1<<2 | 0<<12 | 1<<18 | 0<<19 | 0<<26 | 0<<27 | 0<<28 | 0<<29 | 1<<30 | 1u<<31); //select channel 2, conversions are initiated by software, polarity is positive edge, enable synchronization, no start, no burst, no singlestep, low priority for sequence A, end of sequence mode, sequence enabled
   ADCINTEN = (1<<0); //sequence A interrupt enable (end of sequence because of mode) [no sequence B, overrun and threshold comparison for all channels interrupts]
   IPR4 = (IPR4&(~(3<<6))) | (2<<6); //interrupt priority 2 (0 = highest, 3 = lowest)
   ISER0 = (1<<16); //ADC_SEQA interrupt enable
}

void ADC_SEQA_IRQHandler(void) {
   ADCFLAGS = (1<<28); //clear sequence A end of sequence interrupt flag
   adc_data.sum -= adc_data.data[adc_data.index];
   adc_data.data[adc_data.index] = (ADCDAT2>>4)&0xfff;
   adc_data.sum += adc_data.data[adc_data.index];
   adc_data.index = (adc_data.index+1)%ADC_TOTAL_DATA;
   adc_data.count = MIN2(adc_data.count+1,ADC_TOTAL_DATA);
}
