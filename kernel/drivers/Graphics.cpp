#include "Graphics.hpp"

#include <kernel/Log.hpp>

static Log log("gfx");

namespace driver {
	auto Graphics::set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion) -> Try<> {
		// if acceptSuggestion, prioritise the following alternative options:
		// 1) Same resolution, with closest > format
		// 2) Same resolution, with closest < format
		// 3) Higher resolution, with closest >= format
		// 4) Higher resolution, with closest < format
		// 5) lower resolution, with closest >= forma t
		// 6) lower resolution, with closest < format

		Optional<U32> bestCandidate;
		Optional<I32> bestResolutionDistance;
		framebuffer::Mode bestMode = { 0 };
		Optional<I32> bestFormatDistance;

		const auto modeCount = get_mode_count(framebufferId);
		for(U32 index=0;index<modeCount;index++){
			auto mode = get_mode(framebufferId, index);

			if(mode.width==width&&mode.height==height&&mode.format==format){
				return set_mode(framebufferId, index);
			}

			auto resolutionDistance = (I32)mode.width*(I32)mode.height-(I32)width*(I32)height;
			auto formatDistance = (I32)mode.format-(I32)format;

			if(
				!bestResolutionDistance||
				*bestResolutionDistance==resolutionDistance||
				*bestResolutionDistance<0&&resolutionDistance>*bestResolutionDistance||
				*bestResolutionDistance>0&&resolutionDistance>0&&resolutionDistance<*bestResolutionDistance
			){
				bestResolutionDistance = resolutionDistance;

				// if a different (but better) resolution, then prioritise above anything else
				if(width!=bestMode.width||height!=bestMode.height){
					bestFormatDistance = formatDistance;
					bestCandidate = index;
					bestMode = mode;

				}else if(
					!bestFormatDistance||
					*bestFormatDistance<0&&formatDistance>*bestFormatDistance||
					*bestFormatDistance>0&&formatDistance>0&&formatDistance<*bestFormatDistance
				){
					bestFormatDistance = formatDistance;
					bestCandidate = index;
					bestMode = mode;
				}
			}
		}

		if(bestCandidate){
			log.print_warning("Warning: Unable to switch to mode ", width, "x", height, ", ", to_string(format));
			log.print_info("falling back to ", bestMode.width, "x", bestMode.height, ", ", to_string(bestMode.format));

			return set_mode(framebufferId, *bestCandidate);
		}else{
			return {"Could not find matching suitable mode"};
		}
	}
}
