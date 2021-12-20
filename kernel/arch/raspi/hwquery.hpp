#pragma once

#include <common/types.hpp>

namespace hwquery {
	namespace arch {
		namespace raspi {
			enum struct MachineModel: U32 {
				unknown,
				modelA,
				modelBRelease1MB256,
				modelBRelease2MB256,
				modelBRelease2MB512,
				modelAPlus,
				modelBPlus,
				modelZero,
				modelZeroW,
				model2B,
				model3B,
				model3APlus,
				model3BPlus,
				modelCM,
				modelCM3,
				modelCM3Plus,
				model4B,
				max = model4B
			};
			inline const char *machineModel_name[U32(MachineModel::max)+1] = {
				"unknown",
				"A",
				"B Release 1 256MB",
				"B Release 2 256MB",
				"B Release 2 512MB",
				"A+",
				"B+",
				"Zero",
				"Zero W",
				"2 Model B",
				"3 Model B",
				"3 Model A+",
				"3 Model B+",
				"Compute Module 1",
				"Compute Module 3",
				"Compute Module 3+",
				"4 Model B",
			};

			enum struct Soc: U32 {
				unknown,
				bcm2835,
				bcm2836,
				bcm2837,
				bcm2711,
				max = bcm2711
			};
			inline const char *soc_name[U32(Soc::max)+1] = {
				"unknown",
				"BCM2835",
				"BCM2836",
				"BCM2837",
				"BCM2711"
			};

			extern MachineModel machineModel;
			extern Soc soc;

			void init();
		}
	}
}
