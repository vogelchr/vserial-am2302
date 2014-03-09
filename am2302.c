/* Interface a AM2302 Humidity/Temperature Sensor to an AVR Microcontroller
 *
 * (c) Christian Vogel <vogelchr@vogel.cx>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

/* pinout of the AM2302 (view from front)
 *
 *           +--------+ _
 * Vcc  1 ---|L  ++++ |  \
 * Data 2 ---|A  ++++ | ()\
 * n/c  3 ---|B  ++++ | ()/
 * GND  4 ---|EL ++++ | _/
 *           +--------+
 *
 * Data is pulled high internally with a x kOhms resistor to Vcc.
 *
 * Conversion is started by the MCU by pulling down Data for nominal
 * 1ms. It will take the sensor ~160 usec to react, then the sensor
 * acknowledges by pulling low data for 80us, then releases data-line
 * for 80us.
 *
 * Then 40 bits are transmitted from sensor to MCU according to the
 * following encoding:
 *
 * - A 0-bit is transmitted by the sensor by pulling data low for 50us,
 *   then releaseing data for 26-28us.
 *
 * - A 1-bit is transmitted by the sensor by pulling data low for 50us,
 *   then releasing data for 70us.
 *
 * At the end, the sensor will pull down data for 50us one last time.
 *
 * Resources we are using in the ATmega32U4:
 *
 *    Timer/Counter 1, with its capture unit. It is normally stopped
 *    and only running when we capture bits from the sensor.
 *
 *    Sensor must be connected to PD4/ICP1/ADC8, this is called
 *    "Digital4" on the Olimexino 32U4, maybe also on the Arduino Leonardo.
 */

#include "am2302.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static char am2302_bit_ctr;
static char am2302_data[5];

/*
 * Timer/Counter1 Capture Interrupt. We capture both falling and rising
 * edges, toggling the TCCR1B ICES1 (edge sensitivity) bit as we go along.
 * Timer/Counter1 is reset to 0 every time we captured an edge. Overflow
 * will be handled in the overflow interrupt handler, stopping all
 * acquisition. At 16 Mhz, the counter overflows after 4ms, one LSB is
 * 62ns.
 */

SIGNAL(TIMER1_CAPT_vect)
{
	volatile uint16_t cnt = TCNT1;
	TCNT1 = 0;

	/* if ICES1 is set, we trigger on rising edge,
	 * if it's clear, we trigger on the falling edge */
	if ( TCCR1B & _BV(ICES1) ) {   /* rising edge, this is always */
		TCCR1B &= ~_BV(ICES1); /* ...constant ~80us */
	} else {
		TCCR1B |= _BV(ICES1);
		/* falling edge: length of previous high-period indicates
		 * a 0 or 1 bit:
		 *   0 : nominal 28us == 448 counts
		 *   1 : nominal 70us == 1120 counts
		 * A small hack: we fill bytes from highest to lowest
		 * to ease arithmetics, bit #41 (skipped) is the quiescent
		 * period after the "ACK" bit from the sensor.
		 */
		if (am2302_bit_ctr < 40) { /* bit #40 is "ack" period */
			am2302_data[am2302_bit_ctr / 8] <<= 1;
			if (cnt > 800) { 
				am2302_data[am2302_bit_ctr / 8] |= 1;
			}
		}
		if (am2302_bit_ctr == 0)
			goto disable_irqs;
		am2302_bit_ctr--;
	}
	return;

disable_irqs:
	TIMSK1 &= ~(_BV(ICIE1) | _BV(TOIE1)); /* disable interrupts */
	TIFR1  |=   _BV(ICF1)  | _BV(TOV1);   /* acknowledge interrupts */
	TCCR1B  = 0; /* stop timer */
}

/*
 * Overflow, after ~4ms without detecting any edge, also used for
 * generating the delay for the initial 80us start pulse asynchronously.
 */
SIGNAL(TIMER1_OVF_vect)
{
	/* if the capture unit is not enabled, it means that we are
	 * currently transmitting the start pulse! */
	if (!(TIMSK1 & _BV(ICIE1))) {
		TCNT1   = 0;          /* reset counter to 0 */
		DDRD   &= ~_BV(4);    /* release pull down on port pin */
		/* ---- enable input capture hardware ---- */
		// ICES1 already unset, clearing it would be redundant
		// TCCR1B &= ~_BV(ICES1); /* trigger on falling edge */
		TIFR1  |= _BV(ICF1);  /* clear input capture interrupt */
		TIMSK1 |= _BV(ICIE1); /* enable input capture interrupt */
		return;
	}

	/* timeout! */
	TIMSK1 &= ~(_BV(ICIE1) | _BV(TOIE1)); /* disable interrupts */
	TIFR1  |=   _BV(ICF1)  | _BV(TOV1);   /* acknowledge interrupts */
	TCCR1B  = 0; /* stop timer */
	TCNT1  = 0xffffUL; /* reset counter to FFFF, flag to signal ovfl */
}

unsigned char
am2302_get_raw_data(unsigned char *data)
{
	int i;
	for (i=0; i<sizeof(am2302_data); i++)
		data[i] = am2302_data[sizeof(am2302_data)-1-i];
	return am2302_bit_ctr;
}

void
am2302_trigger_read(void)
{
	int i;

	/* 42th and 41th period is before and after "ack" */
	am2302_bit_ctr = 41; /* 41: skip, 40: skip, 39..0: data */
	for (i=0; i<sizeof(am2302_data); i++)
		am2302_data[i]=0;

	/* make sure everything is tidy... */
	TIMSK1 &= ~(_BV(ICIE1) | _BV(TOIE1)); /* disable interrupts */
	TIFR1  |=   _BV(ICF1)  | _BV(TOV1);   /* acknowledge interrupts */
	TCCR1B  = 0; /* stop timer */

	/* 80us start pulse is 1280 ticks at 16 MHz, the start puls
	 * will end when the overflow routine releases the pulldown, hence
	 * set counter to 1^16 - 1280 = 64256;
	 */

	TCNT1   = (1ULL<<16) - 1280ULL;
	TIFR1  |= _BV(TOV1);  /* clear overflow interrupt */
	TIMSK1 |= _BV(TOIE1); /* enable overflow interrupt */

	DDRD   |= _BV(4);     /* pull down */
	TCCR1B  = _BV(CS10);  /* start timer! */
}

/* data received from sensor:
 *    MSB_RH   LSB_RH   MSB_TEMP   LSB_TEMP   CHECKSUM
 * stored in data array backwards:
 *         4        3          2          1          0
 *
 * Encoding of relative humidity: unsigned 16bit int, LSB = 0.1%
 * Encoding of temperature:
 *    MSB_TEMP & 0x80 is a sign bit
 *    ((MSB_TEMP & 0x7f)<<8) + LSB_TEMP is absolute value of temp.
 * CRC is sum of the four bytes preceeding it.
 */

enum am2302_result
am2302_get_result(int16_t *temp, uint16_t *rh)
{
	if (TCCR1B != 0) /* timer is running */
		return am2302_ongoing;
	if (TCNT1 == 0xfffful) /* timeout */
		return am2302_timeout;

	/* crc */
	if ( ((am2302_data[4] + am2302_data[3] + am2302_data[2] +
	       am2302_data[1]) & 0xff) != am2302_data[0] )
		return am2302_crc;

	/* first byte received @4, second byte received @3 */
	*rh = (am2302_data[4] << 8) | am2302_data[3];

	/* third byte received @2, fourth byte received @1 */
	uint16_t val = (am2302_data[2] << 8) | am2302_data[1];
	if (val & 0x8000) { /* topmost bit is set */
		val &= 0x8000; /* clear topmost bit */
		*temp = - val;
	} else {
		*temp = val;
	}

	return am2302_ok;
}


void
am2302_init(void)
{
	TIMSK1 &= ~(_BV(ICIE1) | _BV(TOIE1)); /* disable interrupts */
	TIFR1  |=   _BV(ICF1)  | _BV(TOV1);   /* acknowledge interrupts */
	TCCR1B  = 0; /* stop timer */

	DDRD  &= ~_BV(4); /* make sure port D4 is an input */
	PORTD &= ~_BV(4); /* pulled up externally */

}


