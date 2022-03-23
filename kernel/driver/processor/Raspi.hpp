#include <kernel/driver/Processor.hpp>

namespace driver {
	namespace processor {
		struct Raspi: driver::Processor {
			/**/ Raspi(const char *name, const char *processor_arch):
				Processor(name, processor_arch, "cpu")
			{}

			auto get_temperature_count() -> U32 override;
			auto get_temperature_name(U32 index) -> const char* override;
			auto get_temperature_value(U32 index) -> F32 override;
			auto get_temperature_max(U32 index) -> F32 override;

			auto get_voltage_count() -> U32 override;
			auto get_voltage_name(U32 index) -> const char* override;
			auto get_voltage_value(U32 index) -> F32 override;
			auto get_voltage_min(U32 index) -> F32 override;
			auto get_voltage_max(U32 index) -> F32 override;
			auto can_set_voltage(U32 index) -> bool override;
			auto set_voltage_value(U32 index, F32 set) -> bool override;

			auto get_clock_count() -> U32 override;
			auto get_clock_name(U32 index) -> const char* override;
			auto get_clock_value(U32 index) -> U32 override;
			auto get_clock_active_value(U32 index) -> U32 override;
			auto get_clock_min(U32 index) -> U32 override;
			auto get_clock_max(U32 index) -> U32 override;
			auto can_set_clock(U32 index) -> bool override;
			auto set_clock_value(U32 index, U32 set) -> bool override;
		};
	}
}
