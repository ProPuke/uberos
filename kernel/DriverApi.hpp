#pragma once

#include <kernel/PodArray.hpp>

#include <common/Bool256.hpp>
#include <common/types.hpp>

struct Driver;

namespace drivers{
	void print_driver_summary(const char *indent, Driver&);
}

class DriverApi {
	public:
		enum struct State {
			disabled,
			enabled,
			active,
			failed,
			max = failed
		};

	protected:
		friend auto to_string(DriverApi::State state) -> const char*;
		friend void drivers::print_driver_summary(const char *indent, Driver&);

		static const char *state_name[(U64)State::max+1];

		State state = State::enabled;

		struct MemoryRange {
			void *start;
			void *end;
		};

		Bool256 subscribedIrqs;
		Bool256 subscribedInterrupts;
		bool    subscribedAllInterrupts = false;
		PodArray<MemoryRange> subscribedMemory;

	public:
		auto driver() -> Driver&;

		void subscribe_irq(U8);
		void unsubscribe_irq(U8);
		void unsubscribe_all_irqs();

		void subscribe_interrupt(U8);
		void unsubscribe_interrupt(U8);
		void unsubscribe_all_interrupts();

		auto subscribe_memory(void*, size_t) -> bool;
		void unsubscribe_memory(void*, size_t);
		void unsubscribe_all_memory();
		auto is_subscribed_to_memory(void*, size_t) -> bool;

		void enable_driver();
		void start_driver();
		void stop_driver();
		void restart_driver();

		auto is_enabled() -> bool { return state==State::enabled||state==State::active; }
		auto is_disabled() -> bool { return state==State::disabled; }
		auto is_active() -> bool { return state==State::active; }
		auto is_failed() -> bool { return state==State::failed; }
};

inline auto to_string(DriverApi::State state) -> const char* { return DriverApi::state_name[(U64)state]; }
