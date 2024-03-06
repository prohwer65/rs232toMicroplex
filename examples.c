
#define F_CPU 1000000UL

#define BAUD 9600
// formula for 1MHz clock and U2X0 = 1 double UART speed 
#define MYUBBR ((F_CPU / (BAUD * 8L)) - 1)



// ----------------------------------------------------------------------------------------------
// init_uart
// ----------------------------------------------------------------------------------------------
void init_uart(void) {
  // double speed by U2X0 flag = 1 to have 0.2% error rate on 9600 baud
 UCSR0A = (1<<U2X0);
  // set baud rate from PRESCALER
 UBRR0H = (uint8_t)(MYUBBR>>8);
 UBRR0L = (uint8_t)(MYUBBR);
 UCSR0B|=(1<<TXEN0); //enable TX
 UCSR0B|=(1<<RXEN0); //enable RX
  // set frame format for SIM808 communication
 UCSR0C|=(1<<UCSZ00)|(1<<UCSZ01); // no parity, 1 stop bit, 8-bit data 
}


TIMER0_COMPA_vect)
// ----------------------------------------------------------------------------------------------
// init_uart
// ----------------------------------------------------------------------------------------------
void init_uart(void) {
  // double speed by U2X0 flag = 1 to have 0.2% error rate on 9600 baud
 UCSR0A = (1<<U2X0);
  // set baud rate from PRESCALER
 UBRR0H = (uint8_t)(MYUBBR>>8);
 UBRR0L = (uint8_t)(MYUBBR);
 UCSR0B|=(1<<TXEN0); //enable TX
 UCSR0B|=(1<<RXEN0); //enable RX
  // set frame format for SIM808 communication
 UCSR0C|=(1<<UCSZ00)|(1<<UCSZ01); // no parity, 1 stop bit, 8-bit data 
}





// ============================================================================
//                         Set MCU clock to 8MHz!
//                      Everything here relies on it!
// ============================================================================

#ifndef F_CPU
#define F_CPU                 8000000UL
#else
#if (F_CPU!=8000000)
#error: F_CPU not set to 8MHz! Readjust project properties!
#endif
#endif


// ============================================================================
//                   Check for ATtiny2313 to be selected
// ============================================================================

#ifndef __AVR_ATtiny2313__
#error: This code is exclusively dedicated to ATtiny2313!
#endif
// Very close relatives could eventually also be used without changes.
