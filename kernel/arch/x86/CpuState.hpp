#pragma once

#include <common/types.hpp>

namespace arch {
	namespace x86 {
		struct __attribute__((packed)) CpuState {
			struct __attribute__((packed)) SegmentRegisters {
				U16 gs;
				U16 :16;
				U16 fs;
				U16 :16;
				U16 es;
				U16 :16;
				U16 ds;
				U16 :16;
			} segments;

			struct __attribute__((packed)) GeneralRegisters {
				U32 edi; // destination index
				U32 esi; // source index
				U32 ebp; // base pointer
				U32 esp; // stack pointer
				U32 ebx; // base
				U32 edx; // data
				U32 ecx; // counter
				U32 eax; // accumulator
			} registers;

			U32 interrupt; // fired interrupt

			struct __attribute__((packed)) InterruptFrame {
				U32 error;     // error code
				U32 eip;       // return address
				U16 cs;        // return code segment
			} interruptFrame;
			U16 :16;

			union Eflags {
				U32 data;

				struct __attribute__((packed)) {
					U8 cf:1;        // carry
					U8 _reserved:1; // always 1
					U8 pf:1;        // parity
					U8 :1;
					U8 af:1;        // auxiliary carry
					U8 :1;
					U8 zf:1;        // zero
					U8 sf:1;        // sign
					U8 tf:1;        // trap
					U8 _if:1;       // interrupt
					U8 df:1;        // direction
					U8 of:1;        // overflow
					U8 iopl:2;      // IO privilege level
					U8 nt:1;        // nested task
					U8 :1;
					U8 rf:1;        // resume
					U8 vm:1;        // virtual 8086 mode
					U8 ac:1;        // alignment check
					U8 vif:1;       // virtual interrupt
					U8 vip:1;       // virtual interrupt pending
					U8 id:1;        // ID
				} bit;
			} eflags;

			// only present from userspace
			struct __attribute__((packed)) {
				U32 esp;  // stack pointer
				U32 ss;   // stack segment
			} userspace;

			// only present in vm86
			struct __attribute__((packed)) {
				U16 es;
				U16 :16;
				U16 ds;
				U16 :16;
				U16 fs;
				U16 :16;
				U16 gs;
				U16 :16;
			} vm86;
		};
	}
}
