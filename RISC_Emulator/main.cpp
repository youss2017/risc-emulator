#include <iostream>
#include <vector>

class cpu_risc32i {

public:

	cpu_risc32i() : cpu_risc32i(4096) {}

	cpu_risc32i(uint32_t memory_size) {
		memory.resize(memory_size);
		reset();
	}

	void reset(uint32_t reset_pc = 0) {
		this->pc = reset_pc;
		for (int i = 0; i < 32; i++)
			registers.REG[0] = 0;
		registers.alias.sp = memory.size() - 4;
		// clear memory
		for (auto& i : memory) i = 0;
	}

	void load_program(uint32_t offset, const std::vector<uint32_t>& bin) {
		for (uint32_t i = 0; i < bin.size(); i++)
			memory[i + offset] = bin.at(i); // throws exception if out of bounds
	}

	void cycle() {
		cycleCount++;

		// 1) Read instruction at PC
		int32_t pc = this->pc;
		uint32_t instruction = memory[pc >> 2];

		if (!instruction) {
			display_registers();
			return;
		}

		// 2) Decode instruction
		uint32_t opcode = instruction          & 0b00000000000000000000000001111111;
		uint32_t rd = instruction              & 0b00000000000000000000111110000000;
		uint32_t funct3 = instruction          & 0b00000000000000000111000000000000;
		uint32_t rs1 = instruction             & 0b00000000000011111000000000000000;
		uint32_t rs2 = instruction             & 0b00000001111100000000000000000000;
		uint32_t imm_lower_btype = instruction & 0b00000000000000000000111110000000; // Note: s and b types use the same bits
		uint32_t imm_upper_btype = instruction & 0b11111110000000000000000000000000;
		uint32_t imm_utype = instruction       & 0b11111111111111111111000000000000;
		uint32_t imm_jtype = instruction       & 0b11111111111111111111000000000000;
		uint32_t imm_itype = instruction       & 0b11111111111100000000000000000000;
		uint32_t funct7 = instruction          & 0b11111110000000000000000000000000;

		rd >>= 7;
		rs1 >>= 15;
		rs2 >>= 20;
		funct3 >>= 12;

		// 3) Execute instruction
		if (opcode == opcode::lui) {
			registers.REG[rd] = imm_utype;
		}
		else if (opcode == opcode::aupic) {
			registers.REG[rd] = imm_utype + pc;
		}
		else if (opcode == opcode::jal) {
			// rd <- pc + 4
			// pc <- pc + imm_j
			int32_t imm_j = (imm_itype & 0x80000000) ? 0b1111111111100000000000000000000 : 0; // this is the sign extension
			imm_j |= imm_jtype & 0b00000000000011111111000000000000;
			imm_j |= (imm_jtype >> 19) & 0b00000000000000000000011111111110;
			registers.REG[rd] = pc + 4;
			this->pc = (pc + imm_j) & ~1;
		}
		else if (opcode == opcode::jalr) {
			// rd <- pc + 4
			// pc <- (rs1 + imm_i) & ~1
			int32_t imm_i = (imm_itype & 0x80000000) ? 0xfffff000 : 0; // this is the sign extension
			imm_i &= imm_itype >> 20;
			registers.REG[rd] = pc + 4;
			this->pc = (pc + imm_i) & ~1;

		}
		else if (opcode == opcode::btype) {
			// pc <- pc + ( rs1 == rs2) ? imm_b : 4 )
			int32_t imm_b = (imm_upper_btype & 0x80000000) ? 0xfffff000 : 0; // this is the sign extension
			imm_b |= (imm_upper_btype >> 25) << 5;
			imm_b |= imm_lower_btype >> 7;
			imm_b &= ~((1 << 11) | (1 << 12)); // clear the 12th and 11th bit
			imm_b |= (imm_lower_btype << 4) & (1 << 11); // set 11th bit
			imm_b |= (imm_upper_btype & 0x80000000) ? (1 << 12) : 0; // set 12th bit
			imm_b &= ~1;
			switch (funct3) {
			case 0b000 /* beq  */: this->pc = pc + (int32_t(registers.REG[rs1]) == int32_t(registers.REG[rs2]) ? imm_b : 4); break;
			case 0b001 /* bne  */: this->pc = pc + (int32_t(registers.REG[rs1]) != int32_t(registers.REG[rs2]) ? imm_b : 4); break;
			case 0b100 /* blt  */: this->pc = pc + (int32_t(registers.REG[rs1]) < int32_t(registers.REG[rs2]) ? imm_b : 4); break;
			case 0b101 /* bge  */: this->pc = pc + (int32_t(registers.REG[rs1]) >= int32_t(registers.REG[rs2]) ? imm_b : 4); break;
			case 0b110 /* bltu */: this->pc = pc + (uint32_t(registers.REG[rs1]) < uint32_t(registers.REG[rs2]) ? imm_b : 4); break;
			case 0b111 /* bgeu */: this->pc = pc + (uint32_t(registers.REG[rs1]) >= uint32_t(registers.REG[rs2]) ? imm_b : 4); break;
			default:
				throw std::runtime_error("Invalid funct3 for branching.");
			}
		}
		else if (opcode == opcode::itype_mem) {
			int32_t imm_i = (imm_itype & 0x80000000) ? 0xfffff000 : 0; // this is the sign extension
			imm_i &= imm_itype >> 20;
			uint32_t read_address = registers.REG[rs1] + imm_i;
			uint32_t data = memory[read_address % memory.size()];
			switch (funct3) {
			case 0b000 /* lb  */: registers.REG[rd] = int32_t(int8_t(data)); break;
			case 0b001 /* lh  */: registers.REG[rd] = int32_t(int16_t(data)); break;
			case 0b010 /* lw  */: registers.REG[rd] = int32_t(data); break;
			case 0b100 /* lbu */: registers.REG[rd] = uint32_t(uint8_t(data)); break;
			case 0b101 /* lhu */: registers.REG[rd] = uint32_t(uint16_t(data)); break;
			default:
				throw std::runtime_error("Invalid funct3 for memory load (lb, lh, lw, lbu, lhu are only valid)");
			}
			this->pc = pc + 4;
		}
		else if (opcode == opcode::itype) {
			int32_t imm_i = (imm_itype & 0x80000000) ? 0xfffff000 : 0; // this is the sign extension
			imm_i |= imm_itype >> 20;
			switch (funct3) {
			case 0b000 /* addi */:
				// rd <- rs1 + imm_i, pc <- pc+4
				registers.REG[rd] = registers.REG[rs1] + imm_i;
				this->pc = pc + 4;
				break;
			case 0b010 /* slti */:
				// rd <- (rs1 < imm_i) ? 1: 0, pc <- pc+4
				registers.REG[rd] = int32_t(registers.REG[rs1]) < int32_t(imm_i);
				this->pc = pc + 4;
				break;
			case 0b011 /* sltiu */:
				// rd <- (rs1 < imm_i) ? 1: 0, pc <- pc+4
				registers.REG[rd] = uint32_t(registers.REG[rs1]) < uint32_t(imm_i);
				this->pc = pc + 4;
				break;
			case 0b100 /* xori */:
				// rd <- (rs1 ^ imm_i) ? 1: 0, pc <- pc+4
				registers.REG[rd] = registers.REG[rs1] ^ imm_i;
				this->pc = pc + 4;
				break;
			case 0b110 /* ori */:
				// rd <- (rs1 ^ imm_i) ? 1: 0, pc <- pc+4
				registers.REG[rd] = registers.REG[rs1] | imm_i;
				this->pc = pc + 4;
				break; break;
			case 0b111 /* andi */:
				// rd <- (rs1 ^ imm_i) ? 1: 0, pc <- pc+4
				registers.REG[rd] = registers.REG[rs1] & imm_i;
				this->pc = pc + 4;
				break; break;
			case 0b001 /* slli */:
				// (TODO): Implement this
				// rd <- (rs1 ^ imm_i) ? 1: 0, pc <- pc+4
				//registers.REG[rd] = registers.REG[rs1] ^ imm_i;
				this->pc = pc + 4;
				break; break;
			case 0b101 /* srli and srai */:
				// (TODO): Implement this
				this->pc = pc + 4;
				break;
			}
		}
		else if (opcode == opcode::stype) {
			int32_t imm_s = (imm_upper_btype & 0x80000000) ? 0xfffff000 : 0; // sign extension
			imm_s |= imm_upper_btype >> 20;
			imm_s |= imm_lower_btype >> 7;
			// so easy :) compared to b-type
			uint32_t memory_address = (imm_s + registers.REG[rs1]) % memory.size();
			uint32_t old_data = memory[memory_address];
			switch (funct3) {
			case 0b000 /* sb */:
				old_data &= 0xffffff00; // clear out bottom byte
				memory[memory_address] = old_data | (registers.REG[rs2] & 0x000000ff /* select bottom 8-bits*/);
				this->pc = pc + 4;
				break;
			case 0b001 /* sh */:
				old_data &= 0xffff0000; // clear out bottom 2-byte
				memory[memory_address] = old_data | (registers.REG[rs2] & 0x0000ffff /* select bottom 16-bits*/);
				this->pc = pc + 4;
				break;
			case 0b010 /* sw */:
				memory[memory_address] = registers.REG[rs2];
				this->pc = pc + 4;
				break;
			}
		}
		else if (opcode == opcode::rtype) {
			switch (funct3) {
			case 0b000 /* add/sub */:
				registers.REG[rd] = funct7 ? registers.REG[rs1] - registers.REG[rs2] : registers.REG[rs1] + registers.REG[rs2];
				this->pc = pc + 4;
				break;
			case 0b001 /* sll */:
				registers.REG[rd] = registers.REG[rs1] << registers.REG[rs2];
				this->pc = pc + 4;
				break;
			case 0b010 /* slt */:
				registers.REG[rd] = int32_t(registers.REG[rs1]) < int32_t(registers.REG[rs2]) ? 1 : 0;
				this->pc = pc + 4;
				break;
			case 0b011 /* sltu */:
				registers.REG[rd] = uint32_t(registers.REG[rs1]) < uint32_t(registers.REG[rs2]) ? 1 : 0;
				this->pc = pc + 4;
				break;
			case 0b101 /* srl or sra */:
				if (!funct7)
					registers.REG[rd] = uint32_t(registers.REG[rs1]) >> registers.REG[rs2];
				else
					registers.REG[rd] = int32_t(registers.REG[rs1]) >> registers.REG[rs2];
				break;
				this->pc = pc + 4;
			case 0b110 /* or */:
				registers.REG[rd] = registers.REG[rs1] | registers.REG[rs2];
				this->pc = pc + 4;
				break;
			case 0b111 /* and */:
				registers.REG[rd] = registers.REG[rs1] & registers.REG[rs2];
				this->pc = pc + 4;
				break;
			default:
				throw std::runtime_error("Invalid r-type funct3.");
			}
		}
		else {
			throw std::runtime_error("Invalid instruction type encountered.");
		}

		// 5) Present register state
		display_registers();
	}

	static int countDigits(int32_t number) {
		int digits = (number < 0); // Add 1 for the negative sign if the number is negative
		for (number = number < 0 ? -number : number; number; number /= 10) digits++;
		return digits ? digits : 1; // Ensure 0 returns 1 digit
	}

	void display_registers() const {

		int maxDigits = 0;
		for (int i = 0; i < 32; i++)
			maxDigits = std::max(countDigits((int32_t)registers.REG[i]), maxDigits);

		printf("\033[H");
		system("cls");

		printf("pc   = %0*d / 0x%08X     |     Cycle Count = %d\n", maxDigits, pc, pc, cycleCount);
		printf("zero = "); if (old_registers.REG[0  ] != registers.REG[0  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[0  ], registers.REG[0  ]); printf("\033[0m");
		printf("ra   = "); if (old_registers.REG[1  ] != registers.REG[1  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[1  ], registers.REG[1  ]); printf("\033[0m");
		printf("sp   = "); if (old_registers.REG[2  ] != registers.REG[2  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[2  ], registers.REG[2  ]); printf("\033[0m");
		printf("gp   = "); if (old_registers.REG[3  ] != registers.REG[3  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[3  ], registers.REG[3  ]); printf("\033[0m");
		printf("tp   = "); if (old_registers.REG[4  ] != registers.REG[4  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[4  ], registers.REG[4  ]); printf("\033[0m");
		printf("t0   = "); if (old_registers.REG[5  ] != registers.REG[5  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[5  ], registers.REG[5  ]); printf("\033[0m");
		printf("t1   = "); if (old_registers.REG[6  ] != registers.REG[6  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[6  ], registers.REG[6  ]); printf("\033[0m");
		printf("t2   = "); if (old_registers.REG[7  ] != registers.REG[7  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[7  ], registers.REG[7  ]); printf("\033[0m");
		printf("s0   = "); if (old_registers.REG[8  ] != registers.REG[8  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[8  ], registers.REG[8  ]); printf("\033[0m");
		printf("s1   = "); if (old_registers.REG[9  ] != registers.REG[9  ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[9  ], registers.REG[9  ]); printf("\033[0m");
		printf("a0   = "); if (old_registers.REG[10 ] != registers.REG[10 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[10 ], registers.REG[10 ]); printf("\033[0m");
		printf("a1   = "); if (old_registers.REG[11 ] != registers.REG[11 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[11 ], registers.REG[11 ]); printf("\033[0m");
		printf("a2   = "); if (old_registers.REG[12 ] != registers.REG[12 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[12 ], registers.REG[12 ]); printf("\033[0m");
		printf("a3   = "); if (old_registers.REG[13 ] != registers.REG[13 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[13 ], registers.REG[13 ]); printf("\033[0m");
		printf("a4   = "); if (old_registers.REG[14 ] != registers.REG[14 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[14 ], registers.REG[14 ]); printf("\033[0m");
		printf("a5   = "); if (old_registers.REG[15 ] != registers.REG[15 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[15 ], registers.REG[15 ]); printf("\033[0m");
		printf("a6   = "); if (old_registers.REG[16 ] != registers.REG[16 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[16 ], registers.REG[16 ]); printf("\033[0m");
		printf("a7   = "); if (old_registers.REG[17 ] != registers.REG[17 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[17 ], registers.REG[17 ]); printf("\033[0m");
		printf("s2   = "); if (old_registers.REG[18 ] != registers.REG[18 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[18 ], registers.REG[18 ]); printf("\033[0m");
		printf("s3   = "); if (old_registers.REG[19 ] != registers.REG[19 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[19 ], registers.REG[19 ]); printf("\033[0m");
		printf("s4   = "); if (old_registers.REG[20 ] != registers.REG[20 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[20 ], registers.REG[20 ]); printf("\033[0m");
		printf("s5   = "); if (old_registers.REG[21 ] != registers.REG[21 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[21 ], registers.REG[21 ]); printf("\033[0m");
		printf("s6   = "); if (old_registers.REG[22 ] != registers.REG[22 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[22 ], registers.REG[22 ]); printf("\033[0m");
		printf("s7   = "); if (old_registers.REG[23 ] != registers.REG[23 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[23 ], registers.REG[23 ]); printf("\033[0m");
		printf("s8   = "); if (old_registers.REG[24 ] != registers.REG[24 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[24 ], registers.REG[24 ]); printf("\033[0m");
		printf("s9   = "); if (old_registers.REG[25 ] != registers.REG[25 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[25 ], registers.REG[25 ]); printf("\033[0m");
		printf("s10  = "); if (old_registers.REG[26 ] != registers.REG[26 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[26 ], registers.REG[26 ]); printf("\033[0m");
		printf("s11  = "); if (old_registers.REG[27 ] != registers.REG[27 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[27 ], registers.REG[27 ]); printf("\033[0m");
		printf("t3   = "); if (old_registers.REG[28 ] != registers.REG[28 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[28 ], registers.REG[28 ]); printf("\033[0m");
		printf("t4   = "); if (old_registers.REG[29 ] != registers.REG[29 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[29 ], registers.REG[29 ]); printf("\033[0m");
		printf("t5   = "); if (old_registers.REG[30 ] != registers.REG[30 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[30 ], registers.REG[30 ]); printf("\033[0m");
		printf("t6   = "); if (old_registers.REG[31 ] != registers.REG[31 ]) printf("\033[31m"); printf("%0*d / 0x%08X\n", maxDigits, registers.REG[31 ], registers.REG[31 ]); printf("\033[0m");

		memcpy((void*)&old_registers, (void*)&registers, sizeof(register_file));
	}


public:
	enum opcode : uint8_t {
		lui = 0b0110111,
		aupic = 0b0010111,
		jal = 0b1101111,
		jalr = 0b1100111,
		btype = 0b1100011,
		stype = 0b0100011,
		itype_mem = 0b0000011,
		itype = 0b0010011,
		rtype = 0b0110011
	};

	struct register_file {
		union {
			int32_t REG[32];
			struct {
				/* x0  */ int32_t zero;
				/* x1  */ int32_t ra;  // return address	(Saved by caller)
				/* x2  */ int32_t sp;  // stack pointer	(Saved by callee)
				/* x3  */ int32_t gp;  // global pointer
				/* x4  */ int32_t tp;  // thread pointer
				/* x5  */ int32_t t0;  // temporary / alternate return address (Saved by caller)
				/* x6  */ int32_t t1;  // temporary (Saved by caller)
				/* x7  */ int32_t t2;  // temporary (Saved by caller)
				/* x8  */ int32_t s0;  // Saved register / frame pointer (Saved by caller)
				/* x9  */ int32_t s1;  // Saved register (Saved by caller)
				/* x10 */ int32_t a0;  // Function argument / return value (Caller)
				/* x11 */ int32_t a1;  // Function argument / return value (Caller)
				/* x12 */ int32_t a2;  // Function argument
				/* x13 */ int32_t a3;  // Function argument
				/* x14 */ int32_t a4;  // Function argument
				/* x15 */ int32_t a5;  // Function argument
				/* x16 */ int32_t a6;  // Function argument
				/* x17 */ int32_t a7;  // Function argument
				/* x18 */ int32_t s2;  // Saved register
				/* x19 */ int32_t s3;  // Saved register
				/* x20 */ int32_t s4;  // Saved register
				/* x21 */ int32_t s5;  // Saved register
				/* x22 */ int32_t s6;  // Saved register
				/* x23 */ int32_t s7;  // Saved register
				/* x24 */ int32_t s8;  // Saved register
				/* x25 */ int32_t s9;  // Saved register
				/* x26 */ int32_t s10; // Saved register
				/* x27 */ int32_t s11; // Saved register
				/* x28 */ int32_t t3;  // Temporary
				/* x29 */ int32_t t4;  // Temporary
				/* x30 */ int32_t t5;  // Temporary
				/* x31 */ int32_t t6;  // Temporary
			} alias;
		};
	};

protected:
	std::vector<uint32_t> memory;
	uint32_t pc; // program counter
	int cycleCount = 0;
	register_file registers;
	register_file old_registers;

};

int main() {
	cpu_risc32i rv;

	rv.load_program(0, {
	0x7ff00313, //addi x6 x0 2
	0x00800393, //addi x7 x0 8
	0x00730533, //add x10 x6 x7
	});

	std::vector<uint32_t> program_b = {
           0x00100293,//        addi x5 x0 1
           0x00000313,//        addi x6 x0 0
           0x00800413,//        addi x8 x0 8
           0x00100493,//        addi x9 x0 1
           0x00628e33,//        add x28 x5 x6
           0x00500333,//        add x6 x0 x5
           0x01c002b3,//        add x5 x0 x28
           0xfff40413,//        addi x8 x8 -1
           0xfe9458e3,//        bge x8 x9 -16 <loop>
	};

	rv.load_program(0, program_b);

	rv.display_registers();

	while (true) {
		system("PAUSE > NUL");
		rv.cycle();
	}
	/*
	0x7ff00313, //addi x6 x0 2
		0x00800393, //addi x7 x0 8
		0x00730533, //add x10 x6 x7*/
}