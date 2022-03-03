#pragma once

#include "../BufferReader.hpp"

namespace fileStash {
	class Directory;
	
	class File {
		friend Directory;

		U64 position;
		U64 _size;
		bool _is_eof = true;

		File(Buffer &buffer, U64 position):
			position(position)
		{
			BufferReader reader(buffer);

			if(reader.read_u32()!=('F'<<24|'I'<<16|'L'<<8|'E')){
				_size = 0;
				return;
			}

			_size = reader.read_u64();
		}

		public:

		bool size() { return _size; }
	};
}
