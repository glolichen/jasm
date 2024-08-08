#ifndef PARSE_HPP
#define PARSE_HPP

#include "lex.hpp"
#include <cstdint>

namespace parse {
	enum OperandType {
		OPTYPE_SIB,
		OPTYPE_REG,
		OPTYPE_IMM,
		OPTYPE_SYM,
	};
	struct ScaledIndexByte {
		lex::Register base, index;
		lex::Immediate32 disp, scale; 
	};
	struct Operand {
		OperandType type;
		std::variant<ScaledIndexByte, lex::Register, lex::Immediate32, std::string> val;
	};
	struct Instruction {
		lex::Instruction type;
		// either of size 0, 1 or 2
		// vector might be overkill but whatever
		std::vector<Operand> operands;
	};

	enum DirOperandType {
		DIROPTYPE_NUM,
		DIROPTYPE_STR,
	};
	struct DirOperand {
		DirOperandType type;
		std::variant<uint64_t, std::string> val;
	};
	struct Directive {
		lex::Directive type;
		// supported directives all only have 1 operand
		DirOperand operand; 		
	};

	enum StatementType {
		STMTYPE_INSN,
		STMTYPE_DIR,
		STMTYPE_LBL,
	};
	struct Statement {
		StatementType type;
		std::variant<Instruction, Directive, std::string> val;
	};

	void parse(std::vector<Statement> &stmts, const std::vector<std::vector<lex::Lexeme>> &tokens);
}

#endif
