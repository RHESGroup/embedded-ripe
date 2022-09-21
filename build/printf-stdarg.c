/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)
	stdarg version contributed by Christian Ettinger

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include "uart.h"

#define putchar(c)      UART0_DATA = c
#define getchar(c)		*c = UART0_DATA

// -------------------------------------------------------
// PRIVATE UART SEND/RECV DEFINITIONS
// -------------------------------------------------------

inline static void prv_uart_getc(uint8_t* c)
{
	// wait till data register not empty
	while(!(UART0_STATE & UART_SR_RXNE));
	
	getchar(c);
}

inline static void prv_uart_putc(uint8_t* c)
{
	putchar(*c);

	// wait for transmit complete
    while(!(UART0_STATE & UART_SR_TC));
}

// -------------------------------------------------------
// PRINTF/SCANF UTILS
// -------------------------------------------------------

static void printchar(char **str, int c, char *buflimit)
{
	if (str) {
		if( buflimit == ( char * ) 0 ) {
			/* Limit of buffer not known, write charater to buffer. */
			**str = (char)c;
			++(*str);
		}
		else if( ( ( unsigned long ) *str ) < ( ( unsigned long ) buflimit ) ) {
			/* Within known limit of buffer, write character. */
			**str = (char)c;
			++(*str);
		}
	}
	else
	{
		prv_uart_putc((uint8_t*)&c);
	}
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad, char *buflimit)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar, buflimit);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string, buflimit);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar, buflimit);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase, char *buflimit)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = (unsigned int)i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad, buflimit);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = (unsigned int)-i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = (unsigned int)u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = (char)(t + '0');
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-', buflimit);
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad, buflimit);
}

static int tiny_scan (const char *format, va_list args)
{
    int i = 0, j = 0, ret = 0;
    char buff[100] = {0}, c = 0;
    char *out_loc;

	do 
	{
		prv_uart_getc((uint8_t*)&c);
		prv_uart_putc((uint8_t*)&c);        
		buff[i] = c;
		i++;
	} while(c != '\0' && c != '\r' && c != '\n');

	// terminate string
	buff[i] = '\0';

	i = 0;
 	while (format && format[i])
 	{
 	    if (format[i] == '%') 
 	    {
			i++;
 	       	switch (format[i]) 
 	       	{
 	           	case 'c': 
 	           	{
	 	           	*(char *)va_arg( args, char* ) = buff[j];
	 	           	j++;
	 	           	ret ++;
	 	           	break;
 	           	}
 	           	case 'd': 
 	           	{
	 	           	*(int *)va_arg( args, int* ) = strtol(&buff[j], &out_loc, 10);
	 	           	j+=out_loc -&buff[j];
	 	           	ret++;
	 	           	break;
 	            }
 	            case 'x': 
 	            {
	 	           	*(int *)va_arg( args, int* ) = strtol(&buff[j], &out_loc, 16);
	 	           	j+=out_loc -&buff[j];
	 	           	ret++;
	 	           	break;
 	            }
				case 's': 
 	            {
					char* arg = (char *)va_arg( args, char* );
					while (buff[j] != '\0')
					{
						*(arg++) = buff[j];
	 	           		j++;
	 	           		ret ++;
					}
					*(arg) = '\0';
	 	           	break;
 	            }
 	        }
 	    } 
 	    else 
 	    {
 	        buff[j] = format[i];
            j++;
        }
        i++;
    }
    
	va_end(args);
    return ret;
}

static int tiny_print( char **out, const char *format, va_list args, unsigned int buflen )
{
	register int width, pad;
	register int pc = 0;
	char scr[2], *buflimit;

	if( buflen == 0 ){
		buflimit = ( char * ) 0;
	}
	else {
		/* Calculate the last valid buffer space, leaving space for the NULL
		terminator. */
		buflimit = ( *out ) + ( buflen - 1 );
	}

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				pc += prints (out, s?s:"(null)", width, pad, buflimit);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a', buflimit);
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a', buflimit);
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A', buflimit);
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a', buflimit);
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad, buflimit);
				continue;
			}
		}
		else {
		out:
			printchar (out, *format, buflimit);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

// -------------------------------------------------------
// PUBLIC INTERFACES
// -------------------------------------------------------

int scanf(const char* format, ...)
{
		va_list args;

		va_start( args, format );
		return tiny_scan( format, args );

}

int printf(const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return tiny_print( 0, format, args, 0 );
}

int sprintf(char *out, const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return tiny_print( &out, format, args, 0 );
}


int snprintf( char *buf, unsigned int count, const char *format, ... )
{
        va_list args;

        ( void ) count;

        va_start( args, format );
        return tiny_print( &buf, format, args, count );
}

/*
 * Initialise the serial hardware.
 */

void uart_init(void)
{
	UART0_BAUDDIV = 16;
	UART0_BRR = 1;

	// enable TX/RX
	UART0_CR1 |= UART_CR1_TE;
    UART0_CR1 |= UART_CR1_RE;

	// enable UART 0
	UART0_CR1 |= UART_CR1_UE;
}
