#pragma once

#include <common/types.hpp>

#define IPC_LIST \
	ITEM(init)\
	ITEM(start_main1)

namespace ipc {
	enum struct Id:U32 {
		#define ITEM(NAME) NAME,
			IPC_LIST
		#undef ITEM

		custom = 0x8000'0000,
		custom_max
	};

	struct init_request {};
	struct init_response {
		U32 versionSupported;
	};

	struct start_main1_request {};
	struct start_main1_response {};

	#define ITEM(NAME) struct NAME##_packet {\
		NAME##_request request;\
		NAME##_response response;\
	};
		IPC_LIST
	#undef ITEM

	constexpr U32 packet_size[] = {
		#define ITEM(NAME) sizeof(NAME##_packet),
			IPC_LIST
		#undef ITEM
	};
}
