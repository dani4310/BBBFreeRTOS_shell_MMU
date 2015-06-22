/****
Hfo @ 2014
****/


#include "am335.h"
#include "serial.h"
#include <stdarg.h>
static int calc_divisor (unsigned int baud)
{
	return (UART_CLK / 16 / baud);
}

void init_serial(unsigned int base)	{
	int clock_divisor;
	if (base == UART4_BASE)	{
		clock_divisor = calc_divisor(UART4_BAUDRATE);
		(*(REG32(base+0x4))) = 0x0;	
		serial_puts(UART0_BASE,"Init4\n");}
	else	{
		clock_divisor = calc_divisor(UART0_BAUDRATE);
		(*(REG32(base+0x4))) = 0x0;	}
	(*(REG32(base+0xC))) = 0x83; 
	(*(REG32(base+0x0))) = clock_divisor & 0xff;
	(*(REG32(base+0x4))) = (clock_divisor >> 8) & 0xff;
	(*(REG32(base+0xC))) = (MCR_DTR | MCR_RTS);
	(*(REG32(base+0x10))) = 0x3;
	(*(REG32(base+0x8))) = (FCR_FIFO_EN | FCR_RXSR | FCR_TXSR);
}

void serial_putc( unsigned int base, const char c )	{
	while (((*(REG32(base+0x14))) & LSR_THRE) == 0);
	(*(REG32(base+0x00))) = c;
}

void serial_puts (unsigned int base, const char *s)
{
	while (*s) {
		serial_putc (base,*s++);
	}
}

void serial_putsn (unsigned int base, const char *s, int n)
{
	int i;
	for (i=0; i<n; i++)	{
		serial_putc (base,s[i]);
	}
}

// unsigned int UARTRegConfigModeEnable(unsigned int baseAdd, unsigned int modeFlag)
// {
//     unsigned int lcrRegValue = 0;

//     /* Preserving the current value of LCR. */
//     lcrRegValue = HWREG(baseAdd + UART_LCR);

//     switch(modeFlag)
//     {
//         case UART_REG_CONFIG_MODE_A:
//         case UART_REG_CONFIG_MODE_B:
//             HWREG(baseAdd + UART_LCR) = (modeFlag & 0xFF);
//         break;

//         case UART_REG_OPERATIONAL_MODE:
//             HWREG(baseAdd + UART_LCR) &= 0x7F;
//         break;

//         default:
//         break;
//     }

//     return lcrRegValue;
// }

signed char UARTCharGet(unsigned int baseAdd)
{
    unsigned int lcrRegValue = 0; 
    signed char retVal = 0; 

    /* Switching to Register Operational Mode of operation. */
 //   lcrRegValue = UARTRegConfigModeEnable(baseAdd, UART_REG_OPERATIONAL_MODE);

    /* Waits indefinitely until a byte arrives in the RX FIFO(or RHR). */
    while(0 == (HWREG(baseAdd + UART_LSR) & UART_LSR_RX_FIFO_E));

    retVal = ((signed char)HWREG(baseAdd + UART_RHR));

    /* Restoring the value of LCR. */
 //   HWREG(baseAdd + UART_LCR) = lcrRegValue;

    return retVal;
}



int io_read(int base, void * buf, int count) {
    int i=0, endofline=0, last_chr_is_esc,b;
    char *ptrbuf=buf;
    char ch;
    while(ptrbuf[i]!='\0')
        i++;
    // io_printf(UART0_BASE,"i=%d\r\n",i);
    while(i < count&&endofline!=1){
    ptrbuf[i]=UARTCharGet(base);
    switch(ptrbuf[i]){
        case '\r':
        case '\n':
            ptrbuf[i]='\0';
            endofline=1;
            break;
        case '[':
            if(last_chr_is_esc){
                last_chr_is_esc=0;
                ch=UARTCharGet(base);
                if(ch>=1&&ch<=6){
                    ch=UARTCharGet(base);
                }
                if(ch==65)
                {
                                // io_printf(UART0_BASE, "\r\nhello\r\n");
                    for(b=0;b<i;b++)
                    {
                        serial_putc( base, '\b');
                        serial_putc( base, ' ');
                        serial_putc( base, '\b');
                    }
                    return -1;
                }
                if(ch==66)
                {
                                // io_printf(UART0_BASE, "\r\nhello\r\n");
                    for(b=0;b<i;b++)
                    {
                        serial_putc( base, '\b');
                        serial_putc( base, ' ');
                        serial_putc( base, '\b');
                    }
                    return -2;
                }

                continue;
            }
        case 27://ESC
            last_chr_is_esc=1;
            continue;
        case 127://BACKSPACE
            last_chr_is_esc=0;
            if(i>0){
                serial_putc( base, '\b');
                serial_putc( base, ' ');
                serial_putc( base, '\b');
                --i;
            }
            continue;
        default:
            last_chr_is_esc=0;
    }
    serial_putc( base, ptrbuf[i]);
    ++i;
    }
    return i;
}

void io_write(int base,const char* buf,int length)
{
    int i=0;
    while(i<length)
    {
        serial_putc(base,buf[i]);
        i++;
    }
    return;
}

int io_printf(unsigned int base, const char *format, ...){
    int i,count=0;

    va_list(v1);
    va_start(v1, format);

    int tmpint;
    unsigned int tmpuint;
    char *tmpcharp;
    //serial_puts(UART0_BASE,"k\r\n");

    for(i=0; format[i]; ++i){
            //serial_puts(UART0_BASE,"f \r\n");
        if(format[i]=='%'){
            switch(format[i+1]){
                case '%':
                    serial_putc(UART0_BASE,'%'); break;
                case 'p':
                        tmpuint = va_arg(v1,unsigned int);
                        tmpcharp = uitoa(format[i+1]=='x'?"0123456789abcdef":"0123456789ABCDEF", tmpuint, 16);
                        io_write(base,tmpcharp, strlen(tmpcharp));
                        break;
                case 'd':
                case 'x':
                case 'X':
                    tmpint = va_arg(v1, int);
                    tmpcharp = itoa(format[i+1]=='x'?"0123456789abcdef":"0123456789ABCDEF", tmpint, format[i+1]=='d'?10: 16);
                    io_write(base, tmpcharp, strlen(tmpcharp));
                    break;
                case 's':
                    tmpcharp = va_arg(v1, char *);
                    io_write(base, tmpcharp, strlen(tmpcharp));
                    break;
            }
            /* Skip the next character */
            ++i;
        }else
            io_write(base, format+i, 1);
    }

    va_end(v1);
    return count;
}

char *itoa(const char *numbox, int num, unsigned int base){
    static char buf[32]={0};
    int i;
    if(num==0){
        buf[30]='0';
        return &buf[30];
    }
    int negative=(num<0);
    if(negative) num=-num;
    for(i=30; i>=0&&num; --i, num/=base)
        buf[i] = numbox[num % base];
    if(negative){
        buf[i]='-';
        --i;
    }
    return buf+i+1;
}
char *uitoa(const char *numbox, unsigned int num, unsigned int base){
    static char buf[32]={0};
    int i;
    if(num==0){
        buf[30]='0';
        return &buf[30];
    }

    for(i=30; i>=0&&num; --i, num/=base)
        buf[i] = numbox[num % base];
 
    return buf+i+1;
}
int strlen(const char *str){
    int count;
    for(count=0;*str;++count, ++str);
    return count;
}
