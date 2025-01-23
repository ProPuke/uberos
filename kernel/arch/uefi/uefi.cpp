#include "uefi.hpp"

namespace uefi {
	SystemTable *systemTable = nullptr;

	void init(SystemTable *systemTable) {
		uefi::systemTable = systemTable;
	}
}
