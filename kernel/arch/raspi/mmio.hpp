#pragma once

#include <common/types.hpp>
#include <common/stdlib.hpp>
#include <kernel/mmio.hpp>

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/mmio/barrier.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmio/barrier.hpp>
#else
	#error "Unsupported architecture"
#endif

namespace mmio {
	// #if defined(ARCH_ARM32)
	// 	using namespace arch::arm32;
	// #elif defined(ARCH_ARM64)
	// 	using namespace arch::arm64;
	// #else
	// 	#error "Unsupported architecture"
	// #endif
}

namespace mmio {
	namespace arch {
		namespace raspi {
			enum struct Address: U32 {
				#if defined(ARCH_RASPI1)
					gpu_peripheral_base = 0x20000000,
					gpu_peripheral_length = 0x01000000,
				#elif defined(ARCH_RASPI2)
					gpu_peripheral_base = 0x3F000000,
					gpu_peripheral_length = 0x01000000,
				#elif defined(ARCH_RASPI3)
					gpu_peripheral_base = 0x3F000000,
					gpu_peripheral_length = 0x01000000,
				#elif defined(ARCH_RASPI4)
					gpu_peripheral_base = 0xFE000000,
					gpu_peripheral_length = 0x01800000, //might be wrong?
				#else
					#error "Unknown model"
				#endif

				local_peripheral_base = gpu_peripheral_base + gpu_peripheral_length,

				system_timer_base  = gpu_peripheral_base + 0x3000,
				interrupts_base    = gpu_peripheral_base + 0xB000,
				mail0_base         = gpu_peripheral_base + 0xB880,
				rstc               = gpu_peripheral_base + 0x10001c,
				rsts               = gpu_peripheral_base + 0x100020,
				wdog               = gpu_peripheral_base + 0x100024,
				gpio_base          = gpu_peripheral_base + 0x200000,
				uart0_base         = gpu_peripheral_base + 0x201000,
				uart1_base         = gpu_peripheral_base + 0x215000,
				emmc_base          = gpu_peripheral_base + 0x300000,
				usb_base           = gpu_peripheral_base + 0x980000,

				core_timer_base    = local_peripheral_base,
				// core_timer_base    = gpu_peripheral_base + 0x3000,

				interrupts_pending = interrupts_base + 0x200,

				mail0_read    = mail0_base + 0x00,
				mail0_status  = mail0_base + 0x18,
				mail0_write   = mail0_base + 0x20,

				gpfsel0       = gpio_base + 0x00,
				gpfsel1       = gpio_base + 0x04,
				gpfsel2       = gpio_base + 0x08,
				gpfsel3       = gpio_base + 0x0C,
				gpfsel4       = gpio_base + 0x10,
				gpfsel5       = gpio_base + 0x14,
				gpset0        = gpio_base + 0x1C,
				gpset1        = gpio_base + 0x20,
				gpclr0        = gpio_base + 0x28,
				gplev0        = gpio_base + 0x34,
				gplev1        = gpio_base + 0x38,
				gpeds0        = gpio_base + 0x40,
				gpeds1        = gpio_base + 0x44,
				gphen0        = gpio_base + 0x64,
				gphen1        = gpio_base + 0x68,
				gppud         = gpio_base + 0x94,
				gppudclk0     = gpio_base + 0x98,
				gppudclk1     = gpio_base + 0x9C,

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

				uart1_enable     = uart1_base + 0x04,
				uart1_mu_io      = uart1_base + 0x40,
				uart1_mu_ier     = uart1_base + 0x44,
				uart1_mu_iir     = uart1_base + 0x48,
				uart1_mu_lcr     = uart1_base + 0x4C,
				uart1_mu_mcr     = uart1_base + 0x50,
				uart1_mu_lsr     = uart1_base + 0x54,
				uart1_mu_msr     = uart1_base + 0x58,
				uart1_mu_scratch = uart1_base + 0x5C,
				uart1_mu_cntl    = uart1_base + 0x60,
				uart1_mu_stat    = uart1_base + 0x64,
				uart1_mu_baud    = uart1_base + 0x68,

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

				// not sure these are correct or valid on pi, as they cause a crash on pi 1 and always read as 0 on others. Docs may be wrong
				// core_timer_control   = core_timer_base + 0x00,
				// core_timer_prescaler = core_timer_base + 0x08
			};

			void write_address(Address reg, U32 data);
			U32 read_address(Address reg);

			struct PeripheralAccessGuard {
				/**/ PeripheralAccessGuard(){ barrier(); };
				/**/~PeripheralAccessGuard(){ barrier(); };

				/**/ PeripheralAccessGuard(const PeripheralAccessGuard&) = delete;
				PeripheralAccessGuard& operator=(const PeripheralAccessGuard&) = delete;
			};

			struct PeripheralReadGuard {
				/**/ PeripheralReadGuard(){ barrier(); };
				/**/~PeripheralReadGuard(){ barrier(); };

				/**/ PeripheralReadGuard(const PeripheralReadGuard&) = delete;
				PeripheralReadGuard& operator=(const PeripheralReadGuard&) = delete;
			};

			struct PeripheralWriteGuard {
				/**/ PeripheralWriteGuard(){ barrier(); };
				/**/~PeripheralWriteGuard(){ barrier(); };

				/**/ PeripheralWriteGuard(const PeripheralWriteGuard&) = delete;
				PeripheralWriteGuard& operator=(const PeripheralWriteGuard&) = delete;
			};
		}
	}
}

#include "mmio.inl"

#if defined(ARCH_ARM32)
	#include <kernel/arch/arm32/mmio/barrier.hpp>
#elif defined(ARCH_ARM64)
	#include <kernel/arch/arm64/mmio/barrier.hpp>
#else
	#error "Unsupported architecture"
#endif
