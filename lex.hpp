#ifndef LEX_HPP
#define LEX_HPP

#include <climits>
#include <string>
#include <vector>
#include <cstdint>
#include <variant>
#include <unordered_set>

namespace lex {
	extern std::string file_name;

	enum Directive {
		DB, DW, DD, DQ,
		RESB, RESW, RESD, RESQ,
		GLOBAL, EXTERN, SECTION
	};
	enum Instruction {
		MOV, LEA, PUSH, POP,
		ADD, SUB, INC, DEC, IMUL, IDIV,
		AND, OR, XOR, NOT, SHL, SHR,
		JMP, JE, JNE, JG, JGE, JL, JLE,
		JA, JAE, JB, JBE,
		CMP, CALL, RET, SYSCALL,
	};
	extern const unsigned INSN_OPERANDS[];

	enum RegisterType {
		REGTYPE_GPR8,
		REGTYPE_GPR16,
		REGTYPE_GPR32,
		REGTYPE_GPR64,
		REGTYPE_CONTROL,
		REGTYPE_SEG,
	};
	enum GPRegs8 {
		AH, BH, CH, DH,
		AL, BL, CL, DL,
		SPL, BPL, DIL, SIL,
		R8B, R9B, R10B, R11B,
		R12B, R13B, R14B, R15B,
	};
	enum GPRegs16 {
		AX, BX, CX, DX,
		SP, BP, DI, SI,
		R8W, R9W, R10W, R11W,
		R12W, R13W, R14W, R15W,
	};
	enum GPRegs32 {
		EAX, EBX, ECX, EDX,
		ESP, EBP, EDI, ESI,
		R8D, R9D, R10D, R11D,
		R12D, R13D, R14D, R15D,
	};
	enum GPRegs64 {
		RAX, RBX, RCX, RDX,
		RSP, RBP, RDI, RSI,
		R8, R9, R10, R11,
		R12, R13, R14, R15,
	};
	enum ControlRegs {
		CR0, CR2, CR3, CR4
	};
	enum SegmentRegs {
		SS, CS, DS, ES, FS, GS
	};
	struct Register {
		RegisterType type;
		std::variant<GPRegs8, GPRegs16, GPRegs32, GPRegs64, ControlRegs, SegmentRegs> reg;
	};

	// max size 64 and 32
	// certain data (such as displacement) are 32 bit max
	struct Immediate64 {
		uint32_t size;
		uint64_t val;
	};
	struct Immediate32 {
		uint32_t size;
		uint32_t val;
	};

	enum LexemeType {
		LEXTYPE_DIRECTIVE,
		LEXTYPE_INSN,
		LEXTYPE_REG,
		LEXTYPE_IMM,		
		LEXTYPE_SYMBOL,
		LEXTYPE_COMMA,
		LEXTYPE_ASTERISK,
		LEXTYPE_ADD_SIGN,
		LEXTYPE_MINUS_SIGN,
		LEXTYPE_SLASH,
		LEXTYPE_OPEN_BRACKET,
		LEXTYPE_CLOSE_BRACKET,
		LEXTYPE_COLON,
		LEXTYPE_DOLLAR,
		LEXTYPE_STR_LIT,
		LEXTYPE_NEWLINE,
	};

	struct Lexeme {
		enum LexemeType type;
		unsigned line_num;
		std::variant<std::string, Immediate64, Register, Directive, Instruction, std::monostate> data;
	};

	void lex(std::vector<std::vector<Lexeme>> &tokens, std::string file_name);
}

#endif

