#pragma once

#include <common/types.hpp>

struct __attribute__((packed)) Tcr {
	U64 t0sz:6;
	U64 _reserved1:1;
	U64 epd0:1;
	U64 irgn0:2;
	U64 orgn0:2;
	U64 sh0:2;
	U64 tg0:2;
	U64 t1sz:6;
	U64 a1:1;
	U64 epd1:1;
	U64 irgn1:2;
	U64 orgn1:2;
	U64 sh1:2;
	U64 tg1:2;
	U64 ips:3;
	U64 _reserved2:1;
	U64 as:1;
	U64 tbi0:1;
	U64 tbi1:1;
	U64 ha:1;
	U64 hd:1;
	U64 hpd0:1;
	U64 hpd1:1;
	U64 hwu059:1;
	U64 hwu060:1;
	U64 hwu061:1;
	U64 hwu062:1;
	U64 hwu159:1;
	U64 hwu160:1;
	U64 hwu161:1;
	U64 hwu162:1;
	U64 tbid0:1;
	U64 tbid1:1;
	U64 nfd0:1;
	U64 nfd1:1;
	U64 e0pd0:1;
	U64 e0pd1:1;
	U64 tcma1:1;
	U64 ds:1;
	U64 _reserved3:4;

	void load_el1() {
		asm volatile("mrs %0, tcr_el1" : "=r" (*reinterpret_cast<U64*>(this)));
	}

	void save_el1() {
		asm volatile("dsb sy" ::: "memory");
		asm volatile("msr tcr_el1, %0" : : "r" (*reinterpret_cast<U64*>(this)));
		asm volatile("isb");
	}
};
static_assert(sizeof(Tcr)==8);

struct __attribute__((packed)) Sctlr {
	U64 mmuEnable:1;
	U64 alignmentChecking:1;
	U64 cacheEnable:2;
	U64 sa:3;
	U64 sa0:1;
	U64 cp15ben:1;
	U64 naa:1;
	U64 itd:1;
	U64 sed:1;
	U64 uma:1;
	U64 enrctx:1;
	U64 eos:1;
	U64 i:1;
	U64 endb:1;
	U64 dze:1;
	U64 uct:1;
	U64 ntwi:1;
	U64 _reserved1:1;
	U64 ntwe:1;
	U64 wxn:1;
	U64 tscxt:1;
	U64 iesb:1;
	U64 eis:1;
	U64 span:1;
	U64 e0e:1;
	U64 exceptionBigEndian:1;
	U64 uci:1;
	U64 enda:1;
	U64 ntlsmd:1;
	U64 lsmaoe:1;
	U64 enib:1;
	U64 enia:1;
	U64 _reserved2:3;
	U64 bt0:1;
	U64 bt1:1;
	U64 itfsb:1;
	U64 tcf0:2;
	U64 tcf:2;
	U64 ata0:1;
	U64 ata:1;
	U64 dssbs:1;
	U64 tweden:1;
	U64 _reserved3:4;
	U64 enasr:1;
	U64 enas0:1;
	U64 enals:1;
	U64 epan:1;
	U64 _reserved4:6;

	void load_el1() {
		asm volatile("mrs %0, sctlr_el1" : "=r" (*reinterpret_cast<U64*>(this)));
	}

	void save_el1() {
		asm volatile("dsb sy" ::: "memory");
		asm volatile("msr sctlr_el1, %0" : : "r" (*reinterpret_cast<U64*>(this)));
		asm volatile("isb");
	}
};
static_assert(sizeof(Sctlr)==8);

struct __attribute__((packed)) Ttbr {
	U64 commonNotPrivate:1;
	U64 tableBaseAddress:47;
	U64 asid:16;

	void load_br0el1() {
		asm volatile("mrs %0, ttbr0_el1" : "=r" (*reinterpret_cast<U64*>(this)));
	}
	void load_br0el2() {
		asm volatile("mrs %0, ttbr0_el2" : "=r" (*reinterpret_cast<U64*>(this)));
	}
	void load_br0el3() {
		asm volatile("mrs %0, ttbr0_el3" : "=r" (*reinterpret_cast<U64*>(this)));
	}
	void load_br1el1() {
		asm volatile("mrs %0, ttbr1_el1" : "=r" (*reinterpret_cast<U64*>(this)));
	}
	void load_br1el2() {
		asm volatile("mrs %0, ttbr1_el2" : "=r" (*reinterpret_cast<U64*>(this)));
	}

	void save_br0el1() {
		asm volatile("dsb sy" ::: "memory");
		asm volatile("msr ttbr0_el1, %0" : : "r" (*reinterpret_cast<U64*>(this)) : "memory");
		asm volatile("isb");
	}
	void save_br0el2() {
		asm volatile("dsb sy" ::: "memory");
		asm volatile("msr ttbr0_el2, %0" : : "r" (*reinterpret_cast<U64*>(this)) : "memory");
		asm volatile("isb");
	}
	void save_br0el3() {
		asm volatile("dsb sy" ::: "memory");
		asm volatile("msr ttbr0_el3, %0" : : "r" (*reinterpret_cast<U64*>(this)) : "memory");
		asm volatile("isb");
	}
	void save_br1el1() {
		asm volatile("dsb sy" ::: "memory");
		asm volatile("msr ttbr1_el1, %0" : : "r" (*reinterpret_cast<U64*>(this)) : "memory");
		asm volatile("isb");
	}
	void save_br1el2() {
		asm volatile("dsb sy" ::: "memory");
		asm volatile("msr ttbr1_el2, %0" : : "r" (*reinterpret_cast<U64*>(this)) : "memory");
		asm volatile("isb");
	}
};
static_assert(sizeof(Ttbr)==8);
