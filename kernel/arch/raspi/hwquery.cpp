#include "hwquery.hpp"

#include <kernel/arch/raspi/mailbox.hpp>
#include <kernel/info.hpp>
#include <kernel/log.hpp>
#include <kernel/memory.hpp>

namespace arch {
	namespace raspi {
		namespace hwquery {
			namespace {
				struct {
					U32 revisionRaw;
					MachineModel machineModel;
					U32 revisionMajor;
					U32 revisionMinor;
					U64 ram;
				} oldMachineModels[] = {
					{0x02, MachineModel::modelBRelease1MB256, 1, 0, 256*1024*1024},
					{0x03, MachineModel::modelBRelease1MB256, 1, 0, 256*1024*1024},
					{0x04, MachineModel::modelBRelease2MB256, 2, 0, 256*1024*1024},
					{0x05, MachineModel::modelBRelease2MB256, 2, 0, 256*1024*1024},
					{0x06, MachineModel::modelBRelease2MB256, 2, 0, 256*1024*1024},
					{0x07, MachineModel::modelA,              2, 0, 256*1024*1024},
					{0x08, MachineModel::modelA,              2, 0, 256*1024*1024},
					{0x09, MachineModel::modelA,              2, 0, 256*1024*1024},
					{0x0d, MachineModel::modelBRelease2MB512, 2, 0, 512*1024*1024},
					{0x0e, MachineModel::modelBRelease2MB512, 2, 0, 512*1024*1024},
					{0x0f, MachineModel::modelBRelease2MB512, 2, 0, 512*1024*1024},
					{0x10, MachineModel::modelBPlus,          1, 0, 512*1024*1024},
					{0x11, MachineModel::modelCM,             1, 0, 512*1024*1024},
					{0x12, MachineModel::modelAPlus,          1, 1, 256*1024*1024},
					{0x13, MachineModel::modelBPlus,          1, 2, 512*1024*1024},
					{0x14, MachineModel::modelCM,             1, 0, 512*1024*1024},
					{0x15, MachineModel::modelAPlus,          1, 1, 256*1024*1024}
				};

				struct {
					U32 type;
					MachineModel machineModel;
					U32 modelMajor;
					// U32 revisionMajor;
					// U32 revisionMinor;
				} newMachineModels[] = {
					{0x00, MachineModel::modelA,              1},
					{0x01, MachineModel::modelBRelease2MB512, 1}, // OR modelBRelease1MB256 or modelBRelease2MB256
					{0x02, MachineModel::modelAPlus,          1},
					{0x03, MachineModel::modelBPlus,          1},
					{0x04, MachineModel::model2B,             2},
					{0x06, MachineModel::modelCM,             1},
					{0x08, MachineModel::model3B,             3},
					{0x09, MachineModel::modelZero,           1},
					{0x0a, MachineModel::modelCM3,            3},
					{0x0c, MachineModel::modelZeroW,          1},
					{0x0d, MachineModel::model3BPlus,         3},
					{0x0e, MachineModel::model3APlus,         3},
					{0x10, MachineModel::modelCM3Plus,        3},
					{0x11, MachineModel::model4B,             4}
				};
			}

			MachineModel machineModel = MachineModel::unknown;
			U32 boardRevision = 0;
			U32 modelMajor = 0;
			U32 revisionMajor = 0;
			U32 revisionMinor = 0;
			Soc soc = Soc::unknown;
			void* videoMemoryStart = nullptr;
			U64 videoMemory = 0;

			void init() {
				log::Section section("arch::raspi::hwquery::init...");

				mailbox::PropertyMessage tags[4];
				tags[0].tag = mailbox::PropertyTag::get_board_revision;
				tags[0].data.boardRevision = 0;
				tags[1].tag = mailbox::PropertyTag::get_arm_memory;
				tags[2].tag = mailbox::PropertyTag::get_vc_memory;
				tags[3].tag = mailbox::PropertyTag::null_tag;

				if(!mailbox::send_messages(tags)){
					log::print_error("Error: unable to query properties");

				}else{
					boardRevision = tags[0].data.boardRevision;
					U32 revisionRaw = boardRevision;
					U64 totalMemory = 0;

					if(revisionRaw&(1<<23)){
						//new scheme
						const U8 type = (revisionRaw>>4)&0xff;
						for(auto &model:newMachineModels){
							if(model.type==type){
								machineModel = model.machineModel;
								modelMajor = model.modelMajor;
								revisionMajor = 1;
								revisionMinor = (revisionRaw&0xf)+1;
								break;
							}
						}

						U32 socId = (boardRevision&0xf000)>>12;
						totalMemory = ((U64)256 << ((boardRevision&0x700000)>>20))*1024*1024;

						if(socId+1<=(U32)Soc::max){
							soc = Soc(socId+1);
						}

						if(machineModel==MachineModel::modelBRelease2MB512&&totalMemory==256*1024*1024){
							machineModel = revisionMinor==1?MachineModel::modelBRelease1MB256:MachineModel::modelBRelease2MB256;
						}

					}else{
						//old scheme
						for(auto &model:oldMachineModels){
							if(model.revisionRaw==revisionRaw){
								machineModel = model.machineModel;
								modelMajor = 1;
								revisionMajor = model.revisionMajor;
								revisionMinor = model.revisionMinor;
								soc = Soc::bcm2835;
								totalMemory = model.ram;
								break;
							}
						}
					}

					videoMemoryStart = (void*)(U64)tags[2].data.memory.address;
					videoMemory = tags[2].data.memory.size;

					// log::print_info("board revision: ", boardRevision);
					log::print_info("board: ", &to_string_hex_trim(boardRevision)[2]);
					log::print_info("device: ", machineModel_name[(U32)machineModel]);
					log::print_info("model: ", modelMajor);
					log::print_info("revision: ", revisionMajor, '.', revisionMinor);
					log::print_info("soc: ", soc_name[(U32)soc]);
					log::print_info("ram: ", totalMemory/1024/1024, "MB");
					log::print_info("vram: ", videoMemory/1024/1024, "MB");
					memory::totalMemory = totalMemory;

					static char revision_buffer[256] = "";
					strcat(revision_buffer, to_string(revisionMajor));
					strcat(revision_buffer, ".");
					strcat(revision_buffer, to_string(revisionMinor));

					info::device_model = machineModel_name[(U32)machineModel];
					info::device_revision = revision_buffer;
				}
			}
		}
	}
}
