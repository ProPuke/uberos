#include "WrapBox.hpp"

#include <common/maths.hpp>
#include <common/ui2d/LayoutControl.hpp>

namespace ui2d::controlContainer {
	/**/ WrapBox::WrapBox(Gui &gui, Direction direction):
		Super(gui),
		direction(direction)
	{}

	/**/ WrapBox::WrapBox(Gui &gui, LayoutContainer *container, Direction direction):
		Super(gui, container),
		direction(direction)
	{}

	//TODO: fixed size support

	auto WrapBox::get_min_size() -> IVec2 {
		// we can't conditionally constrain on both axis at once (due to wrap), so just return the absolute min of both axis (which is the largest minimum element size)

		auto minSize = this->minSize;
		for(auto child:children){
			if(!child->get_is_visible()) continue;

			minSize = max(minSize, child->get_min_size());
		}

		return minSize;
	}

	void WrapBox::set_spacing(I32 set) {
		if(spacing==set) return;
		spacing = set;
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	void WrapBox::set_direction(Direction set) {
		if(direction==set) return;

		direction = set;
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	void WrapBox::set_alignment(float set) {
		if(alignment==set) return;

		alignment = set;
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	void WrapBox::layout() {
		const auto rect = _get_rect();

		switch(direction){
			case Direction::vertical: {
				auto from = 0u;
				auto to = from;
				auto x = 0;

				do{
					auto length = 0;
					auto rowSize = 0;
					for(;to<children.length;to++){
						if(!children[to]->get_is_visible()) continue;

						auto minChildSize = children[to]->get_min_size();
						auto newLength = length+(to>from?spacing:0)+minChildSize.y;
						if(to>from&&(I32)newLength>rect.height()) {
							break;
						}
						rowSize = maths::max(rowSize, minChildSize.x);
						length = newLength;
					}

					if(fillSpace){
						auto remainingSpace = rowSize<rect.height()?rect.height()-rowSize:0;

						bool maxedIndexes[to-from] = {}; //TODO: space-efficient dynamic bitmask

						auto totalExpansion = 0.0;
						for(auto child:children) {
							if(!child->get_is_visible()) continue;

							totalExpansion += child->expandY;
						}
		
						auto remainingSpaceFilled = 0;

						if(remainingSpace>0){
							testChildrenY:
		
							remainingSpaceFilled = 0;
							// test all children, clamping their size to max if they would exceed
							for(auto i=from;i<to;i++){
								if(maxedIndexes[i-from]) continue;
								auto &child = *children[i];
								if(!child.get_is_visible()) continue;

								auto minChildSize = child.get_min_size();
								auto maxChildSize = child.get_max_size();
								auto fill = (I32)(remainingSpace*child.expandY/totalExpansion+0.5);
								if(minChildSize.y+fill>maxChildSize.y){
									fill = maxChildSize.y-minChildSize.y;
									maxedIndexes[i-from] = true; // this child is maxed
									remainingSpace -= fill; // the effective growth we clamped at
									totalExpansion -= child.expandY;
									goto testChildrenY; // start testing again, with the new remaining total and the children that remain
								}
		
								remainingSpaceFilled += fill;
							}
						}

						auto y = 0;

						if(remainingSpaceFilled<remainingSpace){
							y += (remainingSpace-remainingSpaceFilled)*alignment+0.5;
						}
		
						for(auto i=from;i<to;i++){
							auto &child = *children[i];
							if(!child.get_is_visible()) continue;

							auto minChildSize = child.get_min_size();
							auto maxChildSize = child.get_max_size();
		
							auto childWidth = maths::clamp(rowSize, minChildSize.x, maxChildSize.x);
							auto childHeight = maxedIndexes[i-from]?maxChildSize.y:minChildSize.y+(I32)(remainingSpace*(child.expandX/totalExpansion)+0.5f);
				
							auto childX = x + (childWidth<rowSize?(rowSize-childWidth)/2:0);

							child.set_rect({
								rect.x1 + childX, rect.y1 + y,
								rect.x1 + childX + childWidth, rect.y1 + y + childHeight
							});
				
							y += childHeight;
							y += spacing;
						}

					}else{
						auto y = 0;
						for(auto i=from;i<to;i++){
							auto &child = *children[i];
							if(!child.get_is_visible()) continue;

							auto minChildSize = child.get_min_size();
							auto maxChildSize = child.get_max_size();

							auto childWidth = maths::clamp(rowSize, minChildSize.x, maxChildSize.x);
							auto childHeight = minChildSize.y;

							auto childX = x + (I32)(childWidth<rowSize?(rowSize-childWidth)/2:0);

							child.set_rect({
								rect.x1 + childX, rect.y1 + y,
								rect.x1 + childX + (I32)childWidth, rect.x1 + y + (I32)childHeight
							});

							y += childHeight;
							y += spacing;
						}
					}

					x += rowSize + spacing;
					from = to;

				}while(to<children.length);
			} break;
			case Direction::horizontal: {
				auto from = 0u;
				auto to = from;
				auto y = 0;

				do{
					auto length = 0;
					auto rowSize = 0;
					for(;to<children.length;to++){
						if(!children[to]->get_is_visible()) continue;

						auto minChildSize = children[to]->get_min_size();
						auto newLength = length+(to>from?spacing:0)+minChildSize.x;
						if(to>from&&(I32)newLength>rect.width()) break;
						rowSize = maths::max(rowSize, minChildSize.y);
						length = newLength;
					}

					if(fillSpace){
						auto remainingSpace = (I32)rowSize<rect.width()?rect.width()-rowSize:0;

						bool maxedIndexes[to-from] = {}; //TODO: space-efficient dynamic bitmask

						auto totalExpansion = 0.0;
						for(auto child:children){
							if(!child->get_is_visible()) continue;

							totalExpansion += child->expandX;
						}
		
						auto remainingSpaceFilled = 0;

						if(remainingSpace>0){
							testChildrenX:
		
							remainingSpaceFilled = 0;
							// test all children, clamping their size to max if they would exceed
							for(auto i=from;i<to;i++){
								if(maxedIndexes[i-from]) continue;
								auto &child = *children[i];
								if(!child.get_is_visible()) continue;

								auto minChildSize = child.get_min_size();
								auto maxChildSize = child.get_max_size();
								auto fill = (I32)(remainingSpace*child.expandX/totalExpansion+0.5);
								if(minChildSize.x+fill>maxChildSize.x){
									fill = maxChildSize.x-minChildSize.x;
									maxedIndexes[i-from] = true; // this child is maxed
									remainingSpace -= fill; // the effective growth we clamped at
									totalExpansion -= child.expandX;
									goto testChildrenX; // start testing again, with the new remaining total and the children that remain
								}
		
								remainingSpaceFilled += fill;
							}
						}

						auto x = 0;

						if(remainingSpaceFilled<remainingSpace){
							x += (remainingSpace-remainingSpaceFilled)*alignment+0.5;
						}
		
						for(auto i=from;i<to;i++){
							auto &child = *children[i];
							if(!child.get_is_visible()) continue;

							auto minChildSize = child.get_min_size();
							auto maxChildSize = child.get_max_size();
		
							auto childWidth = maxedIndexes[i-from]?maxChildSize.x:minChildSize.x+(I32)(remainingSpace*(child.expandX/totalExpansion)+0.5f);
							auto childHeight = maths::clamp(rowSize, minChildSize.y, maxChildSize.y);
				
							auto childY = y + (I32)(childHeight<rowSize?(rowSize-childHeight)/2:0);
				
							child.set_rect({
								rect.x1 + x, rect.y1 + childY,
								rect.x1 + (I32)(x+(I32)childWidth), (I32)(rect.y1 + childY + (I32)childHeight)
							});
				
							x += childWidth;
							x += spacing;
						}

					}else{
						auto x = 0;
						for(auto i=from;i<to;i++){
							auto &child = *children[i];
							if(!child.get_is_visible()) continue;

							auto minChildSize = child.get_min_size();
							auto maxChildSize = child.get_max_size();

							auto childWidth = minChildSize.x;
							auto childHeight = maths::clamp(rowSize, minChildSize.y, maxChildSize.y);

							auto childY = y + (I32)(childHeight<rowSize?(rowSize-childHeight)/2:0);

							child.set_rect({
								rect.x1 + x, rect.y1 + childY,
								rect.x1 + (I32)(x+childWidth), rect.y1 + childY + (I32)childHeight
							});

							x += childWidth;
							x += spacing;
						}
					}

					y += rowSize + spacing;
					from = to;

				}while(to<children.length);
			} break;
		}
	}
}
