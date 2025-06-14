#pragma once

#include <common/PodArray.hpp>
#include <common/ui2d/Control.hpp>
#include <common/ui2d/LayoutControl.hpp>

namespace ui2d {
	struct LayoutContainer;

	struct LayoutControlBase {
		LayoutContainer *container = nullptr;

		/*   */ /**/ LayoutControlBase(Gui &gui, LayoutContainer *container):
			container(container)
		{}
		virtual /**/~LayoutControlBase(){}

		IVec2 size{0,0}; // *requested* size (use _get_rect() for actual effective)
		IVec2 minSize{0,0};
		IVec2 maxSize{0x7fff'ffff, 0x7fff'ffff};
		float expandX = 1.0;
		float expandY = 1.0;

		virtual void set_size(I32 x, I32 y);
		virtual auto get_min_control_size() -> IVec2 { return {0, 0}; }
		virtual auto get_max_control_size() -> IVec2 { return {0x7fff'ffff, 0x7fff'ffff}; }
		virtual auto get_min_size() -> IVec2 { return max(minSize, get_min_control_size()); }
		virtual auto get_max_size() -> IVec2 { return min(maxSize, get_max_control_size()); }
		virtual void set_min_size(I32 x, I32 y);
		virtual void set_max_size(I32 x, I32 y);
		virtual void set_fixed_size(I32 x, I32 y);
		virtual void set_expand(float x, float y);

		virtual auto get_is_visible() -> bool = 0;
		virtual void set_is_visible(bool set) = 0;

		virtual auto _get_rect() -> graphics2d::Rect = 0;
		virtual void set_rect(graphics2d::Rect) = 0;

		virtual void layout(){}

		virtual void redraw(bool flush = true) = 0;
	};

	template <typename Control>
	struct LayoutControl:Control, LayoutControlBase {
		typedef Control Super;

		template <typename ...Params>
		/**/ LayoutControl(Gui &gui, LayoutContainer *container, Params &&...params);

		void set_size(I32 x, I32 y) override;
		auto get_min_control_size() -> IVec2 override { return Control::get_min_size(); }
		auto get_max_control_size() -> IVec2 override { return Control::get_max_size(); }

		auto get_is_visible() -> bool override { return Control::isVisible; }
		void set_is_visible(bool set) { Control::isVisible = set; }

		auto _get_rect() -> graphics2d::Rect override { return this->rect; }
		void set_rect(graphics2d::Rect set) override { this->rect = set; }

		void redraw(bool flush = true) override {
			Control::redraw(flush);
		}
	};

	struct LayoutContainer: LayoutControl<Control> {
		typedef LayoutControl<Control> Super;

		PodArray<LayoutControlBase*> children;

		bool isVisible = true;

		/**/ LayoutContainer(Gui &gui, LayoutContainer *container = nullptr):
			Super(gui, container)
		{}

		template <typename ControlType, typename ...Params>
		auto add_control(Params &&...params) -> LayoutControl<ControlType>&;
		template <typename ContainerType, typename ...Params>
		auto add_container_control(Params &&...params) -> ContainerType&;
		virtual void remove_control(LayoutControlBase&);

		auto get_is_visible() -> bool override { return isVisible; }
		void set_is_visible(bool set) { isVisible = set; }

		virtual void _on_children_changed();
		virtual void _on_resized();

		void set_rect(graphics2d::Rect) override;

		void redraw(bool flush = true) override;
	};
}

#include "LayoutControl.inl"
