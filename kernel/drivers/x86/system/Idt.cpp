#include "Idt.hpp"

#include <kernel/Spinlock.hpp>
#include <kernel/drivers/x86/system/Gdt.hpp>

namespace driver::system {
	namespace {
		Spinlock spinlock("idt");

		struct __attribute__((packed)) Idtr {
			U16 limit; // the last valid byte (size-1)
			// U32 entriesAddress;
			void *entries;
		};

		#ifdef _64BIT
			struct __attribute__((packed)) Entry {
				U16 isr_low;      // The lower 16 bits of the ISR's address
				U16 kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
				U8 ist;           // The IST in the TSS that the CPU will load into RSP; set to zero for now
				U8 attributes;
				U16 isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
				U32 isr_high;     // The higher 32 bits of the ISR's address
				U32 _reserved = 0;
			};
		#else
			struct __attribute__((packed)) Entry {
				U16 isr_low;      // The lower 16 bits of the ISR's address
				U16 kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
				U8 _reserved = 0; // Set to zero
				U8 attributes;    // Type and attributes; see the IDT page
				U16 isr_high;     // The higher 16 bits of the ISR's address
			};
		#endif

		__attribute__((aligned(0x10)))
		Entry entries[256];
		unsigned entryCount = 0;

		Idtr idtr {
			0,
			// (size_t)&entries
			&entries
		};

		DriverReference<system::Gdt> gdt(nullptr, [](void*){
			Idt::instance.log.print_warning("GDT drivers halted - terminating");
			if(!drivers::stop_driver(Idt::instance)){
				Idt::instance.api.fail_driver("GDT drivers halted");
			}
		}, nullptr);

		// only for use in realmode
		extern "C" void save_idt_real(U32 address);
		extern "C" void load_idt_real(U32 address);

		#ifdef _64BIT
			void _set_entry(U8 i, void *isr, U8 flags) {
				auto &entry = entries[i];

				entry.isr_low = (U32)(size_t)isr & 0xffff;
				entry.kernel_cs = gdt->kernelCodeOffset;
				entry.ist = 0;
				entry.attributes = flags;
				entry.isr_mid = (U32)(size_t)isr>>16 & 0xffff;
				entry.isr_high = (U32)(size_t)isr>>16 & 0xffffffff;
				entry._reserved = 0;

				entryCount = max(entryCount, (unsigned)i+1);
			}
		#else
			void _set_entry(U8 i, void *isr, U8 flags) {
				auto &entry = entries[i];

				entry.isr_low = (U32)isr & 0xffff;
				entry.kernel_cs = gdt->kernelCodeOffset;
				entry.attributes = flags;
				entry.isr_high = (U32)isr>>16;
				entry._reserved = 0;

				entryCount = max(entryCount, (unsigned)i+1);
			}
		#endif

		void _apply_entries() {
			idtr.limit = sizeof(Entry) * entryCount - 1;
			asm volatile(
				"lidt %0"
				:
				: "m"(idtr)
				: "memory"
			);
		}
	}

	auto Idt::_on_start() -> Try<> {
		gdt = drivers::find_and_activate<driver::system::Gdt>();

		if(!gdt){
			return {"GDT not found"}; //TODO: better message
		}

		return {};
	}

	auto Idt::_on_stop() -> Try<> {
		return {};
	}

	void Idt::set_gate_trap(U8 i, void *isr) {
		Spinlock_Guard guard(spinlock);

		return _set_entry(i, isr, 0x8f);
	}

	void Idt::set_gate_interrupt(U8 i, void *isr) {
		Spinlock_Guard guard(spinlock);

		return _set_entry(i, isr, 0x8e);
	}

	void Idt::apply_gates() {
		Spinlock_Guard guard(spinlock);

		return _apply_entries();
	}
}
