#pragma once

#include <common/types.hpp>
#include <common/stdlib.hpp>

// #include <common/OrderedList.hpp>

enum struct ProcessLogType {
	info,
	debug,
	verbose,
	warning,
	error
};

inline const char *processLogTypeName[(U32)ProcessLogType::error+1] = {
	"info",
	"debug",
	"verbose",
	"warning",
	"error",
};

// struct ProcessLogItem {
// 	ProcessLogType type;
// 	String line;
// }

struct Process;

struct ProcessLog {
	// OrderedList<ProcessLogItem> items;

	/**/ ProcessLog(Process &process):
		process(process)
	{}

	Process &process;
	U32 indent = 0;

	template<typename ...Params>
	void print_info(Params ...params){ return print(ProcessLogType::info, params...); }
	template<typename ...Params>
	void print_debug(Params ...params){ return print(ProcessLogType::debug, params...); }
	template<typename ...Params>
	void print_verbose(Params ...params){ return print(ProcessLogType::verbose, params...); }
	template<typename ...Params>
	void print_warning(Params ...params){ return print(ProcessLogType::warning, params...); }
	template<typename ...Params>
	void print_error(Params ...params){ return print(ProcessLogType::error, params...); }

	template<typename ...Params>
	void print(ProcessLogType type, Params ...params){
		print_start(type);
		(_print(params), ...);
		print_end();
	}

	void print_start(ProcessLogType type);
	void print_end();
	template<typename Type>
	inline void _print(Type x){ return puts(::to_string(x)); }
	inline void _print(char x){ return putc(x); }
	inline void _print(const char *x){ return puts(x); }

	void putc(char);
	void puts(const char *);

	template<typename ...Params>
	void beginSection(Params ...params){
		print(ProcessLogType::info, params...);
		indent++;
	}

	inline void endSection(){
		indent--;
		print(ProcessLogType::info, "OK");
	}

	struct Section {
		/**/ Section(ProcessLog &log):
			log(log)
		{}

		ProcessLog &log;

		template<typename ...Params>
		/**/ Section(Params ...params){
			log.beginSection(params...);
		}

		/**/~Section(){
			log.endSection();
		}
	};

	Section section(){ return Section(*this); }
};
