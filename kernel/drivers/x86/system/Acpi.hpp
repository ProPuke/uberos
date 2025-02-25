#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/Maybe.hpp>

namespace driver::system {
	struct Acpi final: Hardware {
		DRIVER_INSTANCE(Acpi, 0xb3de6156, "acpi", "Advanced Configuration and Power Interface", Hardware)

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto can_stop_driver() -> bool override { return false; }
		auto can_restart_driver() -> bool override { return false; }

		struct __attribute__((packed)) TableHeader {
			char signature[4];
			U32 length;
			U8 revision;
			U8 checksum;
			char oemId[6];
			char oemTableId[8];
			U32 oemRevision;
			char creatorId[4];
			U32 creatorRevision;
		};

		struct __attribute__((packed)) Sdt: TableHeader {
		};

		struct GenericAddressStructure {
			U8 addressSpace;
			U8 bitWidth;
			U8 bitOffset;
			U8 accessSize;
			U64 address;
		};

		struct __attribute__((packed)) Fadt: Sdt {
			U32 firmwareCtrl;
			U32 dsdt;

			// v1.0 only. No longer in use:
			U8  _reserved1;

			U8  preferredPowerManagementProfile;
			U16 sci_interrupt;
			U32 smi_commandPort;
			U8  acpiEnable;
			U8  acpiDisable;
			U8  s4bios_rEQ;
			U8  pState_control;
			U32 pm1aEventBlock;
			U32 pm1bEventBlock;
			U32 pm1aControlBlock;
			U32 pm1bControlBlock;
			U32 pm2ControlBlock;
			U32 pmTimerBlock;
			U32 gpe0Block;
			U32 gpe1Block;
			U8  pm1EventLength;
			U8  pm1ControlLength;
			U8  pm2ControlLength;
			U8  pmtimerLength;
			U8  gpe0Length;
			U8  gpe1Length;
			U8  gpe1Base;
			U8  cStateControl;
			U16 worstC2Latency;
			U16 worstC3Latency;
			U16 flushSize;
			U16 flushStride;
			U8  dutyOffset;
			U8  dutyWidth;
			U8  dayAlarm;
			U8  monthAlarm;
			U8  century;

			// v2.0+
			union {
				struct __attribute__((packed)) {
					bool legacyDevices:1;
					bool _8042:1;
					bool vgaNotPresent:1;
					bool msiNotSupported:1;
					bool pcieAspmControls:1;
					bool cmosRtcNotPresent:1;
					U32 _reserved:10;
				};
				U16 data;
			} bootArchitectureFlags;

			U8  _reserved2;
			U32 flags;

			// 12 byte structure; see below for details
			GenericAddressStructure resetReg;

			U8 resetValue;
			U8 _reserved3[3];
		
			U64 x_firmwareControl;
			U64 x_dsdt;

			GenericAddressStructure x_pm1aEventBlock;
			GenericAddressStructure x_pm1bEventBlock;
			GenericAddressStructure x_pm1aControlBlock;
			GenericAddressStructure x_pm1bControlBlock;
			GenericAddressStructure x_pm2ControlBlock;
			GenericAddressStructure x_pmTimerBlock;
			GenericAddressStructure x_gpe0Block;
			GenericAddressStructure x_gpe1Block;
		};

		auto get_entry_count() -> unsigned;
		auto get_entry(unsigned) -> Sdt*;
		auto find_entry_with_signature(const char signature[4]) -> Sdt*;

		auto has_lpc_isa_devices() -> Maybe;
		auto has_ps2() -> Maybe;
		auto is_vga_probeable() -> Maybe;
		auto has_msi() -> Maybe;
		auto has_ospm_aspm() -> Maybe;
		auto has_cmos_rtc() -> Maybe;
	};
}
