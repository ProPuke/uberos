#include "arm64.hpp"

#include <common/stdlib.hpp>

char *insert(char *dest, const char *src) {
	while(*src) *dest++ = *src++;
	return dest;
}

namespace disassemble {
	namespace arm64 {
		Instruction read(U32 data, U64 pc) {
			Instruction instruction;

			unsigned opcode = bits(data, 25, 28);

			instruction.opcode = opcode;

			if(opcode==0b0000){
				//reserved

				// unsigned op0 = bits(data, 29, 31);
				unsigned op1 = bits(data, 16, 24);
				if(op1!=0){
					//invalid
					return instruction;
				}

				unsigned imm = bits(data, 0, 15);

				instruction.group = InstructionGroup::reserved;
				instruction.type = InstructionType::udf;
				instruction.imm = imm;

			}else if(opcode==0b0001){
				//unallocated
				
			}else if(opcode==0b0010){
				//SVE

			}else if(opcode==0b0011){
				//unallocated

			}else if((opcode&0b1110)==0b1000){
				//data processing
				instruction.group = InstructionGroup::dataProcessing;

				unsigned op0 = bits(data, 23, 25);
				if(op0>>1==0){
					unsigned op    = bits(data, 31, 31);
					unsigned immlo = bits(data, 29, 30);
					unsigned immhi = bits(data, 5, 23);
					unsigned rd    = bits(data, 0, 4);

					if(rd>=32){
						//not a valid 64bit register
						return instruction;
					}

					U64 imm;

					if(op==0){
						instruction.type = InstructionType::adr;
						imm = (immhi << 2) | immlo;
						imm = sign_extend(imm, 21);
						imm += pc;

					}else{
						instruction.type = InstructionType::adrp;
						imm = ((immhi << 2) | immlo) << 12;
						imm = sign_extend(imm, 32);
						imm += pc & ~0xfff;
					}

					instruction.rd = rd;
					instruction.imm = imm;
					//TODO

				}else if(op0==2){
					// unsigned sf    = bits(data, 31, 31);
					// unsigned op    = bits(data, 30, 30);
					// unsigned S     = bits(data, 29, 29);
					// unsigned sh    = bits(data, 22, 22);
					// unsigned imm12 = bits(data, 10, 21);
					// unsigned Rn    = bits(data,  5,  9);
					// unsigned Rd    = bits(data,  0,  4);
					//TODO DisassembleAddSubtractImmediateInstr

				}else if(op0==3){
					// unsigned sf    = bits(data, 31, 31);
					// unsigned op    = bits(data, 30, 30);
					// unsigned S     = bits(data, 29, 29);
					// unsigned o2    = bits(data, 22, 22);
					// unsigned uimm6 = bits(data, 16, 21);
					// unsigned op3   = bits(data, 14, 15);
					// unsigned uimm4 = bits(data, 10, 13);
					// unsigned Rn    = bits(data,  5,  9);
					// unsigned Rd    = bits(data,  0,  4);
					//TODO DisassembleAddSubtractImmediateWithTagsInstr

				}else if(op0==4){
					// unsigned sf   = bits(data, 31, 31);
					// unsigned opc  = bits(data, 29, 30);
					// unsigned N    = bits(data, 22, 22);
					// unsigned immr = bits(data, 16, 21);
					// unsigned imms = bits(data, 10, 15);
					// unsigned Rn   = bits(data,  5,  9);
					// unsigned Rd   = bits(data,  0,  4);
					//TODO DisassembleLogicalImmediateInstr

				}else if(op0==5){
					// unsigned sf    = bits(data, 31, 31);
					// unsigned opc   = bits(data, 29, 30);
					// unsigned hw    = bits(data, 21, 22);
					// unsigned imm16 = bits(data,  5, 20);
					// unsigned Rd    = bits(data,  0,  4);
					//TODO DisassembleMoveWideImmediateInstr

				}else if(op0==6){
					// unsigned sf   = bits(data, 31, 31);
					// unsigned opc  = bits(data, 29, 30);
					// unsigned N    = bits(data, 22, 22);
					// unsigned immr = bits(data, 16, 21);
					// unsigned imms = bits(data, 10, 15);
					// unsigned Rn   = bits(data,  5,  9);
					// unsigned Rd   = bits(data,  0,  4);
					//TODO DisassembleBitfieldInstr

				}else if(op0==7){
					// unsigned sf   = bits(data, 31, 31);
					// unsigned op21 = bits(data, 29, 30);
					// unsigned N    = bits(data, 22, 22);
					// unsigned o0   = bits(data, 21, 21);
					// unsigned Rm   = bits(data, 16, 20);
					// unsigned imms = bits(data, 10, 15);
					// unsigned Rn   = bits(data,  5,  9);
					// unsigned Rd   = bits(data,  0,  4);
					//TODO DisassembleExtractInstr
					
				}
				
			}else if((opcode&0b1110)==0b1010){
				//branchAndSystem
				instruction.group = InstructionGroup::branchAndSystem;

			}else if((opcode&0b0101)==0b0100){
				instruction.group = InstructionGroup::loadsStores;

				unsigned op0 = bits(data, 28, 31);
				unsigned op1 = bits(data, 26, 26);
				unsigned op2 = bits(data, 23, 24);
				unsigned op3 = bits(data, 16, 21);
				unsigned op4 = bits(data, 10, 11);

				if((op0 & ~4) == 0 && op1 == 1 && (op2 == 0 || op2 == 1) && (op3 & ~0x1f) == 0){
    				// unsigned q      = bits(data, 30, 30);
    				unsigned l      = bits(data, 22, 22);
    				// unsigned rm     = bits(data, 16, 20);
    				// unsigned opcode = bits(data, 12, 15);
    				// unsigned size   = bits(data, 10, 11);
    				// unsigned rn     = bits(data,  5,  9);
    				// unsigned rt     = bits(data,  0,  4);

					unsigned regcnt;
					unsigned selem;

					switch(data){
						case 0x0: regcnt = 4; selem = 4; break;
						case 0x2: regcnt = 4; selem = 1; break;
						case 0x4: regcnt = 3; selem = 3; break;
						case 0x6: regcnt = 3; selem = 1; break;
						case 0x7: regcnt = 1; selem = 1; break;
						case 0x8: regcnt = 2; selem = 2; break;
						case 0xa: regcnt = 2; selem = 1; break;
						default: return instruction;
					};

					(void)regcnt;

					if(l==0){
						instruction.type = InstructionType((U32)InstructionType::st1 - 1 + selem);
					}else{
						instruction.type = InstructionType((U32)InstructionType::ld1 - 1 + (selem*2 - 1));
					}

					//TODO DisassembleLoadStoreMultStructuresInstr

				}else if((op0 & ~4) == 0 && op1 == 1 && (op2 == 2 || op2 == 3)){
    				unsigned q      = bits(data, 30, 30);
    				unsigned l      = bits(data, 22, 22);
    				unsigned r      = bits(data, 21, 21);
    				// unsigned rm     = bits(data, 16, 20);
    				unsigned opcode = bits(data, 13, 15);
    				unsigned s      = bits(data, 12, 12);
    				unsigned size   = bits(data, 10, 11);
    				// unsigned rn     = bits(data,  5,  9);
    				// unsigned rt     = bits(data,  0,  4);

    				unsigned scale = opcode >> 1;
    				unsigned selem = (((opcode & 1) << 1) | r) + 1;
    				unsigned index = 0;

					bool replicate = false;

					switch(scale){
						case 3:
							replicate = true;
						break;
						case 0:
							index = (q << 3) | (s << 2) | size;
						break;
						case 1:
							index = (q << 2) | (s << 1) | (size >> 1);
						break;
						case 2:
							if((size & 1) == 0){
								index = (q << 1) | s;

							}else{
								index = q;
							}

						break;
						default:
							return instruction;
					};

					(void)index;

					if(replicate){
						instruction.type = InstructionType(((U32)InstructionType::ld1r - 1) + ((selem * 2) - 1));

					}else if(!l){
						instruction.type = InstructionType(((U32)InstructionType::st1 - 1) + selem);

					}else{
						instruction.type = InstructionType(((U32)InstructionType::ld1 - 1) + ((selem * 2) - 1));
					}

					//TODO DisassembleLoadStoreSingleStructuresInstr(i, out, op2 != 2)

				}else if(op0 == 13 && op1 == 0 && (op2 >> 1) == 1 && (op3 >> 5) == 1){
					//TODO DisassembleLoadStoreMemoryTagsInstr(i, out)

				}else if((op0 & ~12) == 0 && op1 == 0 && (op2 >> 1) == 0){
					//TODO DisassembleLoadAndStoreExclusiveInstr(i, out)

				}else if((op0 & ~12) == 1 && op1 == 0 && (op2 >> 1) == 1 && (op3 & ~0x1f) == 0 && op4 == 0){
					//TODO DisassembleLDAPR_STLRInstr(i, out)

				}else if((op0 & ~12) == 1 && (op2 >> 1) == 0){
					//TODO DisassembleLoadAndStoreLiteralInstr(i, out)

				}else if((op0 & ~12) == 2 && op2 < 4){
					//TODO DisassembleLoadAndStoreRegisterPairInstr(i, out, op2)

				}else if((op0 & ~12) == 3 && (op2 >> 1) == 0){
					if((op3 & ~0x1f) == 0){
						//TODO DisassembleLoadAndStoreRegisterInstr(i, out, op4)

					}else{
						if(op4 == 0){
							//TODO DisassembleAtomicMemoryInstr(i, out)

						}else if(op4 == 2){
							//TODO DisassembleLoadAndStoreRegisterOffsetInstr(i, out)

						}else if((op4 & 1) == 1){
							//TODO DisassembleLoadAndStorePACInstr(i, out)

						}else{
							//invalid
							return instruction;
						}
					}

				}else if((op0 & ~12) == 3 && (op2 >> 1) == 1){
					//TODO DisassembleLoadAndStoreRegisterInstr(i, out, UNSIGNED_IMMEDIATE)

				}else{
					//invalid
					return instruction;
				}

			}else if((opcode&0b0111)==0b0101){
				//data processing register
				instruction.group = InstructionGroup::registers;

			}else if((opcode&0b0111)==0b0111){
				//data processing float
				instruction.group = InstructionGroup::floatingPointing;

			}else{
				//unknown
			}

			return instruction;
		}

		const char *to_string(U32 data, U64 pc) {
			auto instruction = read(data, pc);
			return to_string(instruction);
		}

		const char *to_string(Instruction instruction) {
			static char line[256];
			line[0] = '\0';

			char *c = line;
			switch(instruction.group){
				case InstructionGroup::unknown:
					c = insert(c, "  ??? ");
					c = insert(c, "(");
					c = insert(c, ::to_string(instruction.opcode));
					c = insert(c, ")");
				break;
				case InstructionGroup::reserved:
					c = insert(c, "  RSV ");
				break;
				case InstructionGroup::dataProcessing:
					c = insert(c, " DATA ");
				break;
				case InstructionGroup::branchAndSystem:
					c = insert(c, "LOGIC ");
				break;
				case InstructionGroup::loadsStores:
					c = insert(c, "  MEM ");
				break;
				case InstructionGroup::registers:
					c = insert(c, "  REG ");
				break;
				case InstructionGroup::floatingPointing:
					c = insert(c, "  FLT ");
				break;
			}

			if(instruction.group!=InstructionGroup::unknown){
				c = insert(c, InstructionType_name[(unsigned)instruction.type]);

				auto args = false;

				if(instruction.rd!=(U8)~0){
					if(args){
						c = insert(c, ", ");
					}
					c = insert(c, " x");
					c = insert(c, ::to_string((unsigned)instruction.rd));
					args = true;
				}

				if(instruction.imm){
					if(args){
						c = insert(c, ", ");
					}
					c = insert(c, " #");
					c = insert(c, to_string_hex_trim(instruction.imm)+2);
					args = true;
				}
			}

			*c = '\0';

			return line;
		}
	}
}
