#include  <msp430g2553.h>
#include "four_bit_lcd.h"
#include <stdio.h>
#include <string.h>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos))) //push button

#define MCU_CLOCK           1100000
#define PWM_FREQUENCY       46      // In Hertz, ideally 50Hz.

#define SERVO_STEPS         180     // Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN           700     // The minimum duty cycle for this servo
#define SERVO_MAX           3000    // The maximum duty cycle

#define FILL_DEGREES		50		//Servo position to fill dosage capsule
#define DISPENSE_DEGREES	135		//Servo position to dispense dosage capsule

#define DISPENSE_TIME       45     //time to dispense dosage capsule
#define MIN_FILL_TIME       30     //minimum time to fill dosage capsule

unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
unsigned int PWM_Duty       = 0;                            // %


typedef enum {
    D_BOLUS_DOSAGE,
    D_BOLUS_TIME,
    D_FLOW_RATE,
    D_FACTORY_RESET,
    D_ALL_RESET,
    P_FLOW_RATE,
    P_BOLUS_COUNTDOWN,
    P_TOTAL_DELIVERED
} screenState;

typedef enum {
    NONE,
    UP,
    DOWN,
    BOLUS,
    RESET
} buttons;

typedef enum {
    FILL,
    DISPENSE
} valveState;

typedef enum {
    BOLUS_MINS,
    BOLUS_DOSAGE,
    FLOW_RATE,
    TOTAL_DELIVERED
} printerType;

//forward declarations----------
int print(printerType, int);
int printBolusCountdown();
int deliverBolusDosage();
int factoryReset();
int recalculate_times();

//------------------------------


volatile    buttons button    =   NONE;		//push button
volatile	int count	=	0;		//push button
volatile    valveState valve = FILL;

const int DEFAULT_BOLUS_DOSAGE = 0;
const int DEFAULT_BOLUS_MINS = 60;
const int DEFAULT_FLOW_RATE = 0;
//const int DEFAULT_FLOW_RATE = 15;
const float CAPSULE_VOLUME = 0.85;		//volume in mL of dosage capsule

int bolus_dosage = DEFAULT_BOLUS_DOSAGE;
int bolus_mins = DEFAULT_BOLUS_MINS;
int flow_rate = DEFAULT_FLOW_RATE;
volatile float total_delivered = 0;

volatile int bolus_active = 0;

const int MAX_BOLUS_DOSAGE = 100; 	//maximum allowable bolus dosage (in mL)
const int MAX_BOLUS_MINS = 480;		//maximum allowable minutes between bolus dosages
const int MINIMUM_BOLUS_MINS = 10;	//minimum amount of time between bolus dosages
const int MAX_FLOW_RATE = 20;		//maximum allowable primary flow rate (mL/hour)
const int MIN_FLOW_RATE = 0;		//minimum allowable primary flow rate (mL/hour)

volatile float current_time = 0;
volatile float next_valve_change = 0;
volatile int bolus_countdown = 0;		//The number of seconds until the patient can administer the bolus dosage again
volatile int bolus_countdown_prev = 0;

//flags for changed state
volatile int flow_rate_changed = 1;
volatile int bolus_dosage_changed = 0;
volatile int dosage_cycle = 0;
volatile float fill_time = 0;

volatile int timercount = 0;
volatile int max_bolus_count = 0;
volatile int bolus_count = 0;
volatile int bolus_activated = 0;

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
/* itoa:  convert n to characters in s */
void itoa(int x, char s[])
{
//http://stackoverflow.com/questions/190229/where-is-the-itoa-function-in-linux
    int i, sign;
    int n = x;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

int print(printerType x, int val){

    char num[4];
    char str[16];

    switch (x) {


        case BOLUS_MINS:
            itoa(val, num);
            strcpy(str, num);
            strcat(str, " minutes");
            print_screen("Bolus time:", str);
            break;

        case BOLUS_DOSAGE:
            itoa(val, num);
            strcpy(str, num);
            strcat(str, " mL");
            print_screen("Bolus dosage:", str);
            break;

        case FLOW_RATE:
            itoa(val, num);
            strcpy(str, num);
            strcat(str, " mL/hr");
            print_screen("Flow rate:", str);
            break;

        case TOTAL_DELIVERED:
            itoa(val, num);
            strcpy(str, num);
            strcat(str, " mL");
            print_screen("Total delivered:", str);
            break;

        default:
            break;
    }

    return 0;
}

void main (void){
    
    WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer

    //push button code
    {
        //setting P1.4
        P1SEL  &=  (~BIT4);    //  Set P1.4    SEL as  GPIO
        P1DIR  &=  (~BIT4);    //  Set P1.4    SEL as  Input
        P1IES  |=  (BIT4); //  Falling Edge    1   ->  0
        P1IFG  &=  (~BIT4);    //  Clear   interrupt   flag    for P1.4
        P1IE   |=  (BIT4); //  Enable  interrupt   for P1.4
        
        //setting P1.5
        P1SEL  &=  (~BIT5);    //  Set P1.5    SEL as  GPIO
        P1DIR  &=  (~BIT5);    //  Set P1.5    SEL as  Input
        P1IES  |=  (BIT5); //  Falling Edge    1   ->  0
        P1IFG  &=  (~BIT5);    //  Clear   interrupt   flag    for P1.5
        P1IE   |=  (BIT5); //  Enable  interrupt   for P1.5
        
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
        P1IFG  &=  (~BIT7);    //  Clear   interrupt   flag    for P1.7
        P1IE   |=  (BIT7); //  Enable  interrupt   for P1.7
    }
    //end push button code
    
    
    //setting up timer A0
    TA0CCR0 = 12000;   //sets counter limit, should interrupt every .05sec
    TA0CCTL0 = 0x10;       //enable timer interrupts
    TA0CTL = TASSEL_1 + MC_1;   //uses 12kHz clock as source for counting
    
    unsigned int servo_stepval, servo_stepnow;
    unsigned int servo_lut[ SERVO_STEPS+1 ];
    unsigned int i;
    
    // Calculate the step value and define the current step, defaults to minimum.
    servo_stepval   = ( (SERVO_MAX - SERVO_MIN) / SERVO_STEPS );
    servo_stepnow   = SERVO_MIN;
    
    // Fill up the LUT
    for (i = 0; i < SERVO_STEPS; i++) {
        servo_stepnow += servo_stepval;
        servo_lut[i] = servo_stepnow;
    }
    
    // Setup the PWM, etc.
    TACCTL1 = OUTMOD_7;            // TACCR1 reset/set
    TACTL   = TASSEL_2 + MC_1;     // SMCLK, upmode
    TACCR0  = PWM_Period-1;        // PWM Period
    TACCR1  = PWM_Duty;            // TACCR1 PWM Duty Cycle
    P1DIR   |= BIT2;               // P1.2 = output
    P1SEL   |= BIT2;               // P1.2 = TA1 output
    __enable_interrupt();  //  Enable  Global  Interrupts
    
    //making printers
    printerType BM = BOLUS_MINS;
    printerType BD = BOLUS_DOSAGE;
    printerType FR = FLOW_RATE;
    printerType TD = TOTAL_DELIVERED;
    
    screenState state = D_BOLUS_DOSAGE;
    print(BD, bolus_dosage);
    TACCR1 = servo_lut[FILL_DEGREES];
    
    
    // Main loop
    while (1){
        switch(state){
            case D_BOLUS_DOSAGE:
                if (button==UP){
                    button = NONE;
                    if (bolus_dosage < MAX_BOLUS_DOSAGE){
                        bolus_dosage++;
                    }
                    print(BD, bolus_dosage);
                } else if (button==DOWN){
                    button = NONE;
                    if (bolus_dosage > 0){
                        bolus_dosage-=1;
                    }
                    print(BD, bolus_dosage);
                } else if (button==RESET){
                    button = NONE;
                    state = P_FLOW_RATE;
                    print(FR,flow_rate);

                } else if (button==BOLUS){
                    button = NONE;
                    state = D_BOLUS_TIME;
                    print(BM,bolus_mins);
                }
                
                break;
                
            case D_BOLUS_TIME:
                if (button==UP){
                    button = NONE;
                    if (bolus_mins < MAX_BOLUS_MINS - 5){
                        bolus_mins = bolus_mins + 5;
                    } else {
                        bolus_mins = MAX_BOLUS_MINS;
                    }
                    print(BM,bolus_mins);
                    
                } else if (button==DOWN){
                    button = NONE;
                    if (bolus_mins > MINIMUM_BOLUS_MINS + 5){
                        bolus_mins = bolus_mins - 5;
                    } else {
                        bolus_mins = MINIMUM_BOLUS_MINS;
                    }
                    print(BM,bolus_mins);
                    
                } else if (button==RESET){
                    button = NONE;
                    state = P_FLOW_RATE;
                    print(FR,flow_rate);
                    
                } else if (button==BOLUS){
                    button = NONE;
                    state = D_FLOW_RATE;
                    print(FR,flow_rate);
                    
                }
                
                break;
                
            case D_FLOW_RATE:
                if (button==UP){
                    button = NONE;
                    if (flow_rate < MAX_FLOW_RATE){
                        flow_rate++;
                    } else {
                        flow_rate = MAX_FLOW_RATE;
                    }
                    flow_rate_changed = 1;
                    print(FR,flow_rate);
                    
                } else if (button==DOWN){
                    button = NONE;
                    if (flow_rate > MIN_FLOW_RATE){
                        flow_rate--;
                    } else {
                        flow_rate = MIN_FLOW_RATE;
                    }
                    flow_rate_changed = 1;
                    print(FR,flow_rate);
                    
                } else if (button==RESET){
                    button = NONE;
                    state = P_FLOW_RATE;
                    print(FR,flow_rate);
                    
                } else if (button==BOLUS){
                    button = NONE;
                    state = D_FACTORY_RESET;
                    print_screen("To factory reset", "press RESET now");
                    
                }
                
                break;
                
            case D_FACTORY_RESET:
                if(button==RESET){
                    button = NONE;
                    factoryReset();
                    state = D_ALL_RESET;
                    flow_rate_changed = 1;
                    print_screen("All values reset", "Press any key");
                } else if (button==UP || button==DOWN || button==BOLUS){
                    button = NONE;
                    state = D_BOLUS_DOSAGE;
                    print(BD, bolus_dosage);
                }
                
                break;
                
            case D_ALL_RESET:
                if(button!=NONE){
                    button = NONE;
                    state = D_BOLUS_DOSAGE;
                    print(BD, bolus_dosage);
                }
                
                break;
                
            case P_FLOW_RATE:
                if (button==UP){
                    button = NONE;
                    state = P_TOTAL_DELIVERED;
                    int y = (int)(total_delivered+0.5);
                    print(TD,y);
                    //print(TD, total_delivered);
                    
                } else if (button==DOWN){
                    button = NONE;
                    state = P_BOLUS_COUNTDOWN;
                    printBolusCountdown();
                    
                } else if (button==RESET){
                    button = NONE;
                    state = D_BOLUS_DOSAGE;
                    print(BD, bolus_dosage);
                    
                } else if (button==BOLUS){
                    button = NONE;
                    state = P_BOLUS_COUNTDOWN;
                    deliverBolusDosage();
                    printBolusCountdown();
                    
                }
                
                break;
                
            case P_BOLUS_COUNTDOWN:
                if (button==UP){
                    button = NONE;
                    state = P_FLOW_RATE;
                    print(FR,flow_rate);
                    
                } else if (button==DOWN){
                    button = NONE;
                    state = P_TOTAL_DELIVERED;
                    int y = (int)(total_delivered+0.5);
                    print(TD,y);
                    //print(TD, total_delivered);
                    
                    
                } else if (button==RESET){
                    button = NONE;
                    state = D_BOLUS_DOSAGE;
                    print(BD, bolus_dosage);
                    
                } else if (button==BOLUS){
                    button = NONE;
                    state = P_BOLUS_COUNTDOWN;
                    deliverBolusDosage();
                    printBolusCountdown();
                    
                }
                
                break;
                
            case P_TOTAL_DELIVERED:
                if (button==UP){
                    button = NONE;
                    state = P_BOLUS_COUNTDOWN;
                    printBolusCountdown();
                    
                } else if (button==DOWN){
                    button = NONE;
                    state = P_FLOW_RATE;
                    print(FR,flow_rate);
                    
                } else if (button==RESET){
                    button = NONE;
                    state = D_BOLUS_DOSAGE;
                    print(BD, bolus_dosage);
                    
                } else if (button==BOLUS){
                    button = NONE;
                    state = P_BOLUS_COUNTDOWN;
                    deliverBolusDosage();
                    printBolusCountdown();
                    
                }
                
                break;
                
            default:
                break;
        }
        
        if (flow_rate_changed){
            recalculate_times();
        }
        
        
        if (state == P_FLOW_RATE || state == P_BOLUS_COUNTDOWN || state == P_TOTAL_DELIVERED) {
            
            switch (valve) {
                case FILL:
                    if(bolus_activated){
                    	next_valve_change = DISPENSE_TIME + MIN_FILL_TIME;
                    	bolus_activated = 0;
                    }
                    if (current_time >= next_valve_change) {
                        TACCR1 = servo_lut[DISPENSE_DEGREES];
                        current_time = 0;
                        next_valve_change = DISPENSE_TIME;
                        valve = DISPENSE;
                    }
                    break;
                    
                case DISPENSE:
                    if (current_time >= next_valve_change) {
                        TACCR1 = servo_lut[FILL_DEGREES];
                        total_delivered += CAPSULE_VOLUME;
                        if (state == P_TOTAL_DELIVERED) {
                            int y = (int)(total_delivered+0.5);
                            print(TD,y);
                        }
                        if (bolus_active) {
                            next_valve_change = current_time +MIN_FILL_TIME;
                            bolus_count++;
                            if (bolus_count>=max_bolus_count) {
                                bolus_count = 0;
                                bolus_active = 0;
                            }
                        }else{
                            next_valve_change = current_time + fill_time;
                        }
                        valve = FILL;
                    }
                    
                    break;
                    
                    
                default:
                    break;
            }
            
        }
        if(state==P_BOLUS_COUNTDOWN && (bolus_countdown/60)<bolus_countdown_prev ){
            printBolusCountdown();
        }
    }
}


int printBolusCountdown(){
    
    int minutes_left = bolus_countdown/60;
    if (bolus_countdown<60 && bolus_countdown>0) {
        minutes_left = 1;
    }
    bolus_countdown_prev = minutes_left;
    
    char minutes[16] = 0;
    if (minutes_left>=10){
        minutes[0] = minutes_left/10 + '0';
        minutes[1] = minutes_left%10 + '0';
        minutes[2] = '\0';
    }else{
        minutes[0]= minutes_left + '0';
        minutes[1]='\0';
    }
    char min[5] = " min";
    min[4] = '\0';
    strcat(minutes,min);
    print_screen("Bolus countdown:", minutes);
    
    return 0;
    
}

int deliverBolusDosage(){
    if (bolus_active == 0 && bolus_countdown == 0) {
        bolus_activated = 1;
        bolus_active = 1;
        bolus_countdown = bolus_mins*60;
        bolus_countdown_prev = bolus_mins;
    }else{
        //do nothing
    }
    
    return 0;
}

int factoryReset(){
    bolus_dosage = DEFAULT_BOLUS_DOSAGE;
    bolus_mins = DEFAULT_BOLUS_MINS;
    flow_rate = DEFAULT_FLOW_RATE;
    
    return 0;
}

int recalculate_times(){

    if(flow_rate>0){
    	float temp = flow_rate/CAPSULE_VOLUME;
    	fill_time = 3600.0/temp;
    	fill_time = fill_time - DISPENSE_TIME;
    }
    if (bolus_dosage > 0) {
    	float temp = bolus_dosage/CAPSULE_VOLUME;
    	float temp2 = flow_rate/CAPSULE_VOLUME;
    	float temp3 = (float)MIN_FILL_TIME+DISPENSE_TIME;
    	temp2 = 3600/temp2;
    	temp2 = temp3/temp2;
    	temp2++;
    	temp = temp*temp2;
    	max_bolus_count = (int)(temp+0.5);

    }
    bolus_countdown = 0;
    bolus_active = 0;
    flow_rate_changed = 0;

    return 0;



}

//  Port    1   interrupt   service routine
//  used with push button code
#pragma vector=PORT1_VECTOR
__interrupt void    Port_1(void)
{
    if (CHECK_BIT(P1IFG, 4)){
        
        
        P1IFG &= ~BIT4;      // Clear the flag
  	P1IE &= ~BIT4;       // Temporarily disable the interrupt (for 8ms)
	button   =   BOLUS;
  	IFG1 &= ~WDTIFG;  //clear any existing WDT Interrupt flags
	WDTCTL = WDTPW | WDTSSEL | WDTTMSEL | WDTCNTCL | WDT_MDLY_8; //config WDT for 8ms
	IE1 |= WDTIE; //activate WDT interrupt
  
    } else if (CHECK_BIT(P1IFG, 5)) {
        P1IFG &= ~BIT5;      // Clear the flag
  	P1IE &= ~BIT5;       // Temporarily disable the interrupt (for 8ms)
	button   =   RESET;
  	IFG1 &= ~WDTIFG;  //clear any existing WDT Interrupt flags
	WDTCTL = WDTPW | WDTSSEL | WDTTMSEL | WDTCNTCL | WDT_MDLY_8; //config WDT for 8ms
	IE1 |= WDTIE; //activate WDT interrupt
    } else if (CHECK_BIT(P1IFG, 6)){
        P1IFG &= ~BIT6;      // Clear the flag
  	P1IE &= ~BIT6;       // Temporarily disable the interrupt (for 8ms)
	button   =   UP;
  	IFG1 &= ~WDTIFG;  //clear any existing WDT Interrupt flags
	WDTCTL = WDTPW | WDTSSEL | WDTTMSEL | WDTCNTCL | WDT_MDLY_8; //config WDT for 8ms
	IE1 |= WDTIE; //activate WDT interrupt
        
    } else if (CHECK_BIT(P1IFG, 7)) {
        P1IFG &= ~BIT7;      // Clear the flag
  	P1IE &= ~BIT7;       // Temporarily disable the interrupt (for 8ms)
	button   = DOWN;
  	IFG1 &= ~WDTIFG;  //clear any existing WDT Interrupt flags
	WDTCTL = WDTPW | WDTSSEL | WDTTMSEL | WDTCNTCL | WDT_MDLY_8; //config WDT for 8ms
	IE1 |= WDTIE; //activate WDT interrupt
    }
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
{
  
  IE1 &= ~WDTIE;                   /* disable interrupt */
  IFG1 &= ~WDTIFG;                 /* clear interrupt flag */
  WDTCTL = WDTPW | WDTHOLD; //disable WDT
  P1IE = BIT4 | BIT5 | BIT6 | BIT7; 
  
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void){


    timercount++;
    if (timercount >= 44) {
        current_time = current_time +1;
        if(bolus_countdown>0){
            bolus_countdown--;
        }
        timercount = 0;
    }

}
