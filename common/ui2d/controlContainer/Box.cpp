#include "Box.hpp"

#include <common/maths.hpp>
#include <common/ui2d/LayoutControl.hpp>

namespace ui2d::controlContainer {
	/**/ Box::Box(Gui &gui, Direction direction):
		Super(gui),
		direction(direction)
	{}

	/**/ Box::Box(Gui &gui, LayoutContainer *container, Direction direction):
		Super(gui, container),
		direction(direction)
	{}

	auto Box::get_min_size() -> IVec2 {
		auto minSize = IVec2{};

		switch(direction){
			case Direction::vertical: {
				auto count = 0;

				for(auto child:children){
					if(!child->get_is_visible()) continue;

					auto childMin = child->get_min_size();
					minSize.y += childMin.y;
					minSize.x = maths::max(minSize.x, child->get_min_size().x);
					count++;
				}

				if(count>1){
					minSize.y += spacing * (count-1);
				}
			} break;
			case Direction::horizontal: {
				auto count = 0;

				for(auto child:children){
					if(!child->get_is_visible()) continue;

					auto childMin = child->get_min_size();
					minSize.x += childMin.x;
					minSize.y = maths::max(minSize.y, child->get_min_size().y);
					count++;
				}

				if(count>1){
					minSize.x += spacing * (count-1);
				}
			} break;
		}

		switch(style){
			case Style::none:
			break;
			case Style::padded: {
				auto spacing = gui.theme->get_component_spacing();
				minSize.x += spacing*2;
				minSize.y += spacing*2;
			} break;
			case Style::border: {
				auto boxClient = gui.theme->get_box_client_area({0,0,100,100}, Theme::BoxType::default_);
				minSize.x += boxClient.x1 + (100-boxClient.x2);
				minSize.y += boxClient.y1 + (100-boxClient.y2);
			} break;
			case Style::inset: {
				auto boxClient = gui.theme->get_box_client_area({0,0,100,100}, Theme::BoxType::inset);
				minSize.x += boxClient.x1 + (100-boxClient.x2);
				minSize.y += boxClient.y1 + (100-boxClient.y2);
			} break;
		}

		return max(minSize, this->minSize);
	}

	auto Box::get_max_size() -> IVec2 {
		auto maxSize = this->maxSize;

		IVec2 maxComponentSize{0,0};

		switch(direction){
			case Direction::vertical: {
				auto count = 0;

				for(auto child:children){
					if(!child->get_is_visible()) continue;

					auto childMax = child->get_max_size();
					maxComponentSize.x = maths::max(maxComponentSize.x, childMax.x);
					maxComponentSize.y = maths::add_safe(maxComponentSize.y, childMax.y);

					count++;
				}
		
				if(count>1){
					maxComponentSize.y = maths::add_safe(maxComponentSize.y, spacing * (count-1));
				}
			} break;
			case Direction::horizontal: {
				auto count = 0;

				for(auto child:children){
					if(!child->get_is_visible()) continue;

					auto childMax = child->get_max_size();
					maxComponentSize.x = maths::add_safe(maxComponentSize.x, childMax.x);
					maxComponentSize.y = maths::max(maxComponentSize.y, childMax.y);

					count++;
				}
		
				if(count>1){
					maxComponentSize.x = maths::add_safe(maxComponentSize.x, spacing * (count-1));
				}
			} break;
		}

		maxSize = {
			expandX>0?maths::max(maxSize.x, maxComponentSize.x):maths::min(maxSize.x, maxComponentSize.x),
			expandY>0?maths::max(maxSize.y, maxComponentSize.y):maths::min(maxSize.y, maxComponentSize.y)
		};

		switch(style){
			case Style::none:
			break;
			case Style::padded: {
				auto spacing = gui.theme->get_component_spacing();
				maxSize.x = maths::add_safe(maxSize.x, spacing*2);
				maxSize.y = maths::add_safe(maxSize.y, spacing*2);
			} break;
			case Style::border: {
				auto boxClient = gui.theme->get_box_client_area({0,0,100,100}, Theme::BoxType::default_);
				maxSize.x = maths::add_safe(maxSize.x, boxClient.x1 + (100-boxClient.x2));
				maxSize.y = maths::add_safe(maxSize.y, boxClient.y1 + (100-boxClient.y2));
			} break;
			case Style::inset: {
				auto boxClient = gui.theme->get_box_client_area({0,0,100,100}, Theme::BoxType::inset);
				maxSize.x = maths::add_safe(maxSize.x, boxClient.x1 + (100-boxClient.x2));
				maxSize.y = maths::add_safe(maxSize.y, boxClient.y1 + (100-boxClient.y2));
			} break;
		}

		return min(maxSize, this->maxSize);
	}

	void Box::set_spacing(I32 set) {
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

	void Box::set_style(Style set) {
		if(style==set) return;

		style = set;
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	void Box::layout() {
		const auto outerRect = _get_rect();
		auto rect = outerRect;
		auto minSize = get_min_size();
		auto maxSize = get_max_size();

		switch(style){
			case Style::none:
			break;
			case Style::padded: {
				auto spacing = gui.theme->get_component_spacing();
				rect = rect.cropped(spacing, spacing, spacing, spacing);
			} break;
			case Style::border:
				rect = gui.theme->get_box_client_area(rect, Theme::BoxType::default_);
			break;
			case Style::inset:
				rect = gui.theme->get_box_client_area(rect, Theme::BoxType::inset);
			break;
		}

		minSize.x -= (rect.x1-outerRect.x1)+(outerRect.x2-rect.x2);
		minSize.y -= (rect.x1-outerRect.x1)+(outerRect.x2-rect.x2);
		maxSize.x -= (rect.x1-outerRect.x1)+(outerRect.x2-rect.x2);
		maxSize.y -= (rect.x1-outerRect.x1)+(outerRect.x2-rect.x2);

		switch(direction){
			case Direction::vertical: {
				auto remainingSpace = rect.height()>minSize.y?rect.height()-minSize.y:0;

				bool maxedIndexes[children.length] = {}; //TODO: space-efficient dynamic bitmask

				auto totalExpansion = 0.0;
				for(auto child:children){
					if(!child->get_is_visible()) continue;

					totalExpansion += child->expandY;
				}

				auto remainingSpaceFilled = 0;

				if(remainingSpace>0){
					testChildrenY:

					remainingSpaceFilled = 0;
					// test all children, clamping their size to max if they would exceed
					for(auto i=0u;i<children.length;i++){
						if(maxedIndexes[i]) continue;
						auto child = children[i];
						if(!child->get_is_visible()) continue;

						auto minChildSize = child->get_min_size();
						auto maxChildSize = child->get_max_size();
						auto fill = remainingSpace*child->expandY/totalExpansion+0.5;
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

				auto y = 0;

				if(remainingSpaceFilled<remainingSpace){
					y += (remainingSpace-remainingSpaceFilled)*alignment+0.5;
				}

				for(auto i=0u;i<children.length;i++){
					auto child = children[i];
					if(!child->get_is_visible()) continue;

					auto minChildSize = child->get_min_size();

					auto childHeight = maxedIndexes[i]?child->get_max_size().y:minChildSize.y+(U32)(remainingSpace*(child->expandY/totalExpansion)+0.5f);
					auto childWidth = maths::clamp(rect.width(), child->get_min_size().x, child->get_max_size().x);
		
					auto x = (I32)childWidth<rect.width()?(rect.width()-childWidth)/2:0;
		
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
				for(auto child:children){
					if(!child->get_is_visible()) continue;

					totalExpansion += child->expandX;
				}

				auto remainingSpaceFilled = 0u;

				if(remainingSpace>0){
					testChildrenX:

					remainingSpaceFilled = 0;
					// test all children, clamping their size to max if they would exceed
					for(auto i=0u;i<children.length;i++){
						if(maxedIndexes[i]) continue;
						auto child = children[i];
						if(!child->get_is_visible()) continue;

						auto minChildSize = child->get_min_size();
						auto maxChildSize = child->get_max_size();
						auto fill = (I32)(remainingSpace*child->expandX/totalExpansion+0.5);
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

				auto x = 0;

				if(remainingSpaceFilled<remainingSpace){
					x += (remainingSpace-remainingSpaceFilled)*alignment+0.5;
				}

				for(auto i=0u;i<children.length;i++){
					auto child = children[i];
					if(!child->get_is_visible()) continue;

					auto minChildSize = child->get_min_size();

					auto childWidth = maxedIndexes[i]?child->get_max_size().x:minChildSize.x+(I32)(remainingSpace*(child->expandX/totalExpansion)+0.5f);
					auto childHeight = maths::clamp(rect.height(), child->get_min_size().y, child->get_max_size().y);
		
					auto y = (I32)childHeight<rect.height()?(rect.height()-childHeight)/2:0;
		
					child->set_rect({
						rect.x1 + x, rect.y1 + y,
						rect.x1 + x+childWidth, rect.y1 + y+childHeight
					});
		
					x += childWidth;
					x += spacing;
				}
			} break;
		}
	}

	void Box::redraw(bool flush) {
		switch(style){
			case Style::none:
			break;
			case Style::padded:
			break;
			case Style::border:
				gui.theme->draw_box(gui.buffer, rect, Theme::BoxType::default_);
			break;
			case Style::inset:
				gui.theme->draw_box(gui.buffer, rect, Theme::BoxType::inset);
			break;
		}
	}
}
