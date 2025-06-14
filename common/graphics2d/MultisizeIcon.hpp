#pragma once

#include <common/graphics2d/Buffer.hpp>
#include <common/ListOrdered.hpp>
#include <common/types.hpp>

namespace graphics2d {
	class MultisizeIcon {
	protected:
		Buffer **buffers;
		U32 count;

	public:
		/**/ MultisizeIcon():
			count(0)
		{}

		/**/ MultisizeIcon(U32 count, Buffer *buffers[]):
			buffers(buffers),
			count(count)
		{}

		auto get_size_or_larger_or_smaller(U32 size) -> Buffer* {
			for(auto i=0u;i<count;i++){
				auto &buffer = buffers[i];
				if(buffer->height>=size) return buffer;
			}

			return count>0?buffers[count-1]:nullptr;
		}

		auto get_size_or_smaller(U32 size) -> Buffer* {
			for(auto i=0u;i<count;i++){
				auto &buffer = buffers[i];
				if(buffer->height>size) return i>0?buffers[i-1]:nullptr;
			}

			return count>0?buffers[count-1]:nullptr;
		}

		auto get_max_size() -> U32 {
			return count>0?buffers[count-1]->height:0;
		}

		operator bool() { return count>0; }
		auto operator!() -> bool { return count==0; }
	};

	class DynamicMultisizeIcon: protected MultisizeIcon {
		ListOrdered<Buffer*> list;

	public:
		template <typename ...Buffers>
		/**/ DynamicMultisizeIcon(Buffers& ...buffer){
			(add_icon(buffer), ...);
		}

		void add_icon(Buffer &buffer){
			for(auto i=0u;i<list.length;i++){
				if(list[i]->height==buffer.height){
					list[i] = &buffer;
					return;

				}else if(list[i]->height>buffer.height){
					list.insert(i, &buffer);
					return;
				}
			}
			list.push_back(&buffer);
			buffers = &list[0];
			count = list.length;
		}

		void remove_icon(Buffer &buffer){
			for(auto i=0u;i<list.length;i++){
				if(list[i]==&buffer){
					list.remove(i);
					buffers = &list[0];
					count = list.length;
					return;
				}
			}
		}

		void clear() {
			list.clear();
			buffers = &list[0];
			count = 0;
		}
	};
}
