#pragma once

#include "CriticalSection.hpp"

#include "exceptions.hpp"

inline void CriticalSection::lock() {
	exceptions::lock();
}

inline void CriticalSection::unlock() {
	exceptions::unlock();
}
