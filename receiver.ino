#include <avr/io.h>
#include <util/delay.h>

#define FOSC 16000000                       // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD -1

uint16_t adcvalue = 0;
volatile uint16_t inputWaiting = 0;
char myMessage[] = "LDR reading";

int main (void) {

  initADC();
  initUSART();

  DDRD = 0xff;

  while (1) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & _BV(ADSC))
      adcvalue = ADC;

    uint16_t byter = getNumber();

    if(byter < 10) {
      PORTD |= (1 << PIND3);
    } else {
      PORTD = 0b00000000;
    }
    
    _delay_ms(10);
  }
}

static inline void initADC(void) { 
  ADMUX  |= (1 << REFS0 );
  ADCSRA |= (1 << ADPS2 ) | (1 << ADPS1 );
  ADCSRA |= (1 << ADEN );
}

void initUSART(void) {
  UBRR0H = (MYUBRR >> 8);
  UBRR0L = MYUBRR;
  UCSR0B = (1 << TXEN0) | (1 << RXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void transmitByte(uint8_t data) {
  loop_until_bit_is_set(UCSR0A, UDRE0);     /* Wait for empty transmit buffer */
  UDR0 = data;                              /* send data */
}

uint8_t receiveByte(void) {
  loop_until_bit_is_set(UCSR0A, RXC0);       /* Wait for incoming data */
  return UDR0;                               /* return register value */
}

void printByte(uint16_t byte) {
  transmitByte('0' + (byte / 100));                        /* Hundreds */
  transmitByte('0' + ((byte / 10) % 10));                      /* Tens */
  transmitByte('0' + (byte % 10));                             /* Ones */
  transmitByte('\r');
  transmitByte('\n');
}

void printString(const char myString[]) {
  uint16_t i = 0;
  while (myString[i]) {
    transmitByte(myString[i]);
    i++;
  }
  transmitByte('\n');
}

uint16_t getNumber(void) {
  char hundreds = '0';
  char tens = '0';
  char ones = '0';
  char thisChar = '0';
  do {
    tens = ones;
    ones = thisChar;
    thisChar = receiveByte();
    transmitByte(thisChar);
    //_delay_ms(500);
  }
  while (thisChar != '\r');
  inputWaiting = 0;
  return (100 * (hundreds - '0') + 10 * (tens - '0') + ones - '0');
}