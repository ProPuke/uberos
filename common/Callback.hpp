#pragma once

#include "LList.hpp"

struct Callback: LListItem<Callback> {
	/**/ Callback(void *data): data(data) {}

	void *data;
	virtual void call(void *data) = 0;
};
