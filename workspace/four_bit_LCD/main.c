#include "four_bit_lcd.h"

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // stop watchdog timer
	lcd_init();
	send_string("Change ALL");
	send_command(0xC0);
	send_string("THE THINGS!");
	while(1){}
}
