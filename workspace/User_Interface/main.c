#include  <msp430g2553.h>
#include "four_bit_lcd.h"
#include <stdio.h>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos))) //push button

enum screenState{
	D_BOLUS_DOSAGE,
	D_BOLUS_TIME,
	D_FLOW_RATE,
	D_FACTORY_RESET,
	D_ALL_RESET,
	P_FLOW_RATE,
	P_BOLUS_COUNTDOWN,
	P_TOTAL_DELIVERED
};

enum buttons{
	NONE,
	UP,
	DOWN,
	BOLUS,
	RESET
};

volatile    buttons button    =   0;		//push button
volatile	int count	=	0;		//push button

volatile int bolus_dosage = 0;
volatile int bolus_mins = 60;
volatile int flow_rate = 0;


void main (void){

	WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer

    //push button code
    //setting P1.6
	P1SEL  &=  (~BIT6);    //  Set P1.6    SEL as  GPIO
	P1DIR  &=  (~BIT6);    //  Set P1.6    SEL as  Input
	P1IES  |=  (BIT6); //  Falling Edge    1   ->  0
	P1IFG  &=  (~BIT6);    //  Clear   interrupt   flag    for P1.6
	P1IE   |=  (BIT6); //  Enable  interrupt   for P1.6

	//setting P1.7
	P1SEL  &=  (~BIT7);    //  Set P1.6    SEL as  GPIO
	P1DIR  &=  (~BIT7);    //  Set P1.6    SEL as  Input
	P1IES  |=  (BIT7); //  Falling Edge    1   ->  0
	P1IFG  &=  (~BIT7);    //  Clear   interrupt   flag    for P1.6
	P1IE   |=  (BIT7); //  Enable  interrupt   for P1.6

	__enable_interrupt();  //  Enable  Global  Interrupts
	//end push button code



	screenState state = D_BOLUS_DOSAGE;
	char str[16];
	sprintf(str, "%d mL", bolus_dosage);
	print_screen("Bolus dosage:", str);

    // Main loop
    while (1){
    	switch(state){
    	case D_BOLUS_DOSAGE:
    		if (button==UP){
    			button = NONE;
    			bolus_dosage++;
    			str[0] = 0;
    			sprintf(str, "%d mL", bolus_dosage);
    			print_screen("Bolus dosage:", str);
    		} else if (button==DOWN){
    			button = NONE;
    			bolus_dosage--;
    			str[0] = 0;
    			sprintf(str, "%d mL", bolus_dosage);
    			print_screen("Bolus dosage:", str);
    		} else if (button==RESET){
    			button = NONE;
    			state = P_FLOW_RATE;
    			str[0] = 0;
    			sprintf(str, "%d mL/hour", flow_rate);
    			print_screen("Flow rate:", str);
    		} else if (button==BOLUS){
    			button = NONE;
    			state = D_BOLUS_TIME;
    			str[0] = 0;
    			sprintf(str, "%d minutes", bolus_mins);
    			print_screen("Bolus time:", str);
    		}

    		break;

    	case D_BOLUS_TIME:
    		if (button==UP){
    			button = NONE;

    		} else if (button==DOWN){
    			button = NONE;

    		} else if (button==RESET){
    			button = NONE;

    		} else if (button==BOLUS){
    			button = NONE;

    		}

    		break;

    	case D_FLOW_RATE:
    		if (button==UP){
    			button = NONE;

    		} else if (button==DOWN){
    			button = NONE;

    		} else if (button==RESET){
    			button = NONE;

    		} else if (button==BOLUS){
    			button = NONE;

    		}

    		break;

    	case D_FACTORY_RESET:
    		if(1==1){

    		}

    		break;

    	case D_ALL_RESET:
    		if(1==1){

    		}

    		break;

    	case P_FLOW_RATE:
    		if (button==UP){
    			button = NONE;

    		} else if (button==DOWN){
    			button = NONE;

    		} else if (button==RESET){
    			button = NONE;

    		} else if (button==BOLUS){
    			button = NONE;

    		}

    		break;

    	case P_BOLUS_COUNTDOWN:
    		if (button==UP){
    			button = NONE;

    		} else if (button==DOWN){
    			button = NONE;

    		} else if (button==RESET){
    			button = NONE;

    		} else if (button==BOLUS){
    			button = NONE;

    		}

    		break;

    	case P_TOTAL_DELIVERED:
    		if (button==UP){
    			button = NONE;

    		} else if (button==DOWN){
    			button = NONE;

    		} else if (button==RESET){
    			button = NONE;

    		} else if (button==BOLUS){
    			button = NONE;

    		}

    		break;

    	case default:
    		break;
    	}
   }
}

//  Port    1   interrupt   service routine
//  used with push button code
#pragma vector=PORT1_VECTOR
__interrupt void    Port_1(void)
{
	if (CHECK_BIT(P1IFG, 6)){
		//P1.6 pin triggered
		__delay_cycles(50000);	//debounce code
		button   =   UP;
		P1IFG  &=  (~BIT6);    //  P1.6    IFG clear
	} else if (CHECK_BIT(P1IFG, 7)) {
		//P1.7 pin triggered
		__delay_cycles(50000);	//debounce code
		button   =   DOWN;
		P1IFG  &=  (~BIT7);    //  P1.7    IFG clear
	}
}
