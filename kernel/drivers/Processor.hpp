#pragma once

#include <kernel/Driver.hpp>

namespace driver {
	struct Processor: Driver {
		typedef Driver Super;

		static DriverType driverType;

		const char *processor_arch;
		U32 processor_cores = 1;

		/**/ Processor(const char *name, const char *processor_arch, const char *description);

		auto can_disable_driver() -> bool override { return false; }
		auto can_restart_driver() -> bool override { return false; }

		auto _on_stop() -> bool override { return false; };

		//temps in K
		virtual auto get_temperature_count() -> U32 { return 0; }
		virtual auto get_temperature_name(U32 index) -> const char* { return ""; }
		virtual auto get_temperature_value(U32 index) -> F32 { return 0; }
		virtual auto get_temperature_max(U32 index) -> F32 { return 0; }

		//volts in V
		virtual auto get_voltage_count() -> U32 { return 0; }
		virtual auto get_voltage_name(U32 index) -> const char* { return ""; }
		virtual auto get_voltage_value(U32 index) -> F32 { return 0; }
		virtual auto get_voltage_min(U32 index) -> F32 { return 0; }
		virtual auto get_voltage_max(U32 index) -> F32 { return 0; }
		virtual auto can_set_voltage(U32 index) -> bool { return false; }
		virtual auto set_voltage_value(U32 index, F32 set) -> bool { return false; }

		//clocks in Hz
		virtual auto get_clock_count() -> U32 { return 0; }
		virtual auto get_clock_name(U32 index) -> const char* { return ""; }
		virtual auto get_clock_value(U32 index) -> U32 { return 0; } // what it's set to
		virtual auto get_clock_active_value(U32 index) -> U32 { return 0; } // what it's currently *actively* doing (based on automatic adaption etc). May return 0 if not supported
		virtual auto get_clock_min(U32 index) -> U32 { return 0; }
		virtual auto get_clock_max(U32 index) -> U32 { return 0; }
		virtual auto get_clock_default(U32 index) -> U32 { return (get_clock_min(index)+get_clock_max(index))/2; }
		virtual auto can_set_clock(U32 index) -> bool { return false; }
		virtual auto set_clock_value(U32 index, U32 set) -> bool { return false; }
	};
}