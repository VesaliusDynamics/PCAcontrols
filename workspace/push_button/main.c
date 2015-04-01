#include <msp430.h> 

/*
 * main.c
 */

volatile    int flag    =   0;

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	P1SEL  &=  (~BIT6);    //  Set P1.6    SEL as  GPIO
	P1DIR  &=  (~BIT6);    //  Set P1.6    SEL as  Input
	P1IES  |=  (BIT6); //  Falling Edge    1   ->  0
	P1IFG  &=  (~BIT6);    //  Clear   interrupt   flag    for P1.6
	P1IE   |=  (BIT6); //  Enable  interrupt   for P1.6
	__enable_interrupt();  //  Enable  Global  Interrupts
	
	while(1)
	{
		if(flag   ==  1)
		{
			//   Do  Something
			flag =   0;
		}
	}

}

//  Port    1   interrupt   service routine
#pragma vector=PORT1_VECTOR
__interrupt void    Port_1(void)
{
	flag   =   1;
	P1IFG  &=  (~BIT6);    //  P1.6    IFG clear
}
