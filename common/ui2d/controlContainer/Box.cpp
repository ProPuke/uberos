#include "Box.hpp"

#include <common/maths.hpp>
#include <common/ui2d/LayoutControl.hpp>

namespace ui2d::controlContainer {
	/**/ Box::Box(Gui &gui):
		Super(gui)
	{}

	/**/ Box::Box(Gui &gui, LayoutContainer *container):
		Super(gui, container)
	{}

	auto Box::get_min_size() -> UVec2 {
		auto minSize = this->minSize;

		switch(direction){
			case Direction::vertical: {
				for(auto child:children){
					auto childMin = child->get_min_size();
					minSize.x = maths::max(minSize.x, childMin.x);
					minSize.y += childMin.y;
				}

				if(children.length>1){
					minSize.y += spacing * (children.length-1);
				}
			} break;
			case Direction::horizontal: {
				for(auto child:children){
					auto childMin = child->get_min_size();
					minSize.x += childMin.x;
					minSize.y = maths::max(minSize.y, childMin.y);
				}

				if(children.length>1){
					minSize.x += spacing * (children.length-1);
				}
			} break;
		}

		return minSize;
	}

	auto Box::get_max_size() -> UVec2 {
		auto maxSize = this->maxSize;

		UVec2 maxComponentSize{0,0};

		switch(direction){
			case Direction::vertical: {
				for(auto child:children){
					auto childMax = child->get_max_size();
					maxComponentSize.x = maths::max(maxComponentSize.x, childMax.x);
					maxComponentSize.y = maths::add_safe(maxComponentSize.y, childMax.y);
				}
		
				if(children.length>1){
					maxComponentSize.y = maths::add_safe(maxComponentSize.y, spacing * (children.length-1));
				}
			} break;
			case Direction::horizontal: {
				for(auto child:children){
					auto childMax = child->get_max_size();
					maxComponentSize.x = maths::add_safe(maxComponentSize.x, childMax.x);
					maxComponentSize.y = maths::max(maxComponentSize.y, childMax.y);
				}
		
				if(children.length>1){
					maxComponentSize.x = maths::add_safe(maxComponentSize.x, spacing * (children.length-1));
				}
			} break;
		}

		return {
			expandX>0?maths::max(maxSize.x, maxComponentSize.x):maths::min(maxSize.x, maxComponentSize.x),
			expandY>0?maths::max(maxSize.y, maxComponentSize.y):maths::min(maxSize.y, maxComponentSize.y)
		};
	}

	void Box::set_spacing(U32 set) {
		if(spacing==set) return;
		spacing = set;
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	void Box::set_direction(Direction set) {
		if(direction==set) return;

		direction = set;
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	void Box::set_alignment(float set) {
		if(alignment==set) return;

		alignment = set;
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	void Box::layout() {
		auto minSize = get_min_size();

		const auto rect = _get_rect();

		switch(direction){
			case Direction::vertical: {
				auto remainingSpace = rect.height()>(I32)minSize.y?(U32)rect.height()-minSize.y:0u;

				bool maxedIndexes[children.length] = {}; //TODO: space-efficient dynamic bitmask

				auto totalExpansion = 0.0;
				for(auto child:children) totalExpansion += child->expandY;

				auto remainingSpaceFilled = 0u;

				if(remainingSpace>0){
					testChildrenY:

					remainingSpaceFilled = 0;
					// test all children, clamping their size to max if they would exceed
					for(auto i=0u;i<children.length;i++){
						if(maxedIndexes[i]) continue;
						auto child = children[i];
						auto minChildSize = child->get_min_size();
						auto maxChildSize = child->get_max_size();
						auto fill = (U32)(remainingSpace*child->expandY/totalExpansion+0.5);
						if(minChildSize.y+fill>maxChildSize.y){
							fill = maxChildSize.y-minChildSize.y;
							maxedIndexes[i] = true; // this child is maxed
							remainingSpace -= fill; // the effective growth we clamped at
							totalExpansion -= child->expandY;
							goto testChildrenY; // start testing again, with the new remaining total and the children that remain
						}

						remainingSpaceFilled += fill;
					}
				}

				auto y = 0u;

				if(remainingSpaceFilled<remainingSpace){
					y += (remainingSpace-remainingSpaceFilled)*alignment+0.5;
				}

				for(auto i=0u;i<children.length;i++){
					auto child = children[i];
					auto minChildSize = child->get_min_size();

					auto childWidth = maths::clamp((U32)rect.width(), child->minSize.x, child->maxSize.x);
					auto childHeight = maxedIndexes[i]?child->get_max_size().y:minChildSize.y+(U32)(remainingSpace*(child->expandY/totalExpansion)+0.5f);
		
					auto x = (I32)childWidth<rect.width()?((U32)rect.width()-childWidth)/2:0;
		
					child->set_rect({
						(I32)(rect.x1 + x), (I32)(rect.y1 + y),
						(I32)(rect.x1 + x+(I32)childWidth), (I32)(rect.y1 + y+(I32)childHeight)
					});
		
					y += childHeight;
					y += spacing;
				}
			} break;
			case Direction::horizontal: {
				auto remainingSpace = rect.width()>(I32)minSize.x?(U32)rect.width()-minSize.x:0u;

				bool maxedIndexes[children.length] = {}; //TODO: space-efficient dynamic bitmask

				auto totalExpansion = 0.0;
				for(auto child:children) totalExpansion += child->expandX;

				auto remainingSpaceFilled = 0u;

				if(remainingSpace>0){
					testChildrenX:

					remainingSpaceFilled = 0;
					// test all children, clamping their size to max if they would exceed
					for(auto i=0u;i<children.length;i++){
						if(maxedIndexes[i]) continue;
						auto child = children[i];
						auto minChildSize = child->get_min_size();
						auto maxChildSize = child->get_max_size();
						auto fill = (U32)(remainingSpace*child->expandX/totalExpansion+0.5);
						if(minChildSize.x+fill>maxChildSize.x){
							fill = maxChildSize.x-minChildSize.x;
							maxedIndexes[i] = true; // this child is maxed
							remainingSpace -= fill; // the effective growth we clamped at
							totalExpansion -= child->expandX;
							goto testChildrenX; // start testing again, with the new remaining total and the children that remain
						}

						remainingSpaceFilled += fill;
					}
				}

				auto x = 0u;

				if(remainingSpaceFilled<remainingSpace){
					x += (remainingSpace-remainingSpaceFilled)*alignment+0.5;
				}

				for(auto i=0u;i<children.length;i++){
					auto child = children[i];
					auto minChildSize = child->get_min_size();

					auto childWidth = maxedIndexes[i]?child->get_max_size().x:minChildSize.x+(U32)(remainingSpace*(child->expandX/totalExpansion)+0.5f);
					auto childHeight = maths::clamp((U32)rect.height(), child->minSize.y, child->maxSize.y);
		
					auto y = (I32)childHeight<rect.height()?((U32)rect.height()-childHeight)/2:0;
		
					child->set_rect({
						(I32)(rect.x1 + x), (I32)(rect.y1 + y),
						(I32)(rect.x1 + x+(I32)childWidth), (I32)(rect.y1 + y+(I32)childHeight)
					});
		
					x += childWidth;
					x += spacing;
				}
			} break;
		}
	}
}
