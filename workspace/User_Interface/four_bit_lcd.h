// Author : Manpreet Singh Minhas
// This file is for 4 bit mode LCD interfacing with msp430g2553 chip
// 16x2 LCD is used
//https://learningmsp430.wordpress.com/2013/11/16/16x2-lcd-interfacing-in-4-bit-mode/

#include  <msp430g2553.h>

#define SETDIR P1DIR = P1DIR | BIT3 | BIT0 | BIT1
#define DR P1OUT = P1OUT | BIT3 // define RS high
#define CWR P1OUT = P1OUT & (~BIT3) // define RS low
#define READ P1OUT = P1OUT | BIT0
// define Read signal R/W = 1 for reading
#define WRITE P1OUT = P1OUT & (~BIT0)
// define Write signal R/W = 0 for writing
#define ENABLE_HIGH P1OUT = P1OUT | BIT1
// define Enable high signal
#define ENABLE_LOW P1OUT = P1OUT & (~BIT1)
// define Enable Low signal
unsigned int i;
unsigned int j;
void delay(unsigned int k)
{
	for(j=0;j<=k;j++)
	{
		for(i=0;i<100;i++);
	}

}
void data_write(void)
{
	ENABLE_HIGH;
	delay(2);
	ENABLE_LOW;
}

void data_read(void)
{
	ENABLE_LOW;
	delay(2);
	ENABLE_HIGH;
}

void check_busy(void)
{
	P2DIR &= ~(BIT3); // make P1.3 as input
	while((P2IN&BIT3)==1)
	{
		data_read();
	}
	P2DIR |= BIT3; // make P1.3 as output
}

void send_command(unsigned char cmd)
{
	check_busy();
	WRITE;
	CWR;
	P2OUT = (P2OUT & 0xF0)|((cmd>>4) & 0x0F); // send higher nibble
	data_write(); // give enable trigger
	P2OUT = (P2OUT & 0xF0)|(cmd & 0x0F); // send lower nibble
	data_write(); // give enable trigger
}

void send_data(unsigned char data)
{
	check_busy();
	WRITE;
	DR;
	P2OUT = (P2OUT & 0xF0)|((data>>4) & 0x0F); // send higher nibble
	data_write(); // give enable trigger
	P2OUT = (P2OUT & 0xF0)|(data & 0x0F); // send lower nibble
	data_write(); // give enable trigger
}

void send_string(char *s)
{
	while(*s)
	{
		send_data(*s);
		s++;
	}
}

void lcd_init(void)
{
	//P1DIR |= 0xFF;
	//P1OUT &= 0x00;
	//push button code
	SETDIR;
	CWR;
	WRITE;
	ENABLE_LOW;
	//----------------
	P2DIR |= 0XFF;
	P2OUT &= 0XFF;
	send_command(0x33);
	send_command(0x32);
	send_command(0x28); // 4 bit mode
	send_command(0x0E); // clear the screen
	send_command(0x01); // display on cursor on
	send_command(0x06); // increment cursor
	send_command(0x80); // row 1 column 1
}

void print_screen(char *s1, char *s2){
	lcd_init();
	send_string(s1);
	send_command(0xC0);
	send_string(s2);
}
