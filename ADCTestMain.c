// CECS 347 Lab 4 -  Measurement of Distance
// Description: Design a distance meter. An IR distance sensor converts distance into voltage. Software uses 
// the 12-bit ADC built into the microcontroller. The ADC will be sampled at 20Hz using SysTick interrupts. Write 
// a C function that converts the ADC sample into distance with units of 1cm. That data stream will be passed from
// the ISR into the main program using a mailbox, and the main program will output the data on an LCD display
// Name: Len Quach

// ADCTestMain.c
// Runs on LM4F120/TM4C123
// This program periodically samples ADC channel 1 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.

// input signal connected to PE2/AIN1

#include "ADCSWTrigger.h"
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "Nokia5110.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

void SysTick_Init(unsigned long);
void SysTick_Handler(void);
unsigned char TableLookUp(unsigned int);
unsigned char equation(unsigned int);
void Delay(unsigned long);

#define S_PERIOD 60000 //for 125K max sampling rate: 80M/125K=640, 600<640
volatile unsigned int ADCvalue=0;
unsigned long ADC;
unsigned long read_ADC;
unsigned long flag;
unsigned long adcOut;
unsigned long disT; //distance from table
unsigned long disC; //distance from calibration

unsigned int adcTable[] = {3989,3687,2970,2389,2167,2034,1815,1728,1534,1436,1370,1303,1248,1112,1077,1050,1030,979,948,897,839,848,768,750,740,743,653,639};
unsigned int distCm[]   = {  6 ,   8,  10,  12,  14,  16,  18,  20,  22,  24,  26,  28,  30,  32,  34,  36,  38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60};

// The digital number ADCvalue is a representation of the voltage on PE4 
// voltage  ADCvalue
// 0.00V     0
// 0.75V    1024
// 1.50V    2048
// 2.25V    3072
// 3.00V    4095
	
int main(void){unsigned long volatile delay;
	DisableInterrupts();  
  PLL_Init();                           // 80 MHz
	Nokia5110_Init();	                    
  ADC0_InitSWTriggerSeq3_Ch1();         // ADC initialization PE2/AIN1
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // activate port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_DIR_R |= 0x04;             // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;          // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;             // enable digital I/O on PF2
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
	SysTick_Init(S_PERIOD); 
	EnableInterrupts();

	Nokia5110_Clear();
  Nokia5110_OutString("************CECS347 LAB4************");
	Nokia5110_OutString("ADC:        ");
	Nokia5110_OutString("disT:       ");
	Nokia5110_OutString("disC:       ");
	
  while(1){
		if (flag == 1){
		  ADCvalue = ADC;
		  flag = 0; //reset the value
		} 
		GPIO_PORTF_DATA_R |= 0x04;          // profile
		for(delay=0; delay<100000; delay++){};
		     	 
		disT = TableLookUp(ADCvalue); // calculater table look up value
		adcOut = ADCvalue;
		disC = equation(ADCvalue);  // calculate equation    

		Nokia5110_SetCursor(7, 3); //8th column, 4th row
		Nokia5110_OutUDec((unsigned short)adcOut);			
		Nokia5110_SetCursor(7, 4);
		Nokia5110_OutUDec((unsigned short)disT); //distance from table lookup
		Nokia5110_SetCursor(7, 5);
		Nokia5110_OutUDec((unsigned short)disC); //distance from calibration (equation)
			
		Delay(4000000); 
		GPIO_PORTF_DATA_R &= ~0x04; //set breakpoint here to capture the samples							
  }
}

// every time SysTick Timer reaches 0, ISR will be called -> sample 1 value and convert it to digital
// SysTick Timer controls samping rate < max rate
void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;								// disable systick for setup
	NVIC_ST_RELOAD_R = period - 1;		// reload value: if period = 4
	NVIC_ST_CURRENT_R = 0;						// any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x1FFFFFFF)|0x40000000;// priority 2
																	 //enable SysTick with core clock and interrupt 
	NVIC_ST_CTRL_R = 0x07;           // 0000 0111
}	

//Interrupt service routine
//Executed every 62.5ns*(period)
void SysTick_Handler(void){
		for (int i=0; i<50; i++){
			read_ADC = ADC0_InSeq3(); //mailbox (to get the data)
			ADC += read_ADC;
		} 
		ADC /= 50; //50 average measurements
		flag = 1;
}

//table lookup using successive approximation
unsigned char TableLookUp(unsigned int adc){
	unsigned int delta=0;
	unsigned char dis;
	
	for(int i=0; i < sizeof(adcTable); i++){ 
		 if(adcTable[i] < adc){
			  delta = (adcTable[i-1] - adcTable[i]) /2;
			  if(adc >= delta)		
				   dis = distCm[i] + 1;
	    	else
					 dis = distCm[i];
		    return dis;
		 }	
	}
	return 255;
}
	
//find distance using an equation
unsigned char equation(unsigned int adcOut){
	float a = -4.889;
	float b = 42865.3;
  unsigned char dist = a + b/adcOut;
	return dist;
}
	
//function delays 3*ulCount cycles
void Delay(unsigned long ulCount){
  do{
    ulCount--;
	} while(ulCount);
}


