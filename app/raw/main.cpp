#include <common/types.hpp>
#include <common/ipc.hpp>

extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));
extern void _init (void);

I32 main();
I32 ipc_main(ipc::Id ipcId, void *ipcPacket);

I32 entrypoint(ipc::Id ipcId, void *ipcPacket) {
	switch(ipcId){
		case ipc::Id::init:
			auto &packet = *(ipc::init_packet*)ipcPacket;

			for(auto func=__preinit_array_start; func!=__preinit_array_end; func++) {
				(*func)();
			}

			_init();

			for(auto func=__init_array_start; func!=__init_array_end; func++) {
				(*func)();
			}

			packet.response.versionSupported = 1;
		return 0;

		case ipc::Id::start_main1:
			auto &packet = *(ipc::start_main1_packet*)ipcPacket;
		return main();

		case ipc::Id::custom ... ipc::Id::custom_max:
		return ipc_main(ipcId, ipcPacket);

		default:
			//unsupported
		return 1;
	}
}

I32 main() {
	return 0;
}

I32 ipc_main(ipc::Id ipcId, void *ipcPacket) {
	return 1;
}
