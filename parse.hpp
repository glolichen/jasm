#ifndef PARSE_HPP
#define PARSE_HPP

#include "lex.hpp"
#include <cstdint>
#include <optional>

namespace parse {
	enum OperandType {
		OPTYPE_SIB,
		OPTYPE_REG,
		OPTYPE_IMM,
		OPTYPE_SYM,
		OPTYPE_UNRES_SIB,
		OPTYPE_UNRES_IMM,
	};
	struct ScaledIndexByte {
		std::optional<lex::Register> base, index;
		std::optional<uint32_t> disp, scale; 
	};
	// symbols that are not yet resolved are represented as a vector
	typedef std::vector<lex::Lexeme> Unresolved;
	struct Operand {
		OperandType type;
		std::variant<ScaledIndexByte, lex::Register, lex::Immediate64, std::string, Unresolved> val;
	};
	struct Instruction {
		lex::Instruction type;
		// either of size 0, 1 or 2
		// vector might be overkill but whatever
		std::vector<Operand> operands;
	};

	enum DirOperandType {
		DIROPTYPE_SYM,
		DIROPTYPE_IMM,
		DIROPTYPE_UNRES_IMM,
		DIROPTYPE_STR_LIT,
	};
	struct DirOperand {
		DirOperandType type;
		std::variant<uint64_t, std::string, Unresolved> val;
	};
	struct Directive {
		lex::Directive type;
		// supported directives all only have 1 operand
		DirOperand operand; 		
	};
	
	// for EQU
	struct Assignment {
		std::string symbol;
		bool is_resolved;
		std::variant<uint64_t, std::vector<lex::Lexeme>> val;
	};

	enum StatementType {
		STMTYPE_INSN,
		STMTYPE_DIR,
		STMTYPE_ASSIGN,
		STMTYPE_LBL,
	};
	struct Statement {
		StatementType type;
		std::variant<Instruction, Directive, Assignment, std::string> val;
	};

	void check_unres_imm(const std::vector<lex::Lexeme> &tokens);
	void check_unres_imm(const std::vector<lex::Lexeme> &tokens, size_t start, size_t end);
	bool check_unres_sib(const std::vector<lex::Lexeme> &tokens);
	void squash_immediates(std::vector<lex::Lexeme> &tokens, size_t start, size_t end);
	ScaledIndexByte parse_sib(const std::vector<lex::Lexeme> &tokens);	
	std::vector<Statement> parse(const std::vector<std::vector<lex::Lexeme>> &tokens);
}

#endif

