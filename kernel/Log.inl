#pragma once

#include "Log.hpp"

inline void Log::print_inline(char x) { logging::print_inline(x); }
inline void Log::print_inline(char *x) { logging::print_inline(x); }
inline void Log::print_inline(const char *x) { logging::print_inline(x); }
template<typename ...Params>
inline void Log::print_inline(Params ...params) { logging::print_inline(params...); }
inline void Log::print_end() { logging::print_end(); }

inline void Log::print_info_start() { logging::print_start(logging::PrintType::info); print_inline(name, ": "); }
inline void Log::print_debug_start() { logging::print_start(logging::PrintType::debug); print_inline(name, ": "); }
inline void Log::print_warning_start() { logging::print_start(logging::PrintType::warning); print_inline(name, ": "); }
inline void Log::print_error_start() { logging::print_start(logging::PrintType::error); print_inline(name, ": "); }

template<typename ...Params> inline void Log::print_info(Params ...params){ return logging::print(logging::PrintType::info, name, ": ", params...); }
template<typename ...Params> inline void Log::print_debug(Params ...params){ return logging::print(logging::PrintType::debug, name, ": ", params...); }
template<typename ...Params> inline void Log::print_warning(Params ...params){ return logging::print(logging::PrintType::warning, name, ": ", params...); }
template<typename ...Params> inline void Log::print_error(Params ...params){ return logging::print(logging::PrintType::error, name, ": ", params...); }

template<typename ...Params>
inline /**/ Log::Section:: Section(Params ...params){
	logging::beginSection(params...);
}

inline /**/ Log::Section::~Section(){
	logging::endSection();
}

template<typename ...Params>
inline auto Log::section(Params ...params) -> Section { return Section(name, ": ", params...); }
