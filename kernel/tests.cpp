#include "tests.hpp"

#include "tests/fontTest.hpp"
#include "tests/keyboardTest.hpp"
#include "tests/memoryTest.hpp"
#include "tests/taskbar.hpp"
#include "tests/driveList.hpp"

namespace tests {
	void run() {
		taskbar::run();
		fontTest::run();
		keyboardTest::run();
		memoryTest::run();
		driveList::run();
	}
}
