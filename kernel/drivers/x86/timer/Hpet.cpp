#include "Hpet.hpp"

#include <kernel/drivers.hpp>
#include <kernel/drivers/x86/system/Acpi.hpp>

namespace driver {
	namespace timer {
		namespace {
			struct __attribute__((packed)) AddressStructure {
				U8 addressSpaceId;    // 0 - system memory, 1 - system I/O
				U8 registerBitWidth;
				U8 registerBitOffset;
				U8 :8;
				U64 address;
			};

			struct __attribute__((packed)) DescriptionTable: system::Acpi::Sdt {
				U8 hardwareRevId;
				U8 comparatorCount:5;
				U8 counterSize:1;
				U8 :1;
				U8 legacyReplacement:1;
				U16 pciVendorId;
				AddressStructure address;
				U8 hpetNumber;
				U16 minimumTick;
				U8 pageProtection;
			};

			struct __attribute__((packed)) Registers {
				struct __attribute__((packed)) Capabilities {
					U8 revisionId;
					U8 lastTimerCount:5; // number of counters-1
					bool supports64Bit:1;
					U8 :1;
					bool supportsLegacyReplacement:1;
					U16 vendorId;
					U32 counterClockPeriod; // in femtoseconds
				};

				volatile U64 _capabilities;

				U64 :64;

				struct __attribute__((packed)) Config {
					bool enableCnf:1;
					bool enableLegacyReplacementMapping:1;
					U64 :62;
				};

				volatile U64 _configuration;

				U64 :64;

				struct __attribute__((packed)) InterruptStatus {
					U32 activeOrClearInterrupt:32; // bits read as 1 when they're active, but you write 1 to the bits to clear them (writing 0 does nothing)
					U32 :32;

					auto isInterruptActive(U8 timer) volatile -> bool {
						return activeOrClearInterrupt & 1<<timer;
					}

					void clearInterrupt(U8 timer) volatile {
						activeOrClearInterrupt = 1<<timer;
					}

					void clearAllInterrupts() volatile {
						activeOrClearInterrupt = 0xffffffff;
					}
				};

				volatile U64 _interruptStatus;

				U8 _reserved[0xf0 - 0x28];

				union __attribute__((packed)) {
					volatile U64 _64bit;
					volatile U32 _32bit;
				} mainCounter;

				U64 :64;

				struct __attribute__((packed)) Timer {
					struct __attribute__((packed)) Config {
						bool :1;
						enum struct Type {
							edge, // one edge interrupt per trigger
							level // one level interrupt that stays active until the the general interrupt status register is written
						} type:1;
						bool enableInterrupts:1;
						bool enablePeriodic:1;
						bool supportsPeriodic:1; // READONLY
						enum struct Size {
							_32bit,
							_64bit
						} size:1; // READONLY
						bool setPeriodicTimer:1;
						bool :1;
						bool force32bit:1;
						U8 interruptRoute:5; // which interrupt to use. Read back after to ensure it was set and that interrupt was supported. Note that in legacy replacement mode this is ignored for timers 0 and 1
						bool enableFsb:1; // send as FSB messages rather than APIC - This ignores the interrupt settings here, and uses fsbInterruptRoute instead
						bool supportsFsb:1; // READONLY
						U16 :16;
						U32 supportedInterrupts; // READONLY - a bitmask of supported interrupts for interruptRoute (bit 4 == interrupt 4 etc.)
					};

					volatile U64 _config;

					union __attribute__((packed)) Comparator {
						// trigger the interrupt when it hits this value. If set to periodic mode the target will be changed on each trigger, incremented by the last value WE wrote here
						U64 _64bit;
						U32 _32bit;
					} volatile comparator;

					struct __attribute__((packed)) FsbInterruptRoute {
						U32 interruptValue;
						U32 interruptAddress;
					};

					volatile U64 _fsbInterruptRoute;

					U64 :64;

					auto read_config() -> Config { return reinterpret_value<Config>(_config); }
					void write_config(Config &set) { _config = reinterpret_value<U64>(set); }

					auto read_fsbInterruptRoute() -> FsbInterruptRoute { return reinterpret_value<FsbInterruptRoute>(_fsbInterruptRoute); }
					void write_fsbInterruptRoute(FsbInterruptRoute &set) { _fsbInterruptRoute = reinterpret_value<U64>(set); }
				} timer[32];

				auto read_capabilities() -> Capabilities { return reinterpret_value<Capabilities>(_capabilities); }

				auto read_config() -> Config { return reinterpret_value<Config>(_configuration); }
				void write_config(Config &set) { _configuration = reinterpret_value<U64>(set); }

				auto read_interruptStatus() -> InterruptStatus { return reinterpret_value<InterruptStatus>(_interruptStatus); }
				void write_interruptStatus(InterruptStatus &set) { _interruptStatus = reinterpret_value<U64>(set); }
			};

			DriverReference<system::Acpi> acpi;

			Registers *registers = nullptr;
			bool is64bit = false;
			U32 clockCount;
			U32 clockPeriod;
			U8 irqToClockId[32];

			struct Clock {
				void (*callback)(void*);
				void *callbackData;
			};
			Clock clock[32];

			TimerQueue timerQueue(Hpet::instance);

			auto enable() -> Try<> {
				auto config = registers->read_config();
				config.enableCnf = true;
				registers->write_config(config);

				if(!registers->read_config().enableCnf) return {"Unable to enable"};

				return {};
			}

			void disable() {
				auto config = registers->read_config();
				config.enableCnf = false;
				registers->write_config(config);
			}

			void disable_legacyReplacement() {
				auto config = registers->read_config();
				config.enableLegacyReplacementMapping = false;
				registers->write_config(config);
			}

			auto enable_legacyReplacement() -> Try<> {
				auto capabilities = registers->read_capabilities();

				if(!capabilities.supportsLegacyReplacement) return {"Legacy mode not supported"};

				auto config = registers->read_config();
				config.enableLegacyReplacementMapping = true;
				registers->write_config(config);
			}

			void clearAllInterrupts() {
				auto interruptStatus = registers->read_interruptStatus();
				interruptStatus.clearAllInterrupts();
				registers->write_interruptStatus(interruptStatus);
			}
		}

		auto Hpet::_on_start() -> Try<> {
			acpi = drivers::find_and_activate<system::Acpi>();
			if(!acpi) return {"ACPI unavailable"};

			auto table = (DescriptionTable*)acpi->find_entry_with_signature("HPET");
			if(!table) return {"HPET not present"};

			memset(irqToClockId, -1, sizeof(irqToClockId));

			// TRY(api.subscribe_memory((void*)table->address.address, 0x137));
			TRY(api.subscribe_memory((void*)table->address.address, sizeof(Registers)));

			// log.print_info("HPET present at ", (void*)table->address.address);

			registers = (Registers*)table->address.address;

			auto capabilities = registers->read_capabilities();

			log.print_info("vendor: ", format::Hex16{capabilities.vendorId}, " revision ", capabilities.revisionId);

			disable();
			disable_legacyReplacement();

			is64bit = capabilities.supports64Bit;
			clockCount = capabilities.lastTimerCount+1;
			clockPeriod = capabilities.counterClockPeriod;

			log.print_info("timers: ", clockCount, " x ", is64bit?"64bit":"32bit", " precision");

			clearAllInterrupts();

			{
				for(auto i=0u; i<clockCount; i++){
					auto config = registers->timer[i].read_config();

					auto irqRequest = api.subscribe_available_irq({(U64)config.supportedInterrupts, 0, 0, 0});

					if(!irqRequest){
						clockCount = i;
						log.print_warning("Not enough supported IRQs for all timers - Count dropped to ", clockCount);
						if(clockCount<1) return {"No timers available"};
						break;
					}

					auto irq = irqRequest.result;

					config.enableFsb = false;
					config.enableInterrupts = false;
					config.enablePeriodic = false;
					config.force32bit = false;
					config.interruptRoute = irq;
					config.type = Registers::Timer::Config::Type::edge;
					registers->timer[i].write_config(config);

					{
						auto config = registers->timer[i].read_config();
						if(config.interruptRoute!=irq){
							log.print_error("Unable to set timer ", i, " to irq ", irq);
							return {"Failed setting timer irq"};
						}
					}

					irqToClockId[irq] = i;

					irq++;
				}
			}

			TRY(enable());

			return {};
		}

		auto Hpet::_on_stop() -> Try<> {
			memset(irqToClockId, -1, sizeof(irqToClockId));

			disable();
			clearAllInterrupts();

			return {};
		}

		void Hpet::_on_irq(U8 irq) {
			auto clockId = irqToClockId[irq];
			if(clockId>=clockCount) return;

			auto interruptStatus = registers->read_interruptStatus();
			if(!interruptStatus.isInterruptActive(clockId)) return;

			clock[clockId].callback(clock[clockId].callbackData);
		}

		auto Hpet::now() -> U32 {
			return (U32)(registers->mainCounter._32bit*(U64)clockPeriod/1000000000);
		}

		auto Hpet::now64() -> U64 {
			#ifdef HAS_128BIT
				return (U64)((is64bit?registers->mainCounter._64bit:registers->mainCounter._32bit)*(U128)clockPeriod/1000000000);

			#else
				if(is64bit){
					auto ticks = registers->mainCounter._64bit;
					return ticks/1000000000 * clockPeriod + ticks%1000000000 * clockPeriod / 1000000000;

				}else{
					return registers->mainCounter._32bit*(U64)clockPeriod/1000000000;
				}
			#endif
		}

		auto Hpet::get_timer_count() -> U8 {
			return clockCount;
		}

		void Hpet::set_timer(U8 index, U32 usecs, Callback callback, void *data) {
			if(index>=clockCount) {
				log.print_error("Invalid timer timer id specified (", index, ") - This device has ", clockCount, " timers");
				return;
			}

			clock[index].callback = callback;
			clock[index].callbackData = data;

			auto time = now64()+max(1u, usecs); // precision guarenteed to be less than 1 us, so at least 1us is fine

			if(is64bit){
				registers->timer[index].comparator._64bit = time;
			}else{
				registers->timer[index].comparator._32bit = time;
			}

			auto config = registers->timer[index].read_config();
			config.enableInterrupts = true;
			registers->timer[index].write_config(config);
		}

		void Hpet::stop_timer(U8 index) {
			if(index>=clockCount) {
				log.print_error("Invalid timer timer id specified (", index, ") - This device has ", clockCount, " timers");
				return;
			}

			auto config = registers->timer[index].read_config();
			config.enableInterrupts = false;
			registers->timer[index].write_config(config);
		}

		void Hpet::schedule(U32 usecs, Callback callback, void *data) {
			timerQueue.schedule(usecs, callback, data);
		}

		void Hpet::schedule_important(U32 usecs, Callback callback, void *data) {
			timerQueue.schedule_important(usecs, callback, data);
		}
	}
}