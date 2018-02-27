#include <inttypes.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#define USART_baud 38400
int stdio_putchar(char c, FILE *stream);
int stdio_getchar(FILE *stream);

void boot_program_page (uint32_t page, uint8_t *buf)
{
    uint16_t i;
    uint8_t sreg;
    // Disable interrupts.
    sreg = SREG;
    cli();
    eeprom_busy_wait ();
    boot_page_erase (page);
    boot_spm_busy_wait ();      // Wait until the memory is erased.
    for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.
        uint16_t w = *buf++;
        w += (*buf++) << 8;

        boot_page_fill (page + i, w);
    }
    boot_page_write (page);     // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
    // Reenable RWW-section again. We need this if we want to jump back
    // to the application after bootloading.
    boot_rww_enable ();
    // Re-enable interrupts (if they were ever enabled).
    SREG = sreg;
}

static FILE STDIO_BUFFER = FDEV_SETUP_STREAM(stdio_putchar, stdio_getchar, _FDEV_SETUP_RW);

int stdio_putchar(char c, FILE *stream)
{
   if (c == '\n')
        stdio_putchar('\r',stream);

    while((UCSR0A&(1<<UDRE0))==0);

    UDR0 = c;

    return 0;
}

int stdio_getchar(FILE *stream)
{
	int UDR_Buff;

    while((UCSR0A&(1<<RXC0))==0);

	UDR_Buff = UDR0;

	stdio_putchar(UDR_Buff,stream);

	return UDR_Buff;
}

char ASA_STDIO_set(void)
{
	unsigned int baud;

	baud = F_CPU/16/USART_baud-1;
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;

	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
	UCSR0C |= (3<<UCSZ00);

	stdout = &STDIO_BUFFER;
	stdin = &STDIO_BUFFER;

	return 0;
}

int main(void) {
	ASA_STDIO_set();
    printf("SPM_PAGESIZE %d\n", SPM_PAGESIZE);
    uint32_t page = 0;
    uint32_t i=0;
    uint8_t c[SPM_PAGESIZE] ;

	for(i = 0; i < SPM_PAGESIZE; i++)
		c[i] = i;

	boot_program_page(page, c);

    printf("END\n" );
    
}
