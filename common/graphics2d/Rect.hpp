#pragma once

#include <common/types.hpp>
#include <common/stdlib.hpp>

namespace graphics2d {

	struct Rect {
		I32 x1=0, y1=0, x2=0, y2=0;

		auto offset(I32 x, I32 y) const -> Rect {
			return {x1+x, y1+y, x2+x, y2+y};
		}

		auto intersect(Rect rect) const -> Rect {
			Rect result = {
				max(x1, rect.x1),
				max(y1, rect.y1),
				min(x2, rect.x2),
				min(y2, rect.y2)
			};

			if(result.x2<=result.x1){
				result.x1 = 0;
				result.x2 = 0;
			}

			if(result.y2<=result.y1){
				result.y1 = 0;
				result.y2 = 0;
			}

			return result;
		}

		auto cropped(I32 left, I32 top, I32 right, I32 bottom) const {
			Rect result = {
				x1+left,
				y1+top,
				x2-right,
				y2-bottom
			};

			if(result.x2<=result.x1){
				result.x1 = 0;
				result.x2 = 0;
			}

			if(result.y2<=result.y1){
				result.y1 = 0;
				result.y2 = 0;
			}

			return result;
		}

		auto contains(I32 x, I32 y) const -> bool {
			return x>=x1&&x<x2&&y>=y1&&y<y2;
		}

		auto include(Rect with) const -> Rect {
			if(isNonzero()){
				return {
					min(x1, with.x1),
					min(y1, with.y1),
					max(x2, with.x2),
					max(y2, with.y2)
				};

			}else{
				// nothing is included yet, so just use the new area, don't extend from origin
				return with;
			}
		}

		void clear() {
			x1 = 0;
			y1 = 0;
			x2 = 0;
			y2 = 0;
		}

		auto width() const -> I32 { return x2-x1; }
		auto height() const -> I32 { return y2-y1; }

		bool isNonzero() const { return x1||y1||x2||y2; }

		auto operator==(const Rect &compare) const { return x1==compare.x1&&y1==compare.y1&&x2==compare.x2&&y2==compare.y2; }
		auto operator!=(const Rect &compare) const { return x1!=compare.x1||y1!=compare.y1||x2!=compare.x2||y2!=compare.y2; }
	};

}
