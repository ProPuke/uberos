#include "Driver.hpp"

const char * Driver::state_name[(U64)Driver::State::max+1] = {
	"disabled",
	"enabled",
	"enabling",
	"disabling",
	"restarting",
};

/**/ Driver::~Driver() {

}
