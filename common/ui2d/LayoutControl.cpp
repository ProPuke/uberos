#include "LayoutControl.hpp"

namespace ui2d {
	void LayoutControlBase::set_size(U32 x, U32 y) {
		if(size.x==x&&size.y==y) return;

		auto oldEffectiveX = maths::clamp(size.x, minSize.x, maxSize.x);
		auto oldEffectiveY = maths::clamp(size.y, minSize.y, maxSize.y);

		size.x = x;
		size.y = y;

		auto newEffectiveX = maths::clamp(size.x, minSize.x, maxSize.x);
		auto newEffectiveY = maths::clamp(size.y, minSize.y, maxSize.y);

		if(container&&(newEffectiveX!=oldEffectiveX||newEffectiveY!=oldEffectiveY)){
			container->_on_children_changed();
		}
	}

	void LayoutControlBase::set_min_size(U32 x, U32 y) {
		if(minSize.x==x&&minSize.y==y) return;

		minSize.x = x;
		minSize.y = y;

		if(container) container->_on_children_changed();
	}

	void LayoutControlBase::set_max_size(U32 x, U32 y) {
		if(maxSize.x==x&&maxSize.y==y) return;

		maxSize.x = x;
		maxSize.y = y;

		if(container) container->_on_children_changed();
	}

	void LayoutControlBase::set_fixed_size(U32 x, U32 y) {
		if(minSize.x==x&&minSize.y==y&&maxSize.x==x&&maxSize.y==y) return;

		minSize.x = x;
		minSize.y = y;
		maxSize.x = x;
		maxSize.y = y;

		if(container) container->_on_children_changed();
	}

	void LayoutControlBase::set_expand(float x, float y) {
		if(expandX==x&&expandY==y) return;

		expandX = x;
		expandY = y;

		if(container) container->_on_children_changed();
	}
}
