#pragma once

#include <common/types.hpp>
#include <common/stdlib.hpp>

namespace arch {
	namespace raspi {
		namespace mmio {
			enum struct Address: U32 {
				#if defined(ARCH_RASPI1)
					peripheral_base = 0x20000000,
					peripheral_length = 0x01000000,
				#elif defined(ARCH_RASPI2)
					peripheral_base = 0x3F000000,
					peripheral_length = 0x01000000,
				#elif defined(ARCH_RASPI3)
					peripheral_base = 0x3F000000,
					peripheral_length = 0x01000000, //might be wrong?
				#elif defined(ARCH_RASPI4)
					peripheral_base = 0x3F000000, //might be wrong?
					peripheral_length = 0x01000000, //might be wrong?
				#else
					#error "Unknown model"
				#endif
				
				system_timer_base  = peripheral_base + 0x3000,
				interrupts_base    = peripheral_base + 0xB000,
				mail0_base         = peripheral_base + 0xB880,
				rstc               = peripheral_base + 0x10001c,
				rsts               = peripheral_base + 0x100020,
				wdog               = peripheral_base + 0x100024,
				gpio_base          = peripheral_base + 0x200000,
				uart0_base         = peripheral_base + 0x201000,
				emmc_base          = peripheral_base + 0x300000,
				usb_base           = peripheral_base + 0x980000,

				core_timer_base    = peripheral_base + peripheral_length,

				interrupts_pending = interrupts_base + 0x200,

				mail0_read    = mail0_base + 0x00,
				mail0_status  = mail0_base + 0x18,
				mail0_write   = mail0_base + 0x20,

				gppud         = gpio_base + 0x94,
				gppudclk0     = gpio_base + 0x98,

				uart0_dr      = uart0_base + 0x00,
				uart0_rsrecr  = uart0_base + 0x04,
				uart0_fr      = uart0_base + 0x18,
				uart0_ilpr    = uart0_base + 0x20,
				uart0_ibrd    = uart0_base + 0x24,
				uart0_fbrd    = uart0_base + 0x28,
				uart0_lcrh    = uart0_base + 0x2C,
				uart0_cr      = uart0_base + 0x30,
				uart0_ifls    = uart0_base + 0x34,
				uart0_imsc    = uart0_base + 0x38,
				uart0_ris     = uart0_base + 0x3C,
				uart0_mis     = uart0_base + 0x40,
				uart0_icr     = uart0_base + 0x44,
				uart0_dmacr   = uart0_base + 0x48,
				uart0_itcr    = uart0_base + 0x80,
				uart0_itip    = uart0_base + 0x84,
				uart0_itop    = uart0_base + 0x88,
				uart0_tdr     = uart0_base + 0x8C,

				usb_core_base = usb_base,
				usb_host_base = usb_base + 0x400,
				usb_power     = usb_base + 0xE00,

				usb_core_rx_fifo_siz      = usb_core_base + 0x024,
				usb_core_nper_tx_fifo_siz = usb_core_base + 0x028,
				usb_core_nper_tx_stat     = usb_core_base + 0x02C,	// RO
				usb_core_i2c_ctrl         = usb_core_base + 0x030,
				usb_core_phy_vendor_ctrl  = usb_core_base + 0x034,
				usb_core_gpio             = usb_core_base + 0x038,
				usb_core_user_id          = usb_core_base + 0x03C,
				usb_core_vendor_id        = usb_core_base + 0x040,
				usb_core_hw_cfg1          = usb_core_base + 0x044,
				usb_core_hw_cfg2          = usb_core_base + 0x048,
				usb_core_hw_cfg3          = usb_core_base + 0x04C,
				usb_core_hw_cfg4          = usb_core_base + 0x045,

				core_timer_control   = core_timer_base + 0x00,
				core_timer_prescaler = core_timer_base + 0x08
			};

			void write(Address reg, U32 data);
			U32 read(Address reg);
			void delay(I32 count);

			void barrier();

			struct PeripheralAccessGuard {
				/**/ PeripheralAccessGuard(){ barrier(); };
				/**/~PeripheralAccessGuard(){ barrier(); };

				/**/ PeripheralAccessGuard(const PeripheralAccessGuard&) = delete;
				PeripheralAccessGuard& operator=(const PeripheralAccessGuard&) = delete;
			};
		}
	}
}

#include "mmio.inl"

#if defined(ARCH_RASPI1) or defined(ARCH_RASPI2)
	#include "armv7/mmio/barrier.inl"

#elif defined(ARCH_RASPI3) or defined(ARCH_RASPI4)
	#include "armv8/mmio/barrier.inl"

#else
	#error "Unsupported architecture"
#endif
