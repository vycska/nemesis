#ifndef __ADC_H__
#define __ADC_H__

#define ADC_TOTAL_DATA (50)

struct ADC_Data {
   short data[ADC_TOTAL_DATA];
   int index,count,sum;
};

void ADC_Init(void);

#endif
