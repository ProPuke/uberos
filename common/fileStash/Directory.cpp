#include "Directory.hpp"

#include "File.hpp"

namespace fileStash {
	File Directory::get_file(const C8 *name) {
		const auto nameLength = strlen(name);

		BufferReader reader(this->reader.buffer);
		reader.set_position(position);

		if(reader.read_u32()!=('D'<<16|'I'<<8|'R')){
			return File(reader.buffer, reader.buffer.size);
		}

		while(true){
			auto entryLength = reader.read_u8();
			if(!entryLength) break;

			auto entryName = &reader.buffer.data[reader.get_position()];

			reader.skip(entryLength);

			const auto target = reader.read_u64();

			if(entryLength==nameLength&&!memcmp(name, entryName, nameLength)){
				return File(reader.buffer, target);
			}
		}

		return File(reader.buffer, reader.buffer.size);
	}

	Directory Directory::get_directory(const C8 *name) {
		const auto nameLength = strlen(name);

		BufferReader reader(this->reader.buffer);
		reader.set_position(position);

		if(reader.read_u32()!=('D'<<16|'I'<<8|'R')){
			return Directory(reader.buffer, reader.buffer.size);
		}

		while(true){
			auto entryLength = reader.read_u8();
			if(!entryLength) break;

			auto entryName = &reader.buffer.data[reader.get_position()];

			reader.skip(entryLength);

			const auto target = reader.read_u64();

			if(entryLength==nameLength&&!memcmp(name, entryName, nameLength)){
				return Directory(reader.buffer, target);
			}
		}

		return Directory(reader.buffer, reader.buffer.size);
	}
}
