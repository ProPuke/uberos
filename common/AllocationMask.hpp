#pragma once

#include <common/stdlib.hpp>
#include <common/Try.hpp>
#include <common/types.hpp>

struct AllocationMask {
	/**/ AllocationMask(Reg *data, UPtr _count):
		data(data),
		count(
			_count
			// round down to fit within 8 bits and Reg size
			/8*8
			/(8*sizeof(Reg))*(8*sizeof(Reg))
		)
	{
		debug::assert(count%8==0);
		debug::assert((UPtr)data%sizeof(Reg)==0);
		memset(data, 0, (count+7)/8);
	}

	static auto size_required(UPtr count) -> UPtr {
		return count/8 /8*8 /sizeof(Reg)*sizeof(Reg);
	}

	auto claim() -> Try<UPtr> {
		if(earliestFree>=count) return {"No allocation pages remain"}; // none available

		// // are we at the end of the list? just return that
		// if(earliestFree==afterLastTaken){
		// 	((U8*)data)[earliestFree/8] |= 1<<(earliestFree%8);
		// 	afterLastTaken++;
		// 	return {earliestFree++};
		// }

		// be fast and allocate sequentially first, if we've not reached the end
		if(afterLastTaken<count){
			((U8*)data)[afterLastTaken/8] |= 1<<(afterLastTaken%8);
			return {afterLastTaken++};
		}

		/// ..otherwise, begin a search..

		// search 1 byte at a time until we're Reg aligned
		for(;earliestFree<count&&earliestFree%(8*sizeof(Reg)); earliestFree+=8){
			if(((U8*)data)[earliestFree/8]==0xff) continue;

			auto bit = __builtin_ctz(~((U8*)data)[earliestFree/8]); // (__builtin_ctz gets us the first set bit, so we feed it the ~ inverse to find the first unset)
			((U8*)data)[earliestFree/8] |= 1<<bit;
			earliestFree = ((earliestFree&~0xff)|bit)+1;
			afterLastTaken = max(afterLastTaken, earliestFree);
			return {earliestFree-1};
		}

		// search Reg bytes at a time, aborting early if a match is found
		for(;earliestFree+8*sizeof(Reg)<=count; earliestFree+=8*sizeof(Reg)){
			if(data[earliestFree/8/sizeof(Reg)]!=~0u) break;
		}

		// search 1 byte at a time
		for(;earliestFree<count; earliestFree+=8){
			if(((U8*)data)[earliestFree/8]==0xff) continue;

			auto bit = __builtin_ctz(~((U8*)data)[earliestFree/8]); // (__builtin_ctz gets us the first set bit, so we feed it the ~ inverse to find the first unset)
			((U8*)data)[earliestFree/8] |= 1<<bit;
			earliestFree = ((earliestFree&~0xff)|bit)+1;
			afterLastTaken = max(afterLastTaken, earliestFree);
			return {earliestFree-1};
		}

		//nothing found
		afterLastTaken = earliestFree; // both now equal to count
		return {"No allocation pages remain"};
	}

	void release(UPtr index) {
		((U8*)data)[index/8] &= ~(1<<index);

		earliestFree = min(index, earliestFree);

		// have we just removed the last taken position?
		if(index==afterLastTaken-1){
			afterLastTaken--;

			//continue rolling back one bit at a time until we're at the last free bit again
			//TODO: optimise this to byte and Reg steps when doing big rollbacks
			while(afterLastTaken>earliestFree && ((U8*)data)[(afterLastTaken-1)/8] & 1<<((afterLastTaken-1)%8) == 0){
				afterLastTaken--;
			}
		}
	}

	Reg *data;
	UPtr count; // size in bits
	UPtr earliestFree = 0; // there are guarenteed to be no unset bits before this position
	UPtr afterLastTaken = 0; // there are guarenteed to be no set bits from this position onwards
};
