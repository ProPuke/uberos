#pragma once

// #include <common/MpscFifo.hpp>

template <typename Type>
struct HasRpc {
	virtual void rpc(U32 id, void *buffer) {} // TODO: buffer type

	struct Command {
		U32 id;
		U32 args[8];
	};

	// MpscFifo<Command> commandBuffer;
};
