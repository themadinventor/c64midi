/*
 * cc64midi
 * (c) 2015 Fredrik Ahlberg <fredrik@z80.se>
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
 * PD0: RX	Midi In
 *
 * PD2: PC	Strobe from C64
 * PD3: FLAG	IRQ to C64
 *
 * PB1: LED	Status Blinkenlights
 *
 * PB2: /SS	Must be high to enable SPI controller
 *
 * PB3: MOSI	Serial data to C64
 * PB5: SCK	Serial clock to C64
 *
 */

volatile uint8_t buffer[256], head, tail;

ISR(USART_RX_vect)
{
	/* Store in ring buffer */
	buffer[head++] = UDR0;

	/* Strobe interrupt */
	//PORTD &= ~_BV(3);
	//_delay_us(2);
	//PORTD |= _BV(3);

	/* Toggle LED */
	PORTB |= _BV(1);
}

void uart_init(void)
{
	UCSR0B = _BV(RXEN0) | _BV(RXCIE0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);

	uint16_t UBRR = F_CPU / 16 / (31250) - 1;
	UBRR0H = UBRR >> 8;
	UBRR0L = UBRR;
}

ISR(INT0_vect)
{
	if (head == tail) {
		return;
	}

	/* Send one byte from buffer */
	SPDR = buffer[tail++];

}

ISR(SPI_STC_vect)
{
	if (head == tail) {
		/* Toggle led */
		PORTB &= ~_BV(1);
		return;
	}

	/* Send one byte from buffer */
	SPDR = buffer[tail++];
}

int main(void)
{
	uart_init();

	DDRB = _BV(1) | _BV(5) | _BV(3) | _BV(2);
	PORTD |= _BV(0) | _BV(3);
	DDRD |= _BV(3);

	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0) | _BV(SPIE);

	EICRA = _BV(ISC01);
	EIMSK = _BV(INT0);

	sei();

	for (;;) ;
}

