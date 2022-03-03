#pragma once

#define SCTLR_RESERVED                  (3 << 28) | (3 << 22) | (1 << 20)
#define SCTLR_EE_LITTLE_ENDIAN          (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN         (0 << 24)
#define SCTLR_INSTRUCTION_CACHE         (1 << 12)
#define SCTLR_EXCEPT_START_CONTEXT_SYNC (1 << 22)
#define SCTLR_EXCEPT_END_CONTEXT_SYNC   (1 << 11)
#define SCTLR_SP_ALIGN_CHECK_0          (1 <<  4)
#define SCTLR_SP_ALIGN_CHECK            (1 <<  3)
#define SCTLR_DATA_CACHE                (1 <<  2)
#define SCTLR_ALIGN_CHECK               (1 <<  1)
#define SCTLR_MMU                       (1 <<  0)

#define HCR_RW                  (1 << 31)

#define SCR_RESERVED            (3 << 4)
#define SCR_RW                  (1 << 10)
#define SCR_NS                  (1 << 0)

#define SPSR_MASK_ALL           (7 << 6)
#define SPSR_EL1h               (5 << 0)

#define CPACR_FPEN_TRAP_0       (1 << 20)
#define CPACR_FPEN_TRAP_1       (1 << 21)
#define CPACR_FPEN_TRAP_OFF     (1 << 21) | (1 << 20)
