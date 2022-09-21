
#ifndef UART_H
#define UART_H

/*
 * printf() output uses the UART.
 * These constants define the addresses of the required UART registers.
 */
#define UART0_ADDRESS 	(0x40004400UL)
#define UART0_STATE 	(*(((volatile uint32_t *)(UART0_ADDRESS + 0x00UL))))
#define UART0_DATA 		(*(((volatile uint32_t *)(UART0_ADDRESS + 0x04UL))))
#define UART0_BRR 		(*(((volatile uint32_t *)(UART0_ADDRESS + 0x08UL))))
#define UART0_CR1		(*(((volatile uint32_t *)(UART0_ADDRESS + 0x0CUL))))
#define UART0_BAUDDIV 	(*(((volatile uint32_t *)(UART0_ADDRESS + 0x10UL))))

#define UART_SR_TC      (1 << 6)    // TC: Transmission complete
#define UART_SR_RXNE    (1 << 5)    // RXNE: Read data register not empty. 

#define UART_CR1_TE     (1 << 3)    // TE: Transmission enabled 
#define UART_CR1_RE     (1 << 2)    // RE: Reception enabled
#define UART_CR1_UE     (1 << 13)   // UE: UART Enabled

/*
 * Initialise the serial hardware.
 */
void uart_init(void);

int snprintf( char *buf, unsigned int count, const char *format, ... );

int sprintf(char *out, const char *format, ...);

int printf(const char *format, ...);

int scanf(const char* format, ...);

#endif /* UART_H */