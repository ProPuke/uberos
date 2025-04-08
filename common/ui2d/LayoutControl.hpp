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

		UVec2 size{0,0}; // *requested* size (use _get_rect() for actual effective)
		UVec2 minSize{0,0};
		UVec2 maxSize{(U32)~0,(U32)~0};
		float expandX = 1.0;
		float expandY = 1.0;

		virtual void set_size(U32 x, U32 y);
		virtual auto get_min_size() -> UVec2 { return minSize; }
		virtual void set_min_size(U32 x, U32 y);
		virtual auto get_max_size() -> UVec2 { return maxSize; }
		virtual void set_max_size(U32 x, U32 y);
		virtual void set_fixed_size(U32 x, U32 y);
		virtual void set_expand(float x, float y);

		virtual auto _get_rect() -> graphics2d::Rect = 0;
		virtual void set_rect(graphics2d::Rect) = 0;

		virtual void redraw(bool flush = true) = 0;
	};

	template <typename Control>
	struct LayoutControl:Control, LayoutControlBase {
		typedef Control Super;

		template <typename ...Params>
		/**/ LayoutControl(Gui &gui, LayoutContainer *container, Params ...params);

		void set_size(U32 x, U32 y) override;
		auto get_min_size() -> UVec2 override { return minSize; }
		auto get_max_size() -> UVec2 override { return maxSize; }

		auto _get_rect() -> graphics2d::Rect override { return this->rect; }
		void set_rect(graphics2d::Rect set) override { this->rect = set; }

		void redraw(bool flush = true) override {
			Control::redraw(flush);
		}
	};

	struct LayoutContainer: LayoutControl<Control> {
		typedef LayoutControl<Control> Super;

		PodArray<LayoutControlBase*> children;

		/**/ LayoutContainer(Gui &gui, LayoutContainer *container = nullptr):
			Super(gui, container)
		{}

		template <typename ControlType, typename ...Params>
		auto add_control(Params ...params) -> LayoutControl<ControlType>&;
		template <typename ContainerType, typename ...Params>
		auto add_container_control(Params ...params) -> ContainerType&;
		virtual void remove_control(LayoutControlBase&);

		virtual void _on_children_changed();
		virtual void _on_resized();

		virtual void layout();

		void set_rect(graphics2d::Rect) override;

		void redraw(bool flush = true) override;
	};
}

#include "LayoutControl.inl"
