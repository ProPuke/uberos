#pragma once

#include "Buffer.hpp"

#include "stdlib.hpp"

class BufferReader {
	U64 position = 0;

	public:
	
	Buffer &buffer;

	/**/ BufferReader(Buffer &buffer):buffer(buffer){}

	bool is_eof() { return position>=buffer.size; }

	void set_position(U64 set) { position = set; }
	U64 get_position() { return position; }
	U64 get_size() { return buffer.size; }

	void skip(U64 bytes) { position += bytes; }

	U8  read_u8 () { if(position+1>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<U8 *>(&buffer.data[(position+=1)-1]); }
	I8  read_i8 () { if(position+1>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<I8 *>(&buffer.data[(position+=1)-1]); }
	U16 read_u16() { if(position+2>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<U16*>(&buffer.data[(position+=2)-2]); }
	I16 read_i16() { if(position+2>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<I16*>(&buffer.data[(position+=2)-2]); }
	U32 read_u32() { if(position+4>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<U32*>(&buffer.data[(position+=4)-4]); }
	I32 read_i32() { if(position+4>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<I32*>(&buffer.data[(position+=4)-4]); }
	U64 read_u64() { if(position+8>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<U64*>(&buffer.data[(position+=8)-8]); }
	I64 read_i64() { if(position+8>buffer.size) { position=buffer.size; return 0; } else return *reinterpret_cast<I64*>(&buffer.data[(position+=8)-8]); }

	void read_bytes(U8 *target, U64 bytes) {
		const auto remaining = buffer.size-position;
		if(remaining<bytes){
			memcpy(target, &buffer.data[position], remaining);
			bzero(&target[remaining], bytes-remaining);
			position = buffer.size;
		}else{
			memcpy(target, &buffer.data[position], bytes);
			position += bytes;
		}
	}
};
