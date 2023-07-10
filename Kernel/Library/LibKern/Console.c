/*
* Console printing/logging functionalities for the Kernel
*
* Original src: mit-pdos/xv6-public
* Modified by: Tuna CICI
*/

#include <stdint.h>
#include <stdarg.h>

#include <Console.h>
#include <MemoryLayout.h>

static void uart_putc(const char c)
{
	volatile uint32_t *uart = (uint32_t*) PL011_BASE;
        *uart = c;
}

void kprint_int(uint64_t xx, uint16_t base, uint16_t sign)
{
	const static char digits[] = "0123456789ABCDEF";
	char buffer[64] = {0};
	uint64_t x = 0;

	if (sign && (sign = xx < 0)) {
		x = -xx;
	} else {
		x = xx;
	}

	int i = 0;
	
	do {
		buffer[i++] = digits[x % base];
	} while((x /= base) != 0);

	if (sign) {
		buffer[i++] = '-';
	}

	while (--i >= 0) {
		uart_putc(buffer[i]);
	}
	
}

void kprint_str(const char *str)
{
	if (str == 0) {
		uart_putc('(');
		uart_putc('n');
		uart_putc('u');
		uart_putc('l');
		uart_putc('l');
		uart_putc(')');
		uart_putc('\0');
		
		return;
	}

	for (uint32_t i = 0; str[i] != '\0'; i++) {
		uart_putc(str[i]);
	}
}

void log()
{
	return;
}

void kprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (fmt == 0) {
		return;
	}

	for (uint32_t i = 0; fmt[i] != '\0'; i++) {
		char c = fmt[i];

		if (c != '%') {
			uart_putc(c);
			continue;
		}

		uint64_t *argp = va_arg(args, uint64_t*);		
		c = fmt[++i];

		switch (c) {
		case 'd':
			kprint_int(*argp, 10, 1);
			break;
		case 'x':
		case 'p':
			kprint_int(*argp, 16, 0);
			break;
		case 's':
			kprint_str((const char *) argp);
			break;
		default:
			uart_putc('%');
			uart_putc(c);
			break;
		}
	}

	va_end(args);
}
