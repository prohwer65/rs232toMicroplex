// Connect the interface to the PC with a 9-pin straight through serial cable; one end male, the other end female. 
// The UART is setup for 38400 BAUD which allows data to enter the interface faster that it can escape through the MPX protocol.
// Their are four types of data bytes that can be sent to the interface; Commands, Addresses, Channel value levels and Channel length.
// Their are two commands which are 0xAA and 0xA7. 
// 0xAA is used to tell the interface that the next byte will be a channel address and the byte after that will be level value for that channel.
// 0xA7 is used to program the channel length into the interface where the next byte is the number of channels of the MPX protocol. 
//     Channel length can be between 6 and 96 channels. Valid address are 6 to the programmed channel length. Valid channel levels ranges from 0 to 159. 
//     The light is off at 0 and all the way on at 159.
//

// To change a light level send 0xAA, then the address of the light you want to change (6-96), and then the level (0-159). 
// After ever data byte sent the interface automatically increments to the next channel address. This makes it possible to "mass program" sequential addresses with only telling the interface the address of the first.
// History of changes  -------------------------------------------------------
// 2024.03.02 pwr  updated codewords from AT90S to ATtiny naming convention.
//
//  Suggested features:
//  Add command to turn all channels to 0 or off
//  Add command(s) to ramp level changes gradually 1 to 3 secs.
//  Fade to black gradually 1 to 3 secs. 






#define UART_BAUD_RATE         38400            // baud rate
#define UART_BAUD_SELECT       (F_CPU/(UART_BAUD_RATE*16l)-1)

#define synclevel   0x00                        //Sync pulse DAC value
#define offlevel    0x60                        //Light OFF DAC value
#define maxlevel    255-offlevel                //Max light level from PC
#define maxch       96                          //Max # of channels

#include <avr\io.h>
#include <avr\signal.h>
#include <avr\interrupt.h>

unsigned char channels;                     //Number of Microplex channels
unsigned char maxfield;                     //Always 2x channels
unsigned char field;                        //keeps track of sync pulse & channel
unsigned char syncntr;                      //counter for microplex channel 0 sync (5ms)
unsigned char channelTarget[maxch+1];       //Channel Target level values during fade mode
unsigned char channelLevel[maxch+1];        //Channel level data values being sent out
unsigned char data;                         //temp for serial IQR handle
unsigned char addrflag;                     //Flags next RX Byte = address
unsigned char address;                      //RX IRQ channel position
unsigned char channelflag;                  //Flags next RX Byte = # of channels
unsigned char fadeflag;                     //Flags next RX Byte as fadeRate
unsigned char fadeRate;                     // number of secs remaining in the fadeRate
unsigned char fadetoblackflag;              //Flags next RX Byte as fadeRate for fade to black

//signed int delta;                           // sign byte between current level and target level

int main (void)
{
  cli();
  //Setup Ports
  DDRB = 0xFF;                                   //Set Port B as Output
  //Setup UART
  UBRRL = (char)UART_BAUD_SELECT;                // updated to ATtiny2312 naming convention
  UCSRB = 0x98;                                  //enable Rx, Rx IQR & Tx        // updated to ATtiny2312 naming convention
  //Setup Timer to send output signal
  TCCR1A = 0x00;
  TCCR1B = 0x09;                                 //direct ck & CTC1 (ATtiny232 calls the CTC1 WGM12) enable
  OCR1 = 2304;                                   //250uS Frame
  TIMSK = 0x40;                                  //Enable OC1 IRQ


  // need to setup a timer that occurs every second, but don't enable until a fade rate command is revieved 

  channels = 48;                                 //Default number of channels
  maxfield = 96;
  sei();
  for (;;);
}


SIGNAL(SIG_OUTPUT_COMPARE1A)
{
  if (field > maxfield) {                        //Test for end of channels
    if (syncntr <= 18) {                        //Build channel 0 5ms sync pulse
      syncntr++;
      PORTB = synclevel;
      return;
    }
    field = 0;                                  //Reset 1/2 channel counter
    syncntr = 0;                                //Reset 5ms sync pulse counter
  }
  else {
    if (!(field & 0x01)) 
      PORTB = synclevel;                      //DAC is Synclevel if field = even 
    else 
      PORTB = channelLevel [(field >> 1)] + offlevel; //DAC is channel data if field = odd
    field++;
  }
}


SIGNAL(SIG_UART_RECV)
{
  data = UDR;
  if (data >= 0xA7) {
    fadeflag=0;                    //Disable fade mode 
    if (data == 0xAA) {                   //Address Command
                                          // format:  0xAA, starting address, value(s) 
      addrflag = 1;                       //Set Flag for next RX Byte = address
      return;
    }
    if (data == 0xA7) {                   //Channel Length Program Command
                                          // format:  0xA7, # of channels
      channelflag = 1;                       //Set Flag for next RX Byte = # of channels
      return;
    }
    if ( data == 0xBB ) {                 // turn off all channels
                                          // format:  0xBB, nothing else follows
      for (i=0; i<channels; i++) {
        channelLevel[i] = 0;
      }
      return;
    }
    if ( data == 0xFB ) {                 // Fade to Black, all channels go to level 0
                                          // format:  0xFB, transRate in secs,
      for (i=0; i<channels; i++) {
        channelTarget[i] = 0;
        channelLevel[i]  = channelLevel[i]/ 2;
        fadetoblackflag  = 1;             // Set flag for next RX byte to transRate in secs
      }
      // enable 1 second timer starting now.  Assumes all Target levels will be sent in under 1secs
      return;
    }
    if ( data == 0xFA ) {                 // setup for slow fade to new levels
                                          // format:  0xFA, transRate in secs, starting address, value(s) 
      // initialize targetlvl to existing values
      // set
      for (i=0; i<channels; i++) {
        channelTarget[i] = channelLevel[i] ;
      }
      fadeflag=1;              // Set flag for next RX byte to be fadeRate
      // enable 1 second timer starting now.  Assumes all Target levels will be sent in under 1secs
      return;
    }
  }

  if (addrflag == 1) {
    addrflag = 0;
    if (data < channels)  {
      address = data;                     //Set address
    }
    return;
  }

  if ( fadetoblackflag ==1) {

    fadeRate = data+1;
    return;
  }

  if ( fadeflag ==1) {
    fadeRate = data+1;
    addrflag = 1;                         // Set Flag for next RX Byte = address 
    return;
  }

  if (channelflag == 1) {                    // change the number of channels
    channelflag = 0;
    if (data < 6 || data > maxch) {               //Test for valid # of channels
      return;                             
    }
    channels = data;                            //Program # of channels
    maxfield = data * 2;                        //Program # of fields
    return;
  }

  if (data > maxlevel)  {
    data = maxlevel;                            //Cap top level
  }

  if ( fadeRate ==1) {
    channelTarget[address] = data;                 //Load Target channel value into array
    // put initial new values in channelLevel
    //
    unsigned char currLevel = channelLevel[i];

    // need this complicated routine because working with unsigned values......
    if ( currLevel > data) {
      channelLevel[i] = currLevel + (( currLevel - data)/ fadeRate);
    } 

    // need this complicated routine because working with unsigned values......
    if ( currLevel < data) {
      channelLevel[i] = currLevel - (( data - currLevel )/ fadeRate);
    } 


  } else {
    channelLevel[address] = data;                 //Load channel value into array
  }

  if (address < channels - 1) {
    address++;                                  //Auto Increment address if not at max
  }
}


SIGNAL( a timer that occurs every 1s)
{
  if (fadeRate < 2 ) {
    fadeRate =0;
    for (i=0; i<channels; i++) {
      channelLevel[i] = channelTarget[i];
    }

    // stop the timer
    return;
  }

  for (i=0; i<channels; i++) {
    unsigned char currLevel = channelLevel[i] ;
    unsigned char targetlvl = channelTarget[i];

    // need this complicated routine because working with unsigned values......
    if ( currLevel > targetlvl) {
      channelLevel[i] = currLevel + (( currLevel - targetlvl)/ fadeRate);
    } 

    // need this complicated routine because working with unsigned values......
    if ( currLevel < targetlvl) {
      channelLevel[i] = currLevel - (( targetlvl - currLevel )/ fadeRate);
    } 

  }

  fadeRate--;
  return;

}



//:vim smarttab tabstop=4  expandtab autoindent shiftwidth=4
