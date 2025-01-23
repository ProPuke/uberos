#pragma once

#include <kernel/drivers/Serial.hpp>

#include <common/Try.hpp>

namespace driver::serial {
	//TODO: replace with Console driver that uses Serial?
	struct Stdout final: driver::Serial {
		DRIVER_INSTANCE(Stdout, "stdout", "Serial Stdout Interface", driver::Serial)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		void set_baud(U32 set) override;

		auto get_active_baud() -> U32 override;

		void putc(char c) override;
		void puts(const char *str) override;
		auto peekc() -> char override;
		auto getc() -> char override;
	};
}
