#include "acpi.hpp"

#include <kernel/arch/x86-ibm-bios/bios.hpp>
#include <kernel/Log.hpp>

static Log log("arch::x86::acpi");

namespace arch {
	namespace x86 {
		namespace acpi {
			namespace {
				const U8 rsdp_descriptor_signature[8] = {'R','S','D',' ','P','T','R',' '};

				struct __attribute__((packed)) RsdpDescriptor {
					char signature[8];
					U8 checksum; // should be set to ensure all bytes in this struct checksum to (U8)0
					char oemId[6];
					U8 revision; // 0 == 1.0, 2 == 2.0 .. 6.1
					U32 rsdtAddress; //deprecated in v2
				};

				struct __attribute__((packed)) RsdpDescriptor_v2 {
					RsdpDescriptor descriptor;
					U32 length;
					U64 xsdtAddress;
					U8 extendedChecksum; // should be set to ensure all bytes in this struct checksum to (U8)0
					U8 _reserved[3];
				};

				struct __attribute__((packed)) Rsdt: TableHeader {
					U32 sdts[];

					auto get_sdt_count() -> unsigned {
						return (length - sizeof(TableHeader)) / sizeof(sdts[0]);
					}
					auto get_sdt(unsigned i) -> Sdt* {
						return (Sdt*)(size_t)sdts[i];
					}
				};

				struct __attribute__((packed)) Xsdt: TableHeader {
					U64 sdts[];

					auto get_sdt_count() -> unsigned {
						return (length - sizeof(TableHeader)) / sizeof(sdts[0]);
					}
					auto get_sdt(unsigned i) -> Sdt* {
						return (Sdt*)(size_t)sdts[i];
					}
				};

				auto find_rsdp_in_memory(void *start, size_t length) -> RsdpDescriptor* {
					// check every 16 byte position (its alignment)
					for(auto possible=(RsdpDescriptor*)start;;possible = (RsdpDescriptor*)((U8*)possible+16), length-=16) {
						if(*(U64*)possible->signature==*(U64*)rsdp_descriptor_signature){
							return possible;
						}
						if(length<16) break;
					}

					return nullptr;
				}

				#ifdef ARCH_X86_IBM_BIOS
					auto find_rsdp_in_bios() -> RsdpDescriptor* {
						return find_rsdp_in_memory(x86_ibm_bios::bios::get_possible_ebda(), 0x400);
					}
				#endif

				auto validate_rsdp(RsdpDescriptor &rsdp) -> bool {
					U8 checksum = 0;
					for(auto byte = (U8*)&rsdp; byte < (U8*)(&rsdp+1); byte++) {
						checksum += *byte;
					}

					if(checksum!=0) {
						// invalid checksum
						return false;
					}

					// version 2+?
					if(rsdp.revision>=2) {
						auto &rsdp2 = *(RsdpDescriptor_v2*)&rsdp;

						U8 extendedChecksum = 0;
						for(auto byte = (U8*)(&rsdp+1); byte < (U8*)(&rsdp2+1); byte++) {
							extendedChecksum += *byte;
						}

						if(extendedChecksum!=0) {
							// invalid checksum
							return false;
						}
					}

					return true;
				}

				auto find_rsdp() -> RsdpDescriptor* {
					RsdpDescriptor *rsdp = nullptr;

					#ifdef ARCH_X86_IBM_BIOS
						// does the bios tell us where it is?
						if(!rsdp) find_rsdp_in_bios();
					#endif

					// no? try just below 1MB
					if(!rsdp) rsdp = find_rsdp_in_memory((U8*)0xe0000, 0xfffff-0xe0000);

					if(!rsdp) return nullptr;

					if(!validate_rsdp(*rsdp)) return nullptr;

					return rsdp;
				}
			}

			RsdpDescriptor *rsdp = nullptr;

			void *rootTableLocation = nullptr;
			TableHeader *rootTableHeader = nullptr;
			Rsdt *rsdt = nullptr;
			Xsdt *xsdt = nullptr;

			auto get_entry_count() -> unsigned {
				if(xsdt){
					return xsdt->get_sdt_count();
				}else if(rsdt){
					return rsdt->get_sdt_count();
				}else{
					return 0;
				}
			}

			auto get_entry(unsigned i) -> Sdt* {
				if(xsdt){
					return xsdt->get_sdt(i);
				}else if(rsdt){
					return rsdt->get_sdt(i);
				}else{
					return nullptr;
				}
			}

			auto find_entry_with_signature(char signature[4]) -> Sdt* {
				for(auto i=0u, count=get_entry_count();i<count;i++){
					auto entry = get_entry(i);
					if(entry&&*(U64*)entry->signature==*(U64*)entry){
						return entry;
					}
				}

				return nullptr;
			}

			void init() {
				auto section = log.section("init...");

				rsdp = find_rsdp();
				if(!rsdp){
					log.print_error("No valid RSDP found");
					return;
				}

				if(rsdp->revision==0){
					log.print_info("ACPI version: 1.0");
				}else if(rsdp->revision==2){
					log.print_info("ACPI version: 2.0+ (revision ", rsdp->revision, ')');
				}else{
					log.print_info("ACPI version: Unrecognised (revision ", rsdp->revision, ')');
				}

				log.print_info("OEM: ", rsdp->oemId[0], rsdp->oemId[1], rsdp->oemId[2], rsdp->oemId[3], rsdp->oemId[4], rsdp->oemId[5]);

				if(rsdp->revision>=2){
					rootTableHeader = xsdt = (Xsdt*)((RsdpDescriptor_v2*)rsdp)->xsdtAddress;
				}
				// // if(xsdt&&xsdt>=(void*)0xffffffff){
				// // 	log.print_warning("XSDT not 32bit, falling back to RSDT...");
				// // 	xsdt = nullptr;
				// // }

				if(!rootTableHeader){
					rootTableHeader = rsdt = (Rsdt*)rsdp->rsdtAddress;
				}
				// if(rsdt){
				// 	log.print_info("RSDT acquired");
				// }

				if(!rootTableHeader){
					log.print_error("ACPI descriptor table not found (no valid XSDT or RSDT found)");
					return;
				}

				{
					U8 checksum = 0;
					for(U8 *byte=(U8*)rootTableHeader;byte<(U8*)(rootTableHeader)+rootTableHeader->length;byte++){
						checksum += *byte;
					}

					if(checksum!=0){
						log.print_error("ACPI descriptor table was not valid ");
						rootTableHeader = nullptr;
						xsdt = nullptr;
						rsdt = nullptr;
						return;
					}
				}

				log.print_info("OEM Table: ",
					rootTableHeader->oemTableId[0], rootTableHeader->oemTableId[1], rootTableHeader->oemTableId[2], rootTableHeader->oemTableId[3], rootTableHeader->oemTableId[4], rootTableHeader->oemTableId[5], rootTableHeader->oemTableId[6], rootTableHeader->oemTableId[7],
					" (", rootTableHeader->oemId[0], rootTableHeader->oemId[1], rootTableHeader->oemId[2], rootTableHeader->oemId[3], rootTableHeader->oemId[4], rootTableHeader->oemId[5], ')'
				);

				for(auto i=0u;i<get_entry_count();i++){
					auto entry = get_entry(i);
					log.print_info("  ",
						entry->signature[0], entry->signature[1], entry->signature[2], entry->signature[3],
						" (", entry->oemId[0], entry->oemId[1], entry->oemId[2], entry->oemId[3], entry->oemId[4], entry->oemId[5],')'
					);
					// log.print_info("Entry: ", entry);
					// log.print_info("  Signature: ", entry->signature[0], entry->signature[1], entry->signature[2], entry->signature[3]);
					// log.print_info("  OEM: ", entry->oemId[0], entry->oemId[1], entry->oemId[2], entry->oemId[3], entry->oemId[4], entry->oemId[5]);
					// log.print_info("  OEM Table: ", entry->oemTableId[0], entry->oemTableId[1], entry->oemTableId[2], entry->oemTableId[3], entry->oemTableId[4], entry->oemTableId[5], entry->oemTableId[6], entry->oemTableId[7]);
					// log.print_info("  Creator: ", entry->creatorId[0], entry->creatorId[1], entry->creatorId[2], entry->creatorId[3]);
				}
			}
		}
	}
}
