#define UART_BAUD_RATE         38400        	// baud rate
#define UART_BAUD_SELECT       (F_CPU/(UART_BAUD_RATE*16l)-1)

#define synclevel	0x00						//Sync pulse DAC value
#define offlevel	0x60						//Light OFF DAC value
#define maxlevel	255-offlevel				//Max light level from PC
#define maxch		96							//Max # of channels

#include <avr\io.h>
#include <avr\signal.h>
#include <avr\interrupt.h>

unsigned char channels;						//Number of Microplex channels
unsigned char maxfield;						//Always 2x channels
unsigned char field;							//keeps track of sync pulse & channel
unsigned char syncntr;							//counter for microplex channel 0 sync (5ms)
unsigned char channel[maxch+1];				//Channel data values
unsigned char data;							//temp for serial IQR handle
unsigned char addrflag;						//Flags next RX Byte = address
unsigned char address;							//RX IRQ channel position
unsigned char chlength;						//Flags next RX Byte = # of channels



int main (void)
{
 cli();
  //Setup Ports
 DDRB = 0xFF;									//Set Port B as Output
  //Setup UART
 UBRR = (char)UART_BAUD_SELECT;
 UCR = 0x98;	 								//enable Rx, Rx IQR & Tx
  //Setup Timer
 TCCR1A = 0x00;
 TCCR1B = 0x09;									//dirrect ck & CTC1 enable
 OCR1 = 2304;									//250uS Frame
 TIMSK = 0x40;									//Enable OC1 IRQ
 
 channels = 48;									//Defualt number of channels
 maxfield = 96;
 sei();
 for (;;);
}


SIGNAL(SIG_OUTPUT_COMPARE1A)
{
 if (field > maxfield) {						//Test for end of channels
 	if (syncntr <= 18) {						//Build channel 0 5ms sync pulse
 		syncntr++;
 		PORTB = synclevel;
 		return;
 	}
 	field = 0;									//Reset 1/2 channel counter
 	syncntr = 0;								//Reset 5ms sync pulse counter
 }
 else {
 	if (!(field & 0x01)) 
		PORTB = synclevel;						//DAC is Synclevel if field = even 
 	else 
		PORTB = channel [(field >> 1)] + offlevel; //DAC is channel data if field = odd
 	field++;
 }
}


SIGNAL(SIG_UART_RECV)
{
 data = UDR;
 if (data >= 0xA7) {
 	if (data == 0xAA) {						//Address Command
 		addrflag = 1;							//Set Flag for next RX Byte = address
 		return;
 	}
 	if (data == 0xA7) {						//Channel Length Program Command
 		chlength = 1;							//Set Flag for next RX Byte = # of channels
 		return;
 	}
 }
 if (addrflag == 1) {
 	addrflag = 0;
 	if (data < channels) 
		address = data;							//Set address
 	return;
 }
 if (chlength == 1) {
  	chlength = 0;
 	if (data < 6 || data > maxch) 				//Test for valid # of channels
		return;								
 	channels = data;							//Program # of channels
 	maxfield = data * 2; 						//Program # of fields
 	return;
 }
 if (data > maxlevel) 
	data = maxlevel;							//Cap top level
 channel[address] = data;						//Load channel value into array
 if (address < channels - 1) 
	address++;									//Auto Increment address if not at max
}
