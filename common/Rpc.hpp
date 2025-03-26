#pragma once

template <typename Type>
struct HasRpc {
	virtual void rpc(U32 id, void *buffer) {} // TODO: buffer type
};
