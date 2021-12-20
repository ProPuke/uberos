#include "serial.hpp"

#include "mmio.hpp"
#include "mailbox.hpp"
#include <common/types.hpp>
#include <kernel/stdio.hpp>

enum {
    PERIPHERAL_BASE = 0xFE000000,
    GPFSEL0         = PERIPHERAL_BASE + 0x200000,
    GPSET0          = PERIPHERAL_BASE + 0x20001C,
    GPCLR0          = PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0       = PERIPHERAL_BASE + 0x2000E4
};

enum {
    GPIO_MAX_PIN       = 53,
    GPIO_FUNCTION_ALT5 = 2,
};

enum {
    Pull_None = 0,
};

void mmio_write(long reg, unsigned int val) { *(volatile unsigned int *)reg = val; }
unsigned int mmio_read(long reg) { return *(volatile unsigned int *)reg; }

unsigned int gpio_call(unsigned int pin_number, unsigned int value, unsigned int base, unsigned int field_size, unsigned int field_max) {
    unsigned int field_mask = (1 << field_size) - 1;
  
    if (pin_number > field_max) return 0;
    if (value > field_mask) return 0; 

    unsigned int num_fields = 32 / field_size;
    unsigned int reg = base + ((pin_number / num_fields) * 4);
    unsigned int shift = (pin_number % num_fields) * field_size;

    unsigned int curval = mmio_read(reg);
    curval &= ~(field_mask << shift);
    curval |= value << shift;
    mmio_write(reg, curval);

    return 1;
}

unsigned int gpio_set     (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPSET0, 1, GPIO_MAX_PIN); }
unsigned int gpio_clear   (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPCLR0, 1, GPIO_MAX_PIN); }
unsigned int gpio_pull    (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPPUPPDN0, 2, GPIO_MAX_PIN); }
unsigned int gpio_function(unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPFSEL0, 3, GPIO_MAX_PIN); }

void gpio_useAsAlt5(unsigned int pin_number) {
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_ALT5);
}

// UART

enum {
    AUX_BASE        = PERIPHERAL_BASE + 0x215000,
    AUX_ENABLES     = AUX_BASE + 4,
    AUX_MU_IO_REG   = AUX_BASE + 64,
    AUX_MU_IER_REG  = AUX_BASE + 68,
    AUX_MU_IIR_REG  = AUX_BASE + 72,
    AUX_MU_LCR_REG  = AUX_BASE + 76,
    AUX_MU_MCR_REG  = AUX_BASE + 80,
    AUX_MU_LSR_REG  = AUX_BASE + 84,
    AUX_MU_CNTL_REG = AUX_BASE + 96,
    AUX_MU_BAUD_REG = AUX_BASE + 104,
    AUX_UART_CLOCK  = 500000000,
    UART_MAX_QUEUE  = 16 * 1024
};

#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

void uart_init() {
    mmio_write(AUX_ENABLES, 1); //enable UART1
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3); //8 bits
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_IIR_REG, 0xC6); //disable interrupts
    mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));
    gpio_useAsAlt5(14);
    gpio_useAsAlt5(15);
    mmio_write(AUX_MU_CNTL_REG, 3); //enable RX/TX
}

unsigned int uart_isWriteByteReady() { return mmio_read(AUX_MU_LSR_REG) & 0x20; }

void uart_writeByteBlockingActual(unsigned char ch) {
    while (!uart_isWriteByteReady()); 
    mmio_write(AUX_MU_IO_REG, (unsigned int)ch);
}

void uart_writeText(const char *buffer) {
    while (*buffer) {
       if (*buffer == '\n') uart_writeByteBlockingActual('\r');
       uart_writeByteBlockingActual(*buffer++);
    }
}

namespace arch {
	namespace raspi {
		namespace serial {
			const U32 aux_uart_clock = 500000000;

			U32 baud_to_reg(U32 baud) {
				return aux_uart_clock/(baud*8)-1;
			}

			void init() {
				#ifdef ARCH_RASPI_UART0
					{ //uart 0 (untested)
						// Disable UART0
						mmio::write(mmio::Address::uart0_cr, 0x00000000);

						mailbox::PropertyMessage tags[2];
						tags[0].tag = mailbox::PropertyTag::set_clock_rate;
						tags[0].data.clock_rate.clockId = 2;
						tags[0].data.clock_rate.rate = 4000000; //4Mhz
						tags[0].data.clock_rate.skipSettingTurbo = 0;

						tags[1].tag = mailbox::PropertyTag::null_tag;

						if(!mailbox::send_messages(tags)){
							//TODO:error in some way?
						}

						// Setup the GPIO pin 14 && 15
						// Disable pull up/down for all GPIO pins & delay for 150 cycles
						mmio::write(mmio::Address::gppud, 0x00000000);
						mmio::delay(150);

						// Disable pull up/down for pin 14,15 & delay for 150 cycles
						mmio::write(mmio::Address::gppudclk0, 1<<14 | 1<<15);
						mmio::delay(150);

						// Write 0 to GPPUDCLK0 to make it take effect
						mmio::write(mmio::Address::gppudclk0, 0x00000000);

						// Clear any interrupts pending
						mmio::write(mmio::Address::uart0_icr, 0x7FF);

						// Set integer & fractional part of baud rate.
						// Divider = UART_CLOCK/(16 * Baud)
						// Fraction part register = (Fractional part * 64) + 0.5
						// UART_CLOCK = 3000000; Baud = 115200.

						// // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
						mmio::write(mmio::Address::uart0_ibrd, 1);
						// // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
						mmio::write(mmio::Address::uart0_fbrd, 40);

						// mmio::write(mmio::Address::uart0_ibrd, 2);
						// mmio::write(mmio::Address::uart0_fbrd, 11);

						// Enable FIFO & 8bit data transmissions (1 stop bit, no parity)
						mmio::write(mmio::Address::uart0_lcrh, 1<<4 | 1<<5 | 1<<6);

						// Mask all interrupts
						mmio::write(mmio::Address::uart0_imsc, 1<<1 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 | 1<<10);

						// Enable UART0, receive & transfer part of UART.
						mmio::write(mmio::Address::uart0_cr, 1<<0 | 1<<8 | 1<<9);

						{
							stdio::Section section("arch::raspi::serial::init...");
							stdio::print_info("UART0 active");
						}
					}
				#endif

				#ifdef ARCH_RASPI_UART1
					{ //uart 1
						// initialize UART
						mmio::write(mmio::Address::uart1_enable, mmio::read(mmio::Address::uart1_enable)|1); // enable UART1, AUX mini uart
						mmio::write(mmio::Address::uart1_mu_cntl, 0);
						mmio::write(mmio::Address::uart1_mu_lcr, 3); // 8 bits
						mmio::write(mmio::Address::uart1_mu_mcr, 0);
						mmio::write(mmio::Address::uart1_mu_ier, 0);
						mmio::write(mmio::Address::uart1_mu_iir, 0xc6); // disable interrupts
						mmio::write(mmio::Address::uart1_mu_baud, baud_to_reg(115200));


						// map UART1 to GPIO pins
						U32 r = mmio::read(mmio::Address::gpfsel1);
						r &= ~((7<<12)|(7<<15)); // gpio14, gpio15
						r |= (2<<12)|(2<<15); // alt5
						mmio::write(mmio::Address::gpfsel1, r);
						mmio::write(mmio::Address::gppud, 0); // enable pins 14 and 15
						mmio::delay(150);
						mmio::write(mmio::Address::gppudclk0, (1<<14)|(1<<15));
						mmio::delay(150);
						mmio::write(mmio::Address::gppudclk0, 0); // flush GPIO setup
						mmio::write(mmio::Address::uart1_mu_cntl, 3); // enable Tx, Rx

						{
							stdio::Section section("arch::raspi::serial::init...");
							stdio::print_info("UART1 active");
						}
					}
				#endif
			}
		}
	}
}
