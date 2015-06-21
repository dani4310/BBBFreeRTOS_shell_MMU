#ifndef __SERIAL_H_
#define __SERIAL_H_			1

#define UART_CLK			(48000000)
#define UART0_BAUDRATE			115200
#define UART4_BAUDRATE			38400

#define FCR_FIFO_EN     0x01		/* Fifo enable */
#define FCR_RXSR        0x02		/* Receiver soft reset */
#define FCR_TXSR        0x04		/* Transmitter soft reset */

#define MCR_DTR         0x01
#define MCR_RTS         0x02
#define MCR_DMA_EN      0x04
#define MCR_TX_DFR      0x08

#define LCR_WLS_MSK		0x03		/* character length slect mask */
#define LCR_WLS_5		0x00		/* 5 bit character length */
#define LCR_WLS_6		0x01		/* 6 bit character length */
#define LCR_WLS_7		0x02		/* 7 bit character length */
#define LCR_WLS_8		0x03		/* 8 bit character length */
#define LCR_STB			0x04		/* Number of stop Bits, off = 1, on = 1.5 or 2) */
#define LCR_PEN			0x08		/* Parity eneble */
#define LCR_EPS			0x10		/* Even Parity Select */
#define LCR_STKP		0x20		/* Stick Parity */
#define LCR_SBRK		0x40		/* Set Break */
#define LCR_BKSE		0x80		/* Bank select enable */

#define LSR_DR			0x01		/* Data ready */
#define LSR_OE			0x02		/* Overrun */
#define LSR_PE			0x04		/* Parity error */
#define LSR_FE			0x08		/* Framing error */
#define LSR_BI			0x10		/* Break */
#define LSR_THRE		0x20		/* Xmit holding register empty */
#define LSR_TEMT		0x40		/* Xmitter empty */
#define LSR_ERR			0x80		/* Error */

/* useful defaults for LCR */
#define LCR_8N1			0x03


/* Serial Offsets */
#define DLL_REG					0x000
#define RHR_REG					0x000
#define THR_REG					0x000
#define DLH_REG					0x004
#define IER_REG					0x004
#define IIR_REG					0x008
#define FCR_REG					0x008
#define EFR_REG					0x008
#define LCR_REG					0x00C
#define MCR_REG					0x010
#define XON1_ADDR1_REG			0x010
#define LSR_REG					0x014
#define XON2_ADDR2_REG			0x014
#define MSR_REG					0x018
#define TCR_REG					0x018
#define XOFF1_REG				0x018
#define SPR_REG					0x01C
#define TLR_REG					0x01C
#define XOFF2_REG				0x01C
#define MDR1_REG				0x020
#define MDR2_REG				0x024
#define SFLSR_REG				0x028
#define TXFLL_REG				0x028
#define RESUME_REG				0x02C
#define TXFLH_REG				0x02C
#define SFREGL_REG				0x030
#define RXFLL_REG				0x030
#define SFREGH_REG				0x034
#define RXFLH_REG				0x034
#define UASR_REG				0x038
#define BLR_REG					0x038
#define ACREG_REG				0x03C
#define SCR_REG					0x040
#define SSR_REG					0x044
#define EBLR_REG				0x048
#define MVR_REG					0x050
#define SYSC_REG				0x054
#define SYSS_REG				0x058
#define WER_REG					0x05C
#define CFPS_REG				0x060

void init_serial(unsigned int base);
void serial_putsn (unsigned int base, const char *s, int n);
void serial_putc (unsigned int base,const char c);
void serial_puts (unsigned int base,const char *s);
signed char UARTCharGet(unsigned int baseAdd);
unsigned int UARTRegConfigModeEnable(unsigned int baseAdd, unsigned int modeFlag);
int io_printf(unsigned int base, const char *format, ...);
void io_write(int base,const char* buf,int length);
int io_read( int base ,void * buf, int count) ;
char *itoa(const char *numbox, int num, unsigned int base);
char *uitoa(const char *numbox, unsigned int num, unsigned int base);
int strlen(const char *str);
#define HWREG(x)                                                              \
        (*((volatile unsigned int *)(x)))
#define UART_REG_OPERATIONAL_MODE           (0x007F)
#define UART_LSR   (0x14)
#define UART_LSR_RX_FIFO_E   (0x00000001u)
#define UART_RHR   (0x0)
#define UART_LCR   (0xC)
#define UART_REG_CONFIG_MODE_A              (0x0080)
#define UART_REG_CONFIG_MODE_B              (0x00BF)

#endif	/* __SERIAL_H_ */
