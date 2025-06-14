#pragma once

#include "LayoutControl.hpp"

namespace ui2d {
	inline void LayoutContainer::remove_control(LayoutControlBase &control) {
		if(control.container!=this) return;

		for(auto i=0u;i<children.length;i++){
			if(children[i]==&control){
				children.remove(i);
				break;
			}
		}

		// control.container = nullptr;
		delete &control;
		_on_children_changed();
	}

	template <typename ControlType, typename ...Params>
	inline auto LayoutContainer::add_control(Params &&...params) -> LayoutControl<ControlType>& {
		const auto control = new LayoutControl<ControlType>(gui, this, std::forward<Params>(params)...);
		children.push_back(control);
		control->container = this;
		_on_children_changed();
		return *control;
	}

	template <typename ContainerType, typename ...Params>
	inline auto LayoutContainer::add_container_control(Params &&...params) -> ContainerType& {
		const auto control = new ContainerType(gui, this, std::forward<Params>(params)...);
		children.push_back(control);
		control->container = this;
		_on_children_changed();
		return *control;
	}

	inline void LayoutContainer::_on_children_changed() {
		if(container){
			container->_on_children_changed();
		}else{
			layout();
		}
	}

	inline void LayoutContainer::_on_resized() {
		layout();
	}

	inline void LayoutContainer::set_rect(graphics2d::Rect rect) {
		// disabled as we always want to re-layout
		// if(this->rect==rect) return;

		Super::set_rect(rect);
		layout();
	}

	inline void LayoutContainer::redraw(bool flush) {
		// no need, as the gui draws these
		// for(auto child:children){
		// 	child->redraw(flush);
		// }
	}

	template <typename Control>
	template <typename ...Params>
	/**/ LayoutControl<Control>::LayoutControl(Gui &gui, LayoutContainer *container, Params &&...params):
		Super(gui, {0,0,0,0}, params...),
		LayoutControlBase(gui, container)
	{
		if(container){
			container->_on_children_changed();
		}
	}

	template <typename Control>
	void LayoutControl<Control>::set_size(I32 x, I32 y) {
		if(size.x==x&&size.y==y) return;

		auto oldEffectiveX = maths::clamp(size.x, minSize.x, maxSize.x);
		auto oldEffectiveY = maths::clamp(size.y, minSize.y, maxSize.y);

		size.x = x;
		size.y = y;

		auto newEffectiveX = maths::clamp(size.x, minSize.x, maxSize.x);
		auto newEffectiveY = maths::clamp(size.y, minSize.y, maxSize.y);

		if(newEffectiveX!=oldEffectiveX||newEffectiveY!=oldEffectiveY){
			auto rect = _get_rect();
			set_rect({rect.x1, rect.y1, rect.x1+(I32)newEffectiveX, rect.y1+(I32)newEffectiveY});

			if(container){
				container->_on_children_changed();
			}
		}
	}
}
