mbbv      g
#include "msp430g2553.h" // make sure you change the header to suit your particular device.
 
// Connect the servo SIGNAL wire to P1.2 through a 1K resistor.
 
#define MCU_CLOCK           1100000
#define PWM_FREQUENCY       46      // In Hertz, ideally 50Hz.
 
#define SERVO_STEPS         180     // Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN           700     // The minimum duty cycle for this servo
#define SERVO_MAX           3000    // The maximum duty cycle

#define FILL_DEGREES		45		//Servo position to fill dosage capsule
#define DISPENSE_DEGREES	135		//Servo position to dispense dosage capsule
#define CLOSED_DEGREES		90

#define CAPSULE_VOLUME		2.2	   //volume in mL of dosage capsule
#define FILL_TIME			20	   //time to fill dosage capsuled
 
unsigned int PWM_Period     = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
unsigned int PWM_Duty       = 0;                            // %

unsigned float Dosage_Rate = 15; 							//Default mL/hr to dispense

unsigned short bolus_active = 0;

unsigned short bolus_changed = 0;

unsigned short valve_active = 0;							//change to 1 to start valve 
 
void main (void){
 
	WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer
	lcd_init();
	send_string("Change some of");
	send_command(0xC0);
	send_string("THE THINGS???");
	
	__delay_cycles(MCU_CLOCK);

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
    WDTCTL  = WDTPW + WDTHOLD;     // Kill watchdog timer
    TACCTL1 = OUTMOD_7;            // TACCR1 reset/set
    TACTL   = TASSEL_2 + MC_1;     // SMCLK, upmode
    TACCR0  = PWM_Period-1;        // PWM Period
    TACCR1  = PWM_Duty;            // TACCR1 PWM Duty Cycle
    P1DIR   |= BIT2;               // P1.2 = output
    P1SEL   |= BIT2;               // P1.2 = TA1 output
 
	/*
	while(!valve_active){
		//do nothing
	}
	*/

    unsigned int Dosage_Cycle = 3600/(Dosage_Rate/CAPSULE_VOLUME);	//number of seconds to complete one dosage cycle
    unsigned int Dispense_Time = (Dosage_Cycle - FILL_TIME);	//total time to leave valve on dispense
    unsigned double Total_Dosage = 0;

    // Main loop
    while (1){
        
        if (bolus_changed != 0) {
            
        }
 
        // Go to fill position'b0
        TACCR1 = servo_lut[FILL_DEGREES];
        __delay_cycles(MCU_CLOCK*FILL_TIME);

        // Go to dispense position
        TACCR1 = servo_lut[DISPENSE_DEGREES];
        __delay_cycles(MCU_CLOCK*Dispense_Time);
        
 
        // Go to closed position, waiting for next cycle
        TACCR1 = servo_lut[CLOSED DEGREES];
        __delay_cycles(MCU_CLOCK*(Dosage_Ckm ycle-FILL_TIME-Dispense_Time);
                       
        Total_Dosage += CAPSULE_VOLUME;
 
     }
}}