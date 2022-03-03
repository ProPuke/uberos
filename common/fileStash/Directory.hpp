#pragma once

#include "FileType.hpp"
#include "../BufferReader.hpp"

namespace fileStash {
	class File;
	class Directory;

	class DirectoryEntry {
		friend Directory;

		C8 name[256];
		FileType type = FileType::unknown;

		DirectoryEntry(){
			name[0] = 0;
		}

		public:

		C8* get_name() { return name; }
		FileType get_type() { return type; }
	};

	class Directory {
		BufferReader reader;
		U64 position;
		bool _is_eof = true;

		Directory(Buffer &buffer, U64 position):
			reader(buffer),
			position(position)
		{
			reset();
		}

		public:

		bool is_eof() { return _is_eof; }

		void reset() {
			reader.set_position(position);

			_is_eof = false;

			if(reader.read_u32()!=('D'<<16|'I'<<8|'R')){
				_is_eof = true;
				return;
			}
		}

		DirectoryEntry read() {
			DirectoryEntry entry;

			if(is_eof()) return entry;

			auto nameLength = reader.read_u8();
			if(!nameLength){
				_is_eof = true;
				return entry;
			}

			reader.read_bytes((U8*)(C8*)entry.name, nameLength);
			entry.name[nameLength] = 0;

			const auto target = reader.read_u64();

			const auto type = *reinterpret_cast<U32*>(&reader.buffer.data[target]);

			switch(type){
				case ('F'<<24|'I'<<16|'L'<<8|'E'):
					entry.type = FileType::file;
				break;
				case ('D'<<16|'I'<<8|'R'):
					entry.type = FileType::directory;
				break;
			}

			return entry;
		}

		File get_file(const C8 *name);
		Directory get_directory(const C8 *name);
	};
}
