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




// Processor frequency.
//     This will define a symbol, F_CPU, in all source code files equal to the
//     processor frequency. You can then use this symbol in your source code to
//     calculate timings. Do NOT tack on a 'UL' at the end, this will be done
//     automatically to create a 32-bit value in your source code.
#define F_CPU 1000000UL
//#define F_CPU   9216000

//   C:\Program Files\Microchip\xc8\v2.46\avr\avr\include
//#include <util\setbaud.h>
#include <avr\io.h>
//#include <avr\signal.h>
//#include <bits\signal.h>
#include <avr\interrupt.h>



#define UART_BAUD_RATE 38400  // baud rate
#define UART_BAUD_SELECT (F_CPU / (UART_BAUD_RATE * 16L) - 1)

#define SYNCLEVEL 0x00           //Sync pulse DAC value
#define OFFLEVEL 0x60            //Light OFF DAC value
#define MAXLEVEL 255 - OFFLEVEL  //Max light level from PC
#define MAXCHANNELS 48           //Max # of Microplex channels  (ATtiny2313 can handle 48, ATtiny4313 can handle 96)


unsigned char channels=MAXCHANNELS;                        //Number of Microplex channels
unsigned char maxfield=MAXCHANNELS*2;                        //Always 2x channels
unsigned char field=0;                           //keeps track of sync pulse & channel
unsigned char syncntr=0;                         //counter for microplex channel 0 sync (5ms)
unsigned char channelTarget[MAXCHANNELS + 1];  //Channel Target level values during fade mode
unsigned char channelLevel[MAXCHANNELS + 1];   //Channel level data values being sent out
unsigned char data=0;                            //temp for serial IQR handle
unsigned char address=0;                         //RX IRQ channel position
unsigned char fadeRate=0;                        // number of 500 msecs remaining in the fadeRate
//
unsigned char addrflag=0;         //Flags next RX Byte = address
unsigned char channelflag=0;      //Flags next RX Byte = # of channels
unsigned char fadeflag=0;         //Flags next RX Byte as fadeRate
unsigned char fadetoblackflag=0;  //Flags next RX Byte as fadeRate for fade to black
unsigned char toggle=0;   // pwr debug
#define  buttonPin  6    // pwr debug  physical pin 8
#define  rampPin       7  // pwr debug   physical pin 9  indicates when a Ramp or Fade is in progess
#define  pulsePin     8  // pwr debug physical pin 11, 

//signed int delta;                           // sign byte between current level and target level
unsigned char calculateFadeRate(unsigned char currlevel, unsigned char targetlevel, unsigned char fadeRate);


void loop() {
  // put your main code here, to run repeatedly:
  
  int button = digitalRead( buttonPin);
  if ( button == 0) {
    
    fadeRate = 25; //  data + 1;
    for (int i = 0; i < channels; i++) {
      channelTarget[i] = 0xFF;
      channelLevel[i] = calculateFadeRate(0 , 0xFF, fadeRate);
    }
    TIMSK |= 0x40;  // enable timer1
    digitalWrite(rampPin, HIGH);

  }
  //digitalWrite(rampPin, LOW);
  //delay(1000);
  //digitalWrite(rampPin, HIGH);
  //delay(1000);

    
}

void setup(void) {
  cli();
  //Setup Ports
  DDRB = 0xFF;  //Set Port B as Output
  //Setup UART
  UBRRL = (char)UART_BAUD_SELECT;  // updated to ATtiny2312 naming convention
  UCSRB = 0x98;                    //enable Rx, Rx IQR & Tx        // updated to ATtiny2312 naming convention
  //Setup Timer to send output signal
  //TCCR1A = 0x00;
  //TCCR1B = 0x09;                                 //direct ck & CTC1 (ATtiny232 calls the CTC1 WGM12) enable
  //OCR1B = 2304;                                   //250uS Frame
  //TIMSK = 0x40;                                  //Enable OC1 IRQ


  // Clkperiod is 1us  1MHz clock.
  //
  //
  // Setup Timer0 to send output signal
  // OC0A and OC0B disconnected
  // Timer0 with the clock freq = /256 CS0-2:0 = 0b100
  // Enable Clear Timer on Compare (CTC)
  // 9 * 256 * clkperiod  = 250us
  TCCR0A = 0x02;  //Clear Timer0 on Compare (CTC) WGM0-2:0 = 0b010
  TCCR0B = 0x03;  // Clock Select CS0-2:0 = 0xb011 or clk/64  (0b100 or clk/256)
  OCR0A = 2;      // 2 * 64 * clkperiod  = 256u Frame
  TIMSK = 0x01;   //Enable Timer/Counter0 Output Compare Match A (do I need to set the I-bit in Status Register?)


  // Setup timer1 to control fades
  // OC1A and OC1B disconnected
  // timer1 will now be used for the 500ms interrupt, using /1024 clock select bit
  // 450 * 1024 * clkperiod = 500ms
  // need to setup a timer that occurs every 500ms, but don't enable until a fade rate command is revieved
  TCCR1A = 0x00;  // OC1A and OC1B disconnected, WGM1-1:0 = 0b00
  TCCR1B = 0x0d;  // Clear Timer on Compare (CTC)  and clk / 1024 Clock Select CS1-2:0 = 0b101 , WGM1-3:0 = 0b0100
  OCR1A = 450;   // 500ms Frame
  // OCR1AH = 0x11; 0CR1AL = 0x94
  // to enable timer1 interrupt, TIMSK bit 6, but don't change the others bits.
  // TIMSK |=  0x40;  // enable timer1
  // TIMSK &=  0xbf;  // disable timer1

  //SREG = SREG | 0x80;   // enable Global Interrupts I bit???.

  GTCCR = 0x01;            // Reset the clock precalers for both T0 and T1
  channels = 24;     //MAXCHANNELS;  //Default number of channels
  maxfield = channels * 2;  // always 2x channels
  //pinMode(buttonPin, INPUT_PULLUP);   // pwr debug
  pinMode(rampPin, OUTPUT);   //pwr debug
  digitalWrite(rampPin, LOW);
  // pinMode(pulsePin, OUTPUT);   //pwr debug

  unsigned char i;
  for (i = 0; i < channels; i++) {
      channelLevel[i] = 0;
      channelTarget[i] = 0;
  }

  sei();
  for (;;)
    ;
}



ISR(TIMER0_COMPA_vect)  //tSIG_OUTPUT_COMPARE0A)    //occurs every 250us
{
  // digitalWrite( pulsePin, (toggle++&0x1) );       // pwr debug
  if (field > maxfield) {  //Test for end of channels
    if (syncntr <= 18) {   //Build 5ms sync pulse  ( 19*250us)
      syncntr++;
      PORTB = SYNCLEVEL;
      return;
    }
    PORTB = SYNCLEVEL;
    
    field = 0;    //Reset 1/2 channel counter
    syncntr = 0;  //Reset 5ms sync pulse counter
  } else {
    if (!(field & 0x01))  // Odd or Even; Sync pulse or level pulse?
      PORTB = SYNCLEVEL;  //DAC is SYNCLEVEL if field = even
    else
      PORTB = channelLevel[(field >> 1)] + OFFLEVEL;  //DAC is channel data if field = odd
    field++;
  }
}


ISR( USART_RX_vect )  //  SIG_UART_RECV)
{
  int i;
  data = UDR;
  if (data >= 0xA7) {
    // Command parsing
    //
    if (data == 0xAA) {  //Address Command
                         // format:  0xAA, starting address, value(s)
      addrflag = 1;      //Set Flag for next RX Byte = address
      return;
    }
    if (data == 0xA7) {  //Channel Length Program Command
                         // format:  0xA7, # of channels
      channelflag = 1;   //Set Flag for next RX Byte = # of channels
      return;
    }
    if (data == 0xBB) {  // turn off all channels
                         // format:  0xBB, nothing else follows
      for (i = 0; i < channels; i++) {
        channelLevel[i] = 0;
      }
      return;
    }
    if (data == 0xFB) {     // Fade to Black, all channels go to level 0
                            // format:  0xFB, transRate in 500 msecs,
      fadetoblackflag = 1;  // Set flag for next RX byte to transRate in 500 msecs
      return;
    }
    if (data == 0xFA) {  // setup for slow fade to new levels
                         // format:  0xFA, transRate in 500 msecs, starting address, value(s)
      // initialize targetlvl to existing values
      for (i = 0; i < channels; i++) {
        channelTarget[i] = channelLevel[i];
      }
      fadeflag = 1;  // Set flag for next RX byte to be fadeRate

      return;
    }
  }  // end of command parsing

  // data should contain either a starting address, level, fade time.
  if (addrflag == 1) {
    addrflag = 0;
    if (data < channels) {
      address = data;  //Set address
    }
    return;
  }

  if (fadetoblackflag == 1) {
    fadetoblackflag = 0;  // Set flag for next RX byte to transRate in 500 msecs

    fadeRate = data + 1;
    for (i = 0; i < channels; i++) {
      channelTarget[i] = 0;
      channelLevel[i] = calculateFadeRate(channelLevel[i], 0, fadeRate);
    }
    TIMSK |= 0x40;  // enable timer1
    digitalWrite(rampPin, HIGH);
    return;
  }

  if (fadeflag == 1) {
    fadeflag = 0;  //Disable fade mode
    fadeRate = data + 1;
    addrflag = 1;   // Set Flag for next RX Byte = address
    TIMSK |= 0x40;  // enable timer1
    digitalWrite(rampPin, HIGH);
    return;
  }

  if (channelflag == 1) {  // change the number of channels
    channelflag = 0;
    if (data < 6 || data > MAXCHANNELS) {  //Test for valid # of channels
      return;
    }
    channels = data;      //Program # of channels
    maxfield = data * 2;  //Program # of fields
    return;
  }

  if (data > MAXLEVEL) {
    data = MAXLEVEL;  //Cap top level
  }

  if (fadeRate > 0) {
    channelTarget[address] = data;  //Load Target channel value into array
    channelLevel[address] = calculateFadeRate(channelLevel[address], data, fadeRate);
    //TIMSK |= 0x40;  // enable timer1

  } else {
    channelLevel[address] = data;  //Load channel value into array
  }

  if (address < channels - 1) {
    address++;  //Auto Increment address if not at max
  }
}

//Interrupt Service Routine
ISR(TIMER1_COMPA_vect)  // SIG_OUTPUT_COMPARE1A )   //occurs every 500ms when timer1 is enabled
{
  int i;
  if (fadeRate < 2) {
    fadeRate = 0;
    for (i = 0; i < channels; i++) {
      channelLevel[i] = channelTarget[i];
    }

    TIMSK &= 0xbf;  // disable timer1
    digitalWrite(rampPin, LOW);    // turn the LED off when not ramping or fading
    // stop the timer1
    return;
  }

  for (i = 0; i < channels; i++) {
    unsigned char currLevel = channelLevel[i];
    unsigned char targetlvl = channelTarget[i];

    channelLevel[i] = calculateFadeRate(currLevel, targetlvl, fadeRate);
  }

  fadeRate--;
  return;
}


// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// Functions
//
unsigned char calculateFadeRate(unsigned char currLevel, unsigned char targetlvl, unsigned char fadeRate) {

  unsigned char newlevel = currLevel;

  if (fadeRate < 2) {
    fadeRate = 0;
    return newlevel;
  }
  // need this complicated routine because working with unsigned values......
  if (currLevel > targetlvl) {
    newlevel = currLevel + ((currLevel - targetlvl) / fadeRate);
  }

  // need this complicated routine because working with unsigned values......
  if (currLevel < targetlvl) {
    newlevel = currLevel - ((targetlvl - currLevel) / fadeRate);
  }

  return newlevel;
}

//:vim smarttab tabstop=4  expandtab autoindent shiftwidth=4
