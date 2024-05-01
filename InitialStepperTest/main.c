/*
 * InitialStepperTest.c
 *
 * Created: 2023-03-16 3:17:26 PM
 * Author : Luke
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "lcd.h"

// Global Variables
volatile unsigned char STATE;
volatile unsigned int curPosition;
volatile unsigned int curStepperMaterial;
volatile unsigned int new_material;
volatile unsigned int ADC_result;
volatile unsigned int min_ADC;
volatile unsigned int listArray[100];
volatile unsigned int currentListCount;
volatile unsigned int listAddedCount;
volatile unsigned int listDropCount;
volatile unsigned int rampDownFlag;
volatile unsigned int addToListFlag;
volatile unsigned int enableRampDown;
volatile unsigned int bufferFlag;

char forward = 0b1110;
char brake = 0b1111;

//each situation requires a different delay pattern depending on direction change and length of turn
int fiftyStepDelays[] = {130,	130,	127,	120,	113,	106,	99,	92,	85,	78,	71,	64,	57,	50,	48,	47,	47,	46,	46,	45,	45,	44,	44,	44,	43,	43,	44,	44,	44,	45,	45,	46,	46,	47,	47,	48,	50,	57,	64,	71,	78,	85,	92,	99,	106,	113,	120,	127,	130,	130};
int hundredStepDelays[] = {130,	130,	127,	120,	113,	106,	99,	92,	85,	78,	71,	64,	57,	51,	48,	46,	45,	44,	43,	43,	43,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	43,	43,	43,	44,	44,	45,	46,	47,	47,	48,	57,	64,	71,	78,	85,	92,	99,	106,	113,	130,	130};
int frontBufferFiftyStep[] = {43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	44,	44,	44,	45,	45,	46,	46,	47,	47,	48,	50,	57,	64,	71,	78,	85,	92,	99,	106,	113,	120,	127,	130,	130};
int frontBufferHundredStep[] = {43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	43,	43,	43,	44,	44,	45,	46,	47,	47,	48,	57,	64,	71,	78,	85,	92,	99,	106,	113,	130,	130};
int backBufferFiftyStep[] = {130,	130,	127,	120,	113,	106,	99,	92,	85,	78,	71,	64,	57,	50,	48,	47,	47,	46,	46,	45,	45,	44,	44,	44,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43, 43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43};
int backBufferHundredStep[] = {130,	130,	127,	120,	113,	106,	99,	92,	85,	78,	71,	64,	57,	51,	48,	46,	45,	44,	43,	43,	43,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43, 43, 43,20};
int bothBufferFiftyStep[] = {43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43, 43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43, 43, 43, 43};
int bothBufferHundredStep[] = {43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	42,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43,	43, 43, 20};

char step1to2 = 0b00011011;
char step2to3 = 0b00011101;
char step3to4 = 0b00101101;
char step4to1 = 0b00101011;
char step1to4 = 0b00011011;
char step2to1 = 0b00011101;
char step3to2 = 0b00101101;
char step4to3 = 0b00101011;

int endBuffer = 35;

//demo piece values for reflectivity
int aluminiumMax = 250;
int steelMax = 750;
int whiteMax = 899;

int white = 1;
int black = 2;
int steel = 3;
int aluminium = 4;

//Function Declarations
void adcTimer(int count);
void fastPWM();
void initializeStepper();
void home();
void Rotate_Stepper(int new_material, int next_material, int prevBuffer);
void startStop(char startOrStop);
void generalSetUp();
void one_step_CW();
void one_step_CCW();
void CW_100();
void CCW_100();
void CW_50();
void CCW_50();
void ADCsetup();
void CW_100_both();
void CCW_100_both();
void CW_50_both();
void CCW_50_both();
void CW_100_end();
void CCW_100_end();
void CW_50_end();
void CCW_50_end();
void CW_100_prev();
void CCW_100_prev();
void CW_50_prev();
void CCW_50_prev();

int main(void)
{
	generalSetUp();
	ADCsetup();
	fastPWM();
    initializeStepper();
	
	PORTB = forward; //start the belt

	//set state to polling
	STATE = 0;
	
	goto POLLING_STAGE;

	// POLLING STATE
	POLLING_STAGE:
	
	//
	if((PINE & 0x20) == 0x20){
		ADC_result = ADCL + (ADCH<<8);
		if(ADC_result < min_ADC){
			min_ADC = ADC_result;
		}
	}
	if((listDropCount == currentListCount) & (enableRampDown == 1)){
		STATE = 5;
		goto END;
	}else if(STATE == 2){
		goto BUCKET_STAGE;
	}else if(STATE == 0){
		goto POLLING_STAGE;
	}
	
	BUCKET_STAGE:
	
	STATE = 0;
	
	ADC_result = ADCL + (ADCH<<8);
	if(ADC_result < min_ADC){
		min_ADC = ADC_result;
	}
	
	Rotate_Stepper(listArray[listDropCount], listArray[listDropCount+1], bufferFlag);
	
	
	
	//this is here in case exit sensor is triggered while in rotateStepper
	if(STATE == 2){
		goto BUCKET_STAGE;
	}
	
	//unless anything changed go back to polling
	goto POLLING_STAGE;
	
	END:
	adcTimer(4000);
	PORTB = 0xFF & brake;


	int x = 0;
	int whiteDropCount = 0;
	int blackDropCount = 0;
	int steelDropCount = 0;
	int aluminiumDropCount = 0;

	for(x=0;x<listDropCount;x++){
		switch(listArray[x]){
			//white
			case 1:
			whiteDropCount++;
			break;
			//black
			case 2:
			blackDropCount++;
			break;
			//steel
			case 3:
			steelDropCount++;
			break;
			//aluminium
			case 4:
			aluminiumDropCount++;
			break;
		}

	//write the results of the sorting to the LCD
	LCDWriteIntXY(5,1, whiteDropCount, 2);
	LCDWriteIntXY(8,1, blackDropCount, 2);
	LCDWriteIntXY(11, 1, steelDropCount, 2);
	LCDWriteIntXY(14,1, aluminiumDropCount, 2)
	LCDWriteStringXY(0,0,"FINAL SORT");
	LCDWriteStringXY(5,0, "W:");
	LCDWriteStringXY(8,0,"B:");
	LCDWriteStringXY(11,0,"S:");
	LCDWriteStringXY(14,0,"A:");
		
}
	
	return(0);

} // end main


void generalSetUp(){
	CLKPR = 0x80;
	CLKPR = 0x01;
	
	TCCR3B |= _BV(CS30);
	
	
	DDRA = 0x00;
	DDRF = 0x00;
	DDRL = 0xFF;
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0x00;
	DDRH = 0x00;
	DDRE = 0x00;

	min_ADC = 0b1111111111;

	InitLCD(LS_BLINK|LS_ULINE);
	
	//Clear the screen
	LCDClear();
	
	cli(); // disable all of the interrupt ==========================
	
	// config the external interrupts ======================================
	EIMSK = EIMSK | 0b00111100;
	
	//rising edge interrupts
	EICRA |= ( _BV(ISC30) | _BV(ISC31)); //first optical sensor
	EICRB |= ( _BV(ISC51) | _BV(ISC41) | _BV(ISC40)); //start stop button interrupt
	EICRA |= (_BV(ISC21) | _BV(ISC20)); //ramp down button interrupt
	
	sei();
	
}

void ADCsetup(){
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADATE); // enable auto triggering (in free run mode) - using this for continuous loading
	ADMUX |= _BV(REFS0); 
	ADCSRA |= _BV(ADSC); //start ADC conversions
	
}

void startStop(char startOrStop){
	if(startOrStop == brake){
		PORTB = 0xFF & brake;
	}else if(startOrStop == forward){
		PORTB = 0xFF & forward;
	}else if(startOrStop == 0){
		//if not moving, go forward
		if (PORTB == brake){
			PORTB = 0xFF & forward;
			}else{
			PORTB = 0xFF & brake;
		}
	}
}

//initialize stepper and set the current position to 2 - so that it's known
void initializeStepper(){
	int i;
	int j = 1;
	for(i=0;i<50;i++){	
		if(j==1){
			PORTA = step1to2;
			adcTimer(fiftyStepDelays[i]);
			j++;
		}else if(j==2){
			PORTA = step2to3;
			adcTimer(fiftyStepDelays[i]);
			j++;
		}else if(j==3){
			PORTA = step3to4;
			adcTimer(fiftyStepDelays[i]);
			j++;
		}else if(j==4){
			PORTA = step4to1;
			adcTimer(fiftyStepDelays[i]);
			j=1;
		}
	}
	curPosition = 2;
	home();
}

//set the stepper to the home position (centre of black tray)
void home(){
	while((PIND & 0x02) == 0x02){
		one_step_CW(160);
	}
	int i = 0;
	for(i=0;i<8;i++){
		one_step_CW(160);
	}
	curStepperMaterial = black;
	
}



void one_step_CCW(int delay){
	if (curPosition == 1){
		PORTA = step1to2;
		adcTimer(delay);
		curPosition = 2;
	}
	else if (curPosition == 2){
		PORTA = step2to3;
		adcTimer(delay);
		curPosition = 3;
	}
	else if (curPosition == 3){
		PORTA = step3to4;
		adcTimer(delay);
		curPosition = 4;
	}
	else if (curPosition == 4){
		PORTA = step4to1;
		adcTimer(delay);
		curPosition = 1;
	}
}


void one_step_CW(int delay){
	if (curPosition == 1){
		PORTA = step1to4;
		adcTimer(delay);
		curPosition = 4;
	}else if (curPosition == 4){
		PORTA = step4to3;
		adcTimer(delay);
		curPosition = 3;
	}else if (curPosition == 3){
		PORTA = step3to2;
		adcTimer(delay);
		curPosition = 2;
	}else if (curPosition == 2){
		PORTA = step2to1;
		adcTimer(delay);
		curPosition = 1;
	}
	
}

void CW_50(){
	int i;
	for(i=0;i<25;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 4;
		}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 3;
		}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 2;
		}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=25;i<50;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 4;
		}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 3;
		}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 2;
		}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 1;
		}
	}
}

void CW_100(){
	int i;
	for(i=0;i<75;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(hundredStepDelays[i]);
			curPosition = 4;
		}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(hundredStepDelays[i]);
			curPosition = 3;
		}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(hundredStepDelays[i]);
			curPosition = 2;
		}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(hundredStepDelays[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=75;i<100;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(hundredStepDelays[i]);
			curPosition = 4;
		}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(hundredStepDelays[i]);
			curPosition = 3;
		}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(hundredStepDelays[i]);
			curPosition = 2;
		}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(hundredStepDelays[i]);
			curPosition = 1;
		}
	}
}

void CCW_50(){
	int i;
	for(i=0;i<25;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=25;i<50;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 2;
		}
		else if(curPosition == 2){
			PORTA = step2to3;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 3;
		}
		else if(curPosition == 3){
			PORTA = step3to4;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 4;
		}
		else if(curPosition == 4){
			PORTA = step4to1;
			adcTimer(fiftyStepDelays[i]);
			curPosition = 1;
		}
	}
}

void CCW_100(){
	int i;
	for(i=0;i<75;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(hundredStepDelays[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(hundredStepDelays[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(hundredStepDelays[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(hundredStepDelays[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=75;i<100;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(hundredStepDelays[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(hundredStepDelays[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(hundredStepDelays[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(hundredStepDelays[i]);
			curPosition = 1;
		}
	}
}


void CW_50_end(){
	int i;
	for(i=0;i<20;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=20;i<49;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 1;
		}
	}if (curPosition == 1){
	PORTA = step1to4;
	adcTimer(endBuffer);
	curPosition = 4;
	}else if (curPosition == 4){
	PORTA = step4to3;
	adcTimer(endBuffer);
	curPosition = 3;
	}else if (curPosition == 3){
	PORTA = step3to2;
	adcTimer(endBuffer);
	curPosition = 2;
	}else if (curPosition == 2){
	PORTA = step2to1;
	adcTimer(endBuffer);
	curPosition = 1;
}
}

void CW_100_end(){
	int i;
	for(i=0;i<70;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=70;i<100;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 1;
		}
	}if (curPosition == 1){
	PORTA = step1to4;
	adcTimer(endBuffer);
	curPosition = 4;
	}else if (curPosition == 4){
	PORTA = step4to3;
	adcTimer(endBuffer);
	curPosition = 3;
	}else if (curPosition == 3){
	PORTA = step3to2;
	adcTimer(endBuffer);
	curPosition = 2;
	}else if (curPosition == 2){
	PORTA = step2to1;
	adcTimer(endBuffer);
	curPosition = 1;
}
}

void CCW_50_end(){
	int i;
	for(i=0;i<20;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=20;i<50;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 2;
		}
		else if(curPosition == 2){
			PORTA = step2to3;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 3;
		}
		else if(curPosition == 3){
			PORTA = step3to4;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 4;
		}
		else if(curPosition == 4){
			PORTA = step4to1;
			adcTimer(backBufferFiftyStep[i]);
			curPosition = 1;
		}
	}if (curPosition == 1){
	PORTA = step1to2;
	adcTimer(endBuffer);
	curPosition = 2;
}
else if (curPosition == 2){
	PORTA = step2to3;
	adcTimer(endBuffer);
	curPosition = 3;
}
else if (curPosition == 3){
	PORTA = step3to4;
	adcTimer(endBuffer);
	curPosition = 4;
}
else if (curPosition == 4){
	PORTA = step4to1;
	adcTimer(endBuffer);
	curPosition = 1;
}
}

void CCW_100_end(){
	int i;
	for(i=0;i<70;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=70;i<99;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(backBufferHundredStep[i]);
			curPosition = 1;
		}
	}if (curPosition == 1){
	PORTA = step1to2;
	adcTimer(endBuffer);
	curPosition = 2;
	}
	else if (curPosition == 2){
		PORTA = step2to3;
		adcTimer(endBuffer);
		curPosition = 3;
	}
	else if (curPosition == 3){
		PORTA = step3to4;
		adcTimer(endBuffer);
		curPosition = 4;
	}
	else if (curPosition == 4){
		PORTA = step4to1;
		adcTimer(endBuffer);
		curPosition = 1;
	}
}

void CW_50_both(){
	int i;
	for(i=0;i<10;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=10;i<49;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	if (curPosition == 1){
		PORTA = step1to4;
		adcTimer(endBuffer);
		curPosition = 4;
		}else if (curPosition == 4){
		PORTA = step4to3;
		adcTimer(endBuffer);
		curPosition = 3;
		}else if (curPosition == 3){
		PORTA = step3to2;
		adcTimer(endBuffer);
		curPosition = 2;
		}else if (curPosition == 2){
		PORTA = step2to1;
		adcTimer(endBuffer);
		curPosition = 1;
	}
}

void CW_100_both(){
	int i;
	for(i=0;i<60;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=60;i<99;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 1;
		}
	}if (curPosition == 1){
	PORTA = step1to4;
	adcTimer(endBuffer);
	curPosition = 4;
	}else if (curPosition == 4){
	PORTA = step4to3;
	adcTimer(endBuffer);
	curPosition = 3;
	}else if (curPosition == 3){
	PORTA = step3to2;
	adcTimer(endBuffer);
	curPosition = 2;
	}else if (curPosition == 2){
	PORTA = step2to1;
	adcTimer(endBuffer);
	curPosition = 1;
}
}

void CCW_50_both(){
	int i;
	for(i=0;i<10;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=10;i<49;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 2;
		}
		else if(curPosition == 2){
			PORTA = step2to3;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 3;
		}
		else if(curPosition == 3){
			PORTA = step3to4;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 4;
		}
		else if(curPosition == 4){
			PORTA = step4to1;
			adcTimer(bothBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	if (curPosition == 1){
		PORTA = step1to2;
		adcTimer(endBuffer);
		curPosition = 2;
	}
	else if (curPosition == 2){
		PORTA = step2to3;
		adcTimer(endBuffer);
		curPosition = 3;
	}
	else if (curPosition == 3){
		PORTA = step3to4;
		adcTimer(endBuffer);
		curPosition = 4;
	}
	else if (curPosition == 4){
		PORTA = step4to1;
		adcTimer(endBuffer);
		curPosition = 1;
	}
}

void CCW_100_both(){
	int i;
	for(i=0;i<60;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=60;i<99;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(bothBufferHundredStep[i]);
			curPosition = 1;
		}
	}
	if (curPosition == 1){
		PORTA = step1to2;
		adcTimer(endBuffer);
		curPosition = 2;
	}
	else if (curPosition == 2){
		PORTA = step2to3;
		adcTimer(endBuffer);
		curPosition = 3;
	}
	else if (curPosition == 3){
		PORTA = step3to4;
		adcTimer(endBuffer);
		curPosition = 4;
	}
	else if (curPosition == 4){
		PORTA = step4to1;
		adcTimer(endBuffer);
		curPosition = 1;
	}
}

void CW_50_prev(){
	int i;
	for(i=0;i<10;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=10;i<50;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
}

void CW_100_prev(){
	int i;
	for(i=0;i<60;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=60;i<100;i++){
		if (curPosition == 1){
			PORTA = step1to4;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 4;
			}else if (curPosition == 4){
			PORTA = step4to3;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 3;
			}else if (curPosition == 3){
			PORTA = step3to2;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 2;
			}else if (curPosition == 2){
			PORTA = step2to1;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 1;
		}
	}
}

void CCW_50_prev(){
	int i;
	for(i=0;i<10;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=10;i<50;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 2;
		}
		else if(curPosition == 2){
			PORTA = step2to3;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 3;
		}
		else if(curPosition == 3){
			PORTA = step3to4;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 4;
		}
		else if(curPosition == 4){
			PORTA = step4to1;
			adcTimer(frontBufferFiftyStep[i]);
			curPosition = 1;
		}
	}
}

void CCW_100_prev(){
	int i;
	for(i=0;i<60;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 1;
		}
	}
	PORTB = 0xFF & forward;
	for (i=60;i<100;i++){
		if (curPosition == 1){
			PORTA = step1to2;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 2;
		}
		else if (curPosition == 2){
			PORTA = step2to3;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 3;
		}
		else if (curPosition == 3){
			PORTA = step3to4;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 4;
		}
		else if (curPosition == 4){
			PORTA = step4to1;
			adcTimer(frontBufferHundredStep[i]);
			curPosition = 1;
		}
	}
}



void Rotate_Stepper(int new_material, int next_material, int prevBuffer){
	listDropCount++;
	//sets the bufferFlag to 1 if no the objects are in consecutive trays (no stop is needed)
	if(((curStepperMaterial == white) & (new_material == black) & (next_material == white)) |
		((curStepperMaterial == white) & (new_material == black) & (next_material == white)) |
		((curStepperMaterial == white) & (new_material == aluminium) & (next_material == black)) |
		((curStepperMaterial == white) & (new_material == aluminium) & (next_material == steel)) |
		((curStepperMaterial == black) & (new_material == white) & (next_material == black)) |
		((curStepperMaterial == black) & (new_material == white) & (next_material == aluminium)) |
		((curStepperMaterial == black) & (new_material == steel) & (next_material == white)) |
		((curStepperMaterial == black) & (new_material == steel) & (next_material == aluminium)) |
		((curStepperMaterial == steel) & (new_material == white) & (next_material == black)) |
		((curStepperMaterial == steel) & (new_material == white) & (next_material == aluminium)) |
		((curStepperMaterial == steel) & (new_material == aluminium) & (next_material == black)) |
		((curStepperMaterial == steel) & (new_material == aluminium) & (next_material == steel)) |
		((curStepperMaterial == aluminium) & (new_material == black) & (next_material == white)) |
		((curStepperMaterial == aluminium) & (new_material == black) & (next_material == steel)) |
		((curStepperMaterial == aluminium) & (new_material == steel) & (next_material == white)) |
		((curStepperMaterial == aluminium) & (new_material == steel) & (next_material == aluminium)) |
		((curStepperMaterial == white) & (new_material == steel) & (next_material == black)) |
		((curStepperMaterial == black) & (new_material == aluminium) & (next_material == white)) |
		((curStepperMaterial == steel) & (new_material == black) & (next_material == aluminium)) |
		((curStepperMaterial == aluminium) & (new_material == white) & (next_material == steel))){
			bufferFlag = 1;
	}else{
			bufferFlag = 0;
	}
	if((bufferFlag == 0) & (prevBuffer ==0)){
		//white
		if(curStepperMaterial == white){
			if((PINE & 0b00100000) == 0b00100000){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				curStepperMaterial = white;
				PORTB = 0xFF & forward;
				return;
			}else if (new_material == black){	
				CW_100();
				curStepperMaterial = black;
				return;
			}else if (new_material == aluminium){
				CW_50();
				curStepperMaterial = aluminium;
				return;
			}else if (new_material == steel){
				CCW_50();			
				curStepperMaterial = steel;
				return;
			}
		}
		
		//black
		else if(curStepperMaterial == black){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_100();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				curStepperMaterial = black;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == aluminium){
				CCW_50();
				curStepperMaterial = aluminium;
				return;
				}else if (new_material == steel){
				CW_50();
				curStepperMaterial = steel;
				return;
			}
		}
		
		//steel
		else if(curStepperMaterial == steel){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_50();		
				curStepperMaterial = white;
				return;
			}else if (new_material == black){
				CCW_50();		
				curStepperMaterial = black;
				return;
			}else if (new_material == aluminium){
				CW_100();	
				curStepperMaterial = aluminium;
			}else if (new_material == steel){
				curStepperMaterial = steel;
				PORTB = 0xFF & forward;
				return;
			}
		}
		//aluminium
		else if (curStepperMaterial == aluminium){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CCW_50();
				curStepperMaterial = white;
				return;
			}else if (new_material == black){
				CW_50();
				curStepperMaterial = black;
				return;
			}else if (new_material == aluminium){
				curStepperMaterial = aluminium;
				PORTB = 0xFF & forward;
				return;
			}else if (new_material == steel){
				CW_100();
				curStepperMaterial = steel;
				return;
			}
		}
	}//end of no buffer, no prev buffer if
	else if((bufferFlag == 1) & (prevBuffer ==0)){
		//white
		if(curStepperMaterial == white){
			if((PINE & 0b00100000) == 0b00100000){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				curStepperMaterial = white;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == black){
				CW_100_end();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				CW_50_end();
				curStepperMaterial = aluminium;
				return;
				}else if (new_material == steel){
				CCW_50_end();
				curStepperMaterial = steel;
				return;
			}
		}
		
		//black
		else if(curStepperMaterial == black){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_100_end();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				curStepperMaterial = black;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == aluminium){
				CCW_50_end();
				curStepperMaterial = aluminium;
				return;
				}else if (new_material == steel){
				CW_50_end();
				curStepperMaterial = steel;
				return;
			}
		}
		
		//steel
		else if(curStepperMaterial == steel){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_50_end();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				CCW_50_end();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				CW_100_end();
				curStepperMaterial = aluminium;
				}else if (new_material == steel){
				curStepperMaterial = steel;
				PORTB = 0xFF & forward;
				return;
			}
		}
		//aluminium
		else if (curStepperMaterial == aluminium){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CCW_50_end();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				CW_50_end();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				curStepperMaterial = aluminium;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == steel){
				CW_100_end();
				curStepperMaterial = steel;
				return;
			}
		}
	}//end of next buffer, no prev buffer if
	else if((bufferFlag == 1) & (prevBuffer ==1)){
		//white
		if(curStepperMaterial == white){
			if((PINE & 0b00100000) == 0b00100000){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				curStepperMaterial = white;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == black){
				CW_100_both();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				CW_50_both();
				curStepperMaterial = aluminium;
				return;
				}else if (new_material == steel){
				CCW_50_both();
				curStepperMaterial = steel;
				return;
			}
		}
		
		//black
		else if(curStepperMaterial == black){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_100_both();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				curStepperMaterial = black;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == aluminium){
				CCW_50_both();
				curStepperMaterial = aluminium;
				return;
				}else if (new_material == steel){
				CW_50_both();
				curStepperMaterial = steel;
				return;
			}
		}
		
		//steel
		else if(curStepperMaterial == steel){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_50_both();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				CCW_50_both();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				CW_100_both();
				curStepperMaterial = aluminium;
				}else if (new_material == steel){
				curStepperMaterial = steel;
				PORTB = 0xFF & forward;
				return;
			}
		}
		//aluminium
		else if (curStepperMaterial == aluminium){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CCW_50_both();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				CW_50_both();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				curStepperMaterial = aluminium;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == steel){
				CW_100_both();
				curStepperMaterial = steel;
				return;
			}
		}
	}//end of both buffer if
	else if((bufferFlag == 0) & (prevBuffer == 1)){
		//white
		if(curStepperMaterial == white){
			if((PINE & 0b00100000) == 0b00100000){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				curStepperMaterial = white;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == black){
				CW_100_prev();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				CW_50_prev();
				curStepperMaterial = aluminium;
				return;
				}else if (new_material == steel){
				CCW_50_prev();
				curStepperMaterial = steel;
				return;
			}
		}
			
		//black
		else if(curStepperMaterial == black){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_100_prev();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				curStepperMaterial = black;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == aluminium){
				CCW_50_prev();
				curStepperMaterial = aluminium;
				return;
				}else if (new_material == steel){
				CW_50_prev();
				curStepperMaterial = steel;
				return;
			}
		}
			
		//steel
		else if(curStepperMaterial == steel){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CW_50_prev();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				CCW_50_prev();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				CW_100_prev();
				curStepperMaterial = aluminium;
				}else if (new_material == steel){
				curStepperMaterial = steel;
				PORTB = 0xFF & forward;
				return;
			}
		}
		//aluminium
		else if (curStepperMaterial == aluminium){
			if((PINE & 0x20) == 0x20){
				ADC_result = ADCL + (ADCH<<8);
				if(ADC_result < min_ADC){
					min_ADC = ADC_result;
				}
			}
			if (new_material == white){
				CCW_50_prev();
				curStepperMaterial = white;
				return;
				}else if (new_material == black){
				CW_50_prev();
				curStepperMaterial = black;
				return;
				}else if (new_material == aluminium){
				curStepperMaterial = aluminium;
				PORTB = 0xFF & forward;
				return;
				}else if (new_material == steel){
				CW_100_prev();
				curStepperMaterial = steel;
				return;
			}
		}
	}//end of no buffer, but prev buffer if
	
} // End Rotate_Stepper


void adcTimer(int count){
	/* The system clock is 8MHz. You can see the crystal oscillator (16MHz) which is a silver
	looking can on the board. You can use a pre-scaler on the system clock to lower the speed.
	The Timer runs on the CPU CLock which is a function of the system clock. You can also use
	a pre-scaler on the Timer, by 1, 8, 64, 256, or 1024 to lower the speed. The system clock
	has been pre-scaled by 2. This means it's running at half speed, 8MHz. See Technical manual
	for ATmega2560 (full manual) and lookup "16-bit Timer/Counter1".*/
	
	
	ADC_result = ADCL + (ADCH<<8);
	if(ADC_result < min_ADC){
		min_ADC = ADC_result;
	}
	
	
	/*Variable Declarations*/
	//int ADC_result;
	int i; /* keeps track of loop number */
	i = 0; /* initializess loop counter */
	
	TCCR3B |=_BV(WGM32); /* set WGM bits to 0100, see page 145
						Note WGM is spread over two register */
	OCR3A = 0x03E8; // Set Output Compare Register for 1000 cycles = 1ms
	
	TCNT3 = 0x0000; // Sets initial value of Timer Counter to 0x0000
	
	TIFR3 |= _BV(OCF3A); // clear the timer interrupt flag and begin new timing ... (ask question if confusing says so in lab doc)
	
	// Poll the timer to determine when the timer has reached 0x03E8
	
	while(i<count){
		ADC_result = ADCL + (ADCH<<8);
		if(ADC_result < min_ADC){
			min_ADC = ADC_result;
		}
		
		if((TIFR3 & 0x02) == 0x02){
			
			TIFR3 |= _BV(OCF3A); // clear interrupt flag by writing a ONE to the bit
			
			i++; // increment loop number
			
			//if(addToListFlag){
			//	addToList();
			//}
		} // end if
	} // end while
	return;
} // End of mtimer

void fastPWM(){
	TCCR0A = TCCR0A | _BV(WGM00) | _BV(WGM01);
	TCCR0A |= _BV(COM0A1); // or 0b10000000
	TCCR0B |= _BV(CS01); // or 0b00000100, prescales by 256
	OCR0A = 0b10011111; //dutycycle
}


ISR(INT2_vect){ //when ramp down button is pressed - set ramp down flag
	rampDownFlag = 1;
	EIFR |= _BV(INTF2);
}


ISR(INT3_vect){ //when an item triggers EX - we need to stop the belt and go to bucket stage
	PORTB = 0xFF & brake;
	STATE = 2;
	EIFR |= _BV(INTF3);
}

ISR(INT4_vect){
	startStop(0);
	int x = 0;
	int whiteDropCount = 0;
	int blackDropCount = 0;
	int steelDropCount = 0;
	int aluminiumDropCount = 0;
	int whiteBeltCount = 0;
	int blackBeltCount = 0;
	int steelBeltCount = 0;
	int aluminiumBeltCount = 0;
	int total;
	if(PORTB == brake){
		for(x=0;x<listDropCount;x++){
			switch(listArray[x]){
				//white
				case 1:
					whiteDropCount++;
				break;
				//black
				case 2:
					blackDropCount++;
				break;
				//steel
				case 3:
					steelDropCount++;
				break;
				//aluminium
				case 4:
					aluminiumDropCount++;
				break;	
			}
		}
		for(x=listDropCount;x<currentListCount;x++){
			switch(listArray[x]){
				//white
				case 1:
				whiteBeltCount++;
				break;
				//black
				case 2:
				blackBeltCount++;
				break;
				//steel
				case 3:
				steelBeltCount++;
				break;
				//aluminium
				case 4:
				aluminiumBeltCount++;
				break;	
			}
		}
		total = whiteDropCount + blackDropCount + steelDropCount + aluminiumDropCount;
		LCDClear();
		LCDWriteIntXY(5,1, whiteBeltCount, 2);
		LCDWriteIntXY(8,1, blackBeltCount, 2);
		LCDWriteIntXY(11, 1, steelBeltCount, 2);
		LCDWriteIntXY(14,1, aluminiumBeltCount, 2);
		LCDWriteStringXY(0,0,"BELT");
		LCDWriteStringXY(5,0, "W:");
		LCDWriteStringXY(8,0,"B:");
		LCDWriteStringXY(11,0,"S:");
		LCDWriteStringXY(14,0,"A:");
		adcTimer(20000);
		LCDWriteIntXY(5,1, whiteDropCount, 2);
		LCDWriteIntXY(8,1, blackDropCount, 2);
		LCDWriteIntXY(11, 1, steelDropCount, 2);
		LCDWriteIntXY(14,1, aluminiumDropCount, 2)
		LCDWriteIntXY(0,1, total, 2);
		LCDWriteStringXY(0,0,"TRAY");
		LCDWriteStringXY(5,0, "W:");
		LCDWriteStringXY(8,0,"B:");
		LCDWriteStringXY(11,0,"S:");
		LCDWriteStringXY(14,0,"A:");
	}else{
		LCDClear();
		LCDWriteStringXY(0,0,"RUNNING");
		adcTimer(1000);
	}
	EIFR |= _BV(INTF4);
}

ISR(INT5_vect){ //when an item passes OR - we need to add it to the list
	int material;
	if(min_ADC < aluminiumMax){
		material = aluminium;
		}else if(min_ADC < steelMax){
		material = steel;
		}else if(min_ADC < whiteMax){
		material = white;
		}else{
		material = black;
	}
	listArray[currentListCount] = material;
	min_ADC = 0b1111111111;
	currentListCount++;
	if(rampDownFlag == 1){
		enableRampDown = 1;
	}
	EIFR |= _BV(INTF5);
}



