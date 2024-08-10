#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>
#include <string>
#include <set>
#include <cassert>
#include <iostream>

#include "parse.hpp"
#include "lex.hpp"

const uint32_t INSN_OPERANDS[] = {
	2, 2, 1, 1,
	2, 2, 1, 1, 2, 2,
	2, 2, 2, 1, 2, 2,
	1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1,
	2, 1, 0, 0,
};
const parse::DirOperandType DIR_OPERAND_TYPE[] = {
	parse::DIROPTYPE_IMM, parse::DIROPTYPE_IMM, parse::DIROPTYPE_IMM, parse::DIROPTYPE_IMM,
	parse::DIROPTYPE_IMM, parse::DIROPTYPE_IMM, parse::DIROPTYPE_IMM, parse::DIROPTYPE_IMM,
	parse::DIROPTYPE_SYM, parse::DIROPTYPE_SYM, parse::DIROPTYPE_SYM,
};

inline lex::Immediate64 val_to_imm64(uint64_t val) {
	uint32_t size = 64;
	if (val <= UINT8_MAX) size = 8;
	else if (val <= UINT16_MAX) size = 16;
	else if (val <= UINT32_MAX) size = 32;
	return lex::Immediate64 { size, val };
}
inline bool is_valid_scale(lex::Immediate64 imm) {
	return imm.val == 1 || imm.val == 2 || imm.val == 4 || imm.val == 8;
}

inline bool is_arithmetic(lex::LexemeType token) {
	return token == lex::LEXTYPE_ADD_SIGN || token == lex::LEXTYPE_MINUS_SIGN ||
			token == lex::LEXTYPE_ASTERISK || token == lex::LEXTYPE_SLASH;
}

void parse::check_unres_imm(const std::vector<lex::Lexeme> &tokens) {
	parse::check_unres_imm(tokens, 0, tokens.size() - 1);
}

// return true if the expression is OK
// otherwise throw an exception
void parse::check_unres_imm(const std::vector<lex::Lexeme> &tokens, size_t start, size_t end) {
	if (tokens.empty())
		lex::assemble_error(tokens[0].line_num, "empty operand not allowed");	
	bool is_arith_op = true;
	for (size_t i = start; i <= end; i++) {
		lex::LexemeType type = tokens[i].type;
		if (is_arith_op && (type == lex::LEXTYPE_SYMBOL || type == lex::LEXTYPE_IMM ||
				type == lex::LEXTYPE_DOLLAR)) {
			is_arith_op = !is_arith_op;
			continue;
		}
		if (!is_arith_op && is_arithmetic(type)) {
			is_arith_op = !is_arith_op;
			continue;
		}
		lex::assemble_error(tokens[0].line_num, "invalid operand");
	}
}

// true if expression is resolved, false if unresolved
// throw exception for syntax error syntax error
bool parse::check_unres_sib(const std::vector<lex::Lexeme> &tokens) {
	if (tokens.size() <= 2)
		return false;
	bool is_arith_op = true, is_resolved = true;
	for (size_t i = 1; i < tokens.size() - 1; i++) {
		lex::LexemeType type = tokens[i].type;
		if (is_arith_op && (type == lex::LEXTYPE_REG || type == lex::LEXTYPE_IMM)) {
			is_arith_op = !is_arith_op;
			continue;
		}
		if (is_arith_op && type == lex::LEXTYPE_SYMBOL) {
			is_arith_op = !is_arith_op;
			is_resolved = false;
			continue;
		}
		if (!is_arith_op && is_arithmetic(type)) {
			is_arith_op = !is_arith_op;
			continue;
		}
		lex::assemble_error(tokens[0].line_num, "invalid operand");
	}
	return is_resolved;
}

// TODO: add support for parentheses
void parse::squash_immediates(std::vector<lex::Lexeme> &tokens, size_t start, size_t end) {
	std::vector<bool> nosquash(end - start + 1);

	// first combine multiplications and divisions
	// mark "unsquashable" (operation with reg or sym)
	for (size_t i = start; i <= end; i++) {
		bool is_mul = tokens[i].type == lex::LEXTYPE_ASTERISK;
		bool is_div = tokens[i].type == lex::LEXTYPE_SLASH;
		if (is_mul || is_div) {
			assert(i != 0 && i != end);
			lex::Lexeme left = tokens[i - 1], right = tokens[i + 1];
			if (left.type == lex::LEXTYPE_IMM && right.type == lex::LEXTYPE_IMM) {
				uint64_t o1 = std::get<lex::Immediate64>(left.data).val;
				uint64_t o2 = std::get<lex::Immediate64>(right.data).val;
				uint64_t result;
				if (is_mul) result = o1 * o2;
				if (is_div) result = o1 / o2;
				tokens[i + 1].data.emplace<lex::Immediate64>(val_to_imm64(result));
				tokens.erase(std::next(tokens.begin(), i - 1), std::next(tokens.begin(), i + 1));
				nosquash.erase(std::next(nosquash.begin(), i - 1), std::next(nosquash.begin(), i + 1));
				i -= 2, end -= 2;
				continue;
			}
			nosquash[i - 1] = true, nosquash[i] = true, nosquash[i + 1] = true;
		}	
	}

	uint64_t total = 0;
	// store in decreasing order to make deleting elements easier
	std::set<size_t, std::greater<size_t>> squashed_tokens;
	for (size_t i = start; i <= end; i++) {
		if (nosquash[i] || tokens[i].type != lex::LEXTYPE_IMM)
			continue;

		bool is_add, is_sub;
		if (i == start)
			is_add = true, is_sub = false;
		else {
			lex::LexemeType prev = tokens[i - 1].type;
			is_add = prev == lex::LEXTYPE_ADD_SIGN;
			is_sub = prev == lex::LEXTYPE_MINUS_SIGN;
		}

		if (!is_add && !is_sub)
			lex::assemble_error(tokens[i].line_num, "invalid calculation");
		if (is_add)
			total += std::get<lex::Immediate64>(tokens[i].data).val;
		if (is_sub)
			total -= std::get<lex::Immediate64>(tokens[i].data).val;

		if (i != start)
			squashed_tokens.insert(i - 1);
		squashed_tokens.insert(i);
	}

	if (squashed_tokens.empty())
		return;

	for (const size_t &index : squashed_tokens)
		tokens.erase(tokens.begin() + index);

	tokens.insert(std::next(tokens.begin(), start), lex::Lexeme {
		lex::LEXTYPE_IMM, tokens[0].line_num, val_to_imm64(total)
	});

	if (tokens.size() <= start + 1 || is_arithmetic(tokens[start + 1].type))
		return;

	tokens.insert(std::next(tokens.begin(), start + 1), lex::Lexeme {
		lex::LEXTYPE_ADD_SIGN, tokens[0].line_num, std::monostate {}
	});
}

parse::ScaledIndexByte parse::parse_sib(const std::vector<lex::Lexeme> &tokens) {
	parse::ScaledIndexByte sib = { std::nullopt, std::nullopt, std::nullopt, std::nullopt };
	// tokens must be in the form:
	// base (GPR) + index (GPR) * scale (Imm{1,2,4,8}) + displacement (Imm32)
	// operands must have no symbols (in parse::parse they become parse::Unresolved)
	
	std::vector<bool> processed(tokens.size());
	for (uint32_t i = 1; i < tokens.size() - 1; i++) {
		if (tokens[i].type == lex::LEXTYPE_ASTERISK) {
			if (sib.index != std::nullopt || sib.scale != std::nullopt)
				lex::assemble_error(tokens[i].line_num, "invalid SIB expression");

			lex::Lexeme left = tokens[i - 1], right = tokens[i + 1];
			processed[i - 1] = true, processed[i] = true, processed[i + 1] = true;
	
			if (left.type == lex::LEXTYPE_REG && right.type == lex::LEXTYPE_IMM) {
				if (!is_valid_scale(std::get<lex::Immediate64>(right.data)))
					lex::assemble_error(left.line_num, "invalid SIB scale");
				sib.index = std::get<lex::Register>(left.data);
				sib.scale = std::get<lex::Immediate64>(right.data).val;
				continue;
			}
			if (left.type == lex::LEXTYPE_IMM && right.type == lex::LEXTYPE_REG) {
				if (!is_valid_scale(std::get<lex::Immediate64>(left.data)))
					lex::assemble_error(left.line_num, "invalid SIB scale");
				sib.index = std::get<lex::Register>(right.data);
				sib.scale = std::get<lex::Immediate64>(left.data).val;
				continue;
			}

			lex::assemble_error(left.line_num, "invalid SIB expression");	
		}
		if (tokens[i].type == lex::LEXTYPE_IMM || tokens[i].type == lex::LEXTYPE_REG ||
				tokens[i].type == lex::LEXTYPE_ADD_SIGN) {
			continue;
		}
		lex::assemble_error(tokens[i].line_num, "invalid SIB expression");
	}

	// when adjacent to plus sign (+):
	// first unprocessed register is the base
	// first unprocessed immediate is the displacement
	// if no index set yet, second unprocessed register is index
	// anything else = error
	for (uint32_t i = 1; i < tokens.size() - 1; i++) {
		if (tokens[i].type == lex::LEXTYPE_ADD_SIGN) {
			lex::Lexeme left = tokens[i - 1], right = tokens[i + 1];

			if (left.type == lex::LEXTYPE_REG && !processed[i - 1]) {
				if (sib.base == std::nullopt)
					sib.base = std::get<lex::Register>(left.data);
				else if (sib.index == std::nullopt)
					sib.index = std::get<lex::Register>(left.data);
				else
					lex::assemble_error(left.line_num, "invalid SIB expression");
			}
			if (right.type == lex::LEXTYPE_REG && !processed[i + 1]) {
				if (sib.base == std::nullopt)
					sib.base = std::get<lex::Register>(right.data);
				else if (sib.index == std::nullopt)
					sib.index = std::get<lex::Register>(right.data);
				else
					lex::assemble_error(left.line_num, "invalid SIB expression");
			}
			if (left.type == lex::LEXTYPE_IMM && !processed[i - 1]) {
				assert(sib.disp == std::nullopt);
				lex::Immediate64 imm = std::get<lex::Immediate64>(left.data);
				if (imm.size <= 32)
					sib.disp = imm.val;
				else
					lex::assemble_error(left.line_num, "displacement larger than 32 bits");
			}
			if (right.type == lex::LEXTYPE_IMM && !processed[i + 1]) {
				assert(sib.disp == std::nullopt);
				lex::Immediate64 imm = std::get<lex::Immediate64>(right.data);
				if (imm.size <= 32)
					sib.disp = imm.val;
				else
					lex::assemble_error(left.line_num, "displacement larger than 32 bits");
			}
	
			processed[i - 1] = true, processed[i] = true, processed[i + 1] = true;

			if ((left.type != lex::LEXTYPE_IMM && left.type != lex::LEXTYPE_REG) ||
					(right.type != lex::LEXTYPE_IMM && right.type != lex::LEXTYPE_REG)) {
				lex::assemble_error(left.line_num, "invalid SIB expression");
			}
		}
	}
	return sib;
}

std::vector<parse::Statement> parse::parse(const std::vector<std::vector<lex::Lexeme>> &tokens) {
	std::vector<parse::Statement> stmts;
	for (std::vector<lex::Lexeme> ltokens : tokens) {
		assert(!ltokens.empty());
		if (ltokens[0].type == lex::LEXTYPE_INSN) {
			std::vector<std::vector<lex::Lexeme>> operands;
			if (ltokens.size() >= 2)
				operands.emplace_back(std::vector<lex::Lexeme>());
			for (uint32_t i = 1; i < ltokens.size(); i++) {
				if (ltokens[i].type == lex::LEXTYPE_COMMA) {
					if (operands.back().empty())
						lex::assemble_error(ltokens[i].line_num, "invalid use of commas");
					operands.emplace_back(std::vector<lex::Lexeme>());
					continue;
				}
				operands.back().emplace_back(ltokens[i]);
			}
			if (INSN_OPERANDS[std::get<lex::Instruction>(ltokens[0].data)] != operands.size())
				lex::assemble_error(ltokens[0].line_num, "invalid number of operands");
			
			parse::Instruction insn = {
				std::get<lex::Instruction>(ltokens[0].data),
				std::vector<parse::Operand>(operands.size()),
			};

			for (uint32_t i = 0; i < operands.size(); i++) {
				std::vector<lex::Lexeme> ops = operands[i];
				assert(!ops.empty());

				if (ops.front().type == lex::LEXTYPE_OPEN_BRACKET &&
						ops.back().type == lex::LEXTYPE_CLOSE_BRACKET) {
					parse::squash_immediates(ops, 1, ops.size() - 2);
				}
				else
					parse::squash_immediates(ops, 0, ops.size() - 1);

				if (ops.size() == 1) {
					if (ops[0].type == lex::LEXTYPE_REG)
						insn.operands[i] = { parse::OPTYPE_REG, std::get<lex::Register>(ops[0].data) };
					else if (ops[0].type == lex::LEXTYPE_IMM)
						insn.operands[i] = { parse::OPTYPE_IMM, std::get<lex::Immediate64>(ops[0].data) };
					else if (ops[0].type == lex::LEXTYPE_SYMBOL)
						insn.operands[i] = { parse::OPTYPE_SYM, std::get<std::string>(ops[0].data) };
					else
						lex::assemble_error(ops[0].line_num, "invalid operand");
					continue;
				}

				// SIB, possibly unresolved
				if (ops.front().type == lex::LEXTYPE_OPEN_BRACKET &&
						ops.back().type == lex::LEXTYPE_CLOSE_BRACKET) {
					// checking of the current operand eventually works out to a valid SIB expression
					// right now is too hard, so we'll just shove it in as an unresolved and check if
					// it's valid after resolving the symbols after the first assembler pass
					if (parse::check_unres_sib(ops))
						insn.operands[i] = { parse::OPTYPE_SIB, parse::parse_sib(ops) };
					else
						insn.operands[i] = { parse::OPTYPE_UNRES_SIB, ops };
					continue;
				}

				// unresolved immediate
				parse::check_unres_imm(ops);
				insn.operands[i] = { parse::OPTYPE_UNRES_IMM, ops };
			}

			stmts.emplace_back(parse::Statement { parse::STMTYPE_INSN, insn });
			continue;
		}

		if (ltokens[0].type == lex::LEXTYPE_SYMBOL) {
			if (ltokens.size() < 2)
				lex::assemble_error(ltokens[0].line_num, "label must be followed by colon");
			if (ltokens.size() == 2 && ltokens[1].type == lex::LEXTYPE_COLON) {
				stmts.emplace_back(parse::Statement {
					parse::STMTYPE_LBL,
					std::get<std::string>(ltokens[0].data)
				});
				continue;
			}
			if (ltokens[1].type == lex::LEXTYPE_EQU) {
				if (ltokens.size() < 3)
					lex::assemble_error(ltokens[0].line_num, "not enough operands for EQU");
				
				parse::squash_immediates(ltokens, 2, ltokens.size() - 1);
				
				if (ltokens.size() == 3) {
					if (ltokens[2].type == lex::LEXTYPE_IMM) {
						stmts.emplace_back(parse::Statement {
							parse::STMTYPE_ASSIGN,
							parse::Assignment {
								std::get<std::string>(ltokens[0].data),
								true, std::get<lex::Immediate64>(ltokens[2].data).val
							}
						});
					}
					else if (ltokens[2].type == lex::LEXTYPE_STR_LIT) {
						std::string lit = std::get<std::string>(ltokens[2].data);
						if (lit.size() > 8)
							lex::assemble_error(ltokens[0].line_num, "string literal too large to fit in quadword");
						// x86 is little endian -- least significant byte is first
						// in a string, the first character is the least significnat
						uint64_t val = 0;
						for (size_t i = 0; i < lit.size(); i++)
							val |= lit[i] << (i * 8);
						stmts.emplace_back(parse::Statement {
							parse::STMTYPE_ASSIGN,
							parse::Assignment {
								std::get<std::string>(ltokens[0].data),
								true, val
							}
						});
					}
					else
						lex::assemble_error(ltokens[0].line_num, "cannot assign operand");
					continue;
				}

				parse::check_unres_imm(ltokens, 2, ltokens.size() - 1);
				stmts.emplace_back(parse::Statement {
					parse::STMTYPE_ASSIGN,
					parse::Assignment {
						std::get<std::string>(ltokens[0].data),
						false, std::vector<lex::Lexeme>(std::next(ltokens.begin(), 2), ltokens.end())
					}
				});
				continue;
			}
			lex::assemble_error(ltokens[0].line_num, "invalid use of symbols");
		}

		if (ltokens[0].type == lex::LEXTYPE_DIRECTIVE) {
			if (ltokens.size() == 1)
				lex::assemble_error(ltokens[0].line_num, "not enough operands for directive");

			lex::Directive dirtype = std::get<lex::Directive>(ltokens[0].data);
			parse::DirOperandType optype = DIR_OPERAND_TYPE[dirtype];
			if (optype == parse::DIROPTYPE_SYM) {
				if (ltokens.size() != 2 || ltokens[1].type != lex::LEXTYPE_SYMBOL)
					lex::assemble_error(ltokens[0].line_num, "invalid directive operand");
				stmts.emplace_back(parse::Statement {
					parse::STMTYPE_DIR,
					parse::Directive {
						dirtype,
						parse::DirOperand {
							parse::DIROPTYPE_SYM,
							std::get<std::string>(ltokens[1].data)
						}
					}
				});
				continue;
			}

			assert(optype == parse::DIROPTYPE_IMM);

			// number directive operand
			parse::squash_immediates(ltokens, 1, ltokens.size() - 1);
			if (ltokens.size() == 2) {
				if (dirtype == lex::DB && ltokens[1].type == lex::LEXTYPE_STR_LIT) {
					stmts.emplace_back(parse::Statement {
						parse::STMTYPE_DIR,
						parse::Directive {
							dirtype,
							parse::DirOperand {
								parse::DIROPTYPE_STR_LIT,
								std::get<std::string>(ltokens[1].data)
							}
						}
					});
					continue;
				}
				if (ltokens[1].type != lex::LEXTYPE_IMM)
					lex::assemble_error(ltokens[0].line_num, "invalid directive operand");
				stmts.emplace_back(parse::Statement {
					parse::STMTYPE_DIR,
					parse::Directive {
						dirtype,
						parse::DirOperand {
							parse::DIROPTYPE_IMM,
							std::get<lex::Immediate64>(ltokens[1].data).val
						}
					}
				});
				continue;
			}

			// this should catch any illegal expressions
			parse::check_unres_imm(ltokens);
			stmts.emplace_back(parse::Statement {
				parse::STMTYPE_DIR,
				parse::Directive {
					dirtype,
					parse::DirOperand {
						parse::DIROPTYPE_UNRES_IMM,
						std::vector<lex::Lexeme>(std::next(ltokens.begin()), ltokens.end())
					}
				}
			});
		}

		lex::assemble_error(ltokens[0].line_num, "line must start with instruction, directive or label");
	}
	return stmts;
}

