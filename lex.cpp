#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>
#include <variant>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <regex>
#include <climits>
#include <iostream>
#include <sstream>

#include "lex.hpp"

const std::unordered_map<std::string, lex::Directive> DIRECTIVES = {
	{ "db", lex::DB },
	{ "dw", lex::DW },
	{ "dd", lex::DD },
	{ "dq", lex::DQ },
	{ "resb", lex::RESB },
	{ "resw", lex::RESW },
	{ "resd", lex::RESD },
	{ "resq", lex::RESQ },
	{ "global", lex::GLOBAL },
	{ "extern", lex::EXTERN },
	{ "section", lex::SECTION },
};

const std::unordered_map<std::string, lex::Instruction> INSNS = {
	{ "mov", lex::MOV },
	{ "lea", lex::LEA },
	{ "push", lex::PUSH },
	{ "pop", lex::POP },
	{ "add", lex::ADD },
	{ "sub", lex::SUB },
	{ "inc", lex::INC },
	{ "dec", lex::DEC },
	{ "imul", lex::IMUL },
	{ "idiv", lex::IDIV },
	{ "and", lex::AND },
	{  "or", lex::OR  },
	{ "xor", lex::XOR },
	{ "not", lex::NOT },
	{ "shl", lex::SHL },
	{ "shr", lex::SHR },
	{ "jmp", lex::JMP },
	{  "je", lex::JE  },
	{ "jne", lex::JNE },
	{  "jg", lex::JG  },
	{ "jge", lex::JGE },
	{  "jl", lex::JL  },
	{ "jle", lex::JLE },
	{  "ja", lex::JA  },
	{ "jae", lex::JAE },
	{  "jb", lex::JB  },
	{ "jbe", lex::JBE },
	{ "cmp", lex::CMP },
	{ "call", lex::CALL },
	{ "ret", lex::RET },
	{ "syscall", lex::SYSCALL },
};

const std::unordered_map<std::string, lex::Register> REGS = {
	{ "al",  lex::Register { lex::REGTYPE_GPR8,  lex::AL  } },
	{ "ah",  lex::Register { lex::REGTYPE_GPR8,  lex::AH  } },
	{ "ax",  lex::Register { lex::REGTYPE_GPR16, lex::AX  } },
	{ "eax", lex::Register { lex::REGTYPE_GPR32, lex::EAX } },
	{ "rax", lex::Register { lex::REGTYPE_GPR64, lex::RAX } },

	{ "bl",  lex::Register { lex::REGTYPE_GPR8,  lex::BL  } },
	{ "bh",  lex::Register { lex::REGTYPE_GPR8,  lex::BH  } },
	{ "bx",  lex::Register { lex::REGTYPE_GPR16, lex::BX  } },
	{ "ebx", lex::Register { lex::REGTYPE_GPR32, lex::EBX } },
	{ "rbx", lex::Register { lex::REGTYPE_GPR64, lex::RBX } },
	  
	{ "cl",  lex::Register { lex::REGTYPE_GPR8,  lex::CL  } },
	{ "ch",  lex::Register { lex::REGTYPE_GPR8,  lex::CH  } },
	{ "cx",  lex::Register { lex::REGTYPE_GPR16, lex::CX  } },
	{ "ecx", lex::Register { lex::REGTYPE_GPR32, lex::ECX } },
	{ "rcx", lex::Register { lex::REGTYPE_GPR64, lex::RCX } },
	  
	{ "dl",  lex::Register { lex::REGTYPE_GPR8,  lex::DL  } },
	{ "dh",  lex::Register { lex::REGTYPE_GPR8,  lex::DH  } },
	{ "dx",  lex::Register { lex::REGTYPE_GPR16, lex::DX  } },
	{ "edx", lex::Register { lex::REGTYPE_GPR32, lex::EDX } },
	{ "rdx", lex::Register { lex::REGTYPE_GPR64, lex::RDX } },
  
	{ "sil", lex::Register { lex::REGTYPE_GPR8,  lex::SIL } },
	{ "si",  lex::Register { lex::REGTYPE_GPR16, lex::SI  } },
	{ "esi", lex::Register { lex::REGTYPE_GPR32, lex::ESI } },
	{ "rsi", lex::Register { lex::REGTYPE_GPR64, lex::RSI } },

	{ "dil", lex::Register { lex::REGTYPE_GPR8,  lex::DIL } },
	{ "di",  lex::Register { lex::REGTYPE_GPR16, lex::DI  } },
	{ "edi", lex::Register { lex::REGTYPE_GPR32, lex::EDI } },
	{ "rdi", lex::Register { lex::REGTYPE_GPR64, lex::RDI } },
	
	{ "spl", lex::Register { lex::REGTYPE_GPR8,  lex::SPL } },
	{ "sp",  lex::Register { lex::REGTYPE_GPR16, lex::SP  } },
	{ "esp", lex::Register { lex::REGTYPE_GPR32, lex::ESP } },
	{ "rsp", lex::Register { lex::REGTYPE_GPR64, lex::RSP } },

	{ "bpl", lex::Register { lex::REGTYPE_GPR8,  lex::BPL } },
	{ "bp",  lex::Register { lex::REGTYPE_GPR16, lex::BP  } },
	{ "ebp", lex::Register { lex::REGTYPE_GPR32, lex::EBP } },
	{ "rbp", lex::Register { lex::REGTYPE_GPR64, lex::RBP } },

	{ "r8b", lex::Register { lex::REGTYPE_GPR8,  lex::R8B } },
	{ "r8w", lex::Register { lex::REGTYPE_GPR16, lex::R8W } },
	{ "r8d", lex::Register { lex::REGTYPE_GPR32, lex::R8D } },
	{ "r8" , lex::Register { lex::REGTYPE_GPR64, lex::R8  } },
	
	{ "r9b", lex::Register { lex::REGTYPE_GPR8,  lex::R9B } },
	{ "r9w", lex::Register { lex::REGTYPE_GPR16, lex::R9W } },
	{ "r9d", lex::Register { lex::REGTYPE_GPR32, lex::R9D } },
	{ "r9" , lex::Register { lex::REGTYPE_GPR64, lex::R9  } },
	
	{ "r10b", lex::Register { lex::REGTYPE_GPR8,  lex::R10B } },
	{ "r10w", lex::Register { lex::REGTYPE_GPR16, lex::R10W } },
	{ "r10d", lex::Register { lex::REGTYPE_GPR32, lex::R10D } },
	{ "r10" , lex::Register { lex::REGTYPE_GPR64, lex::R10  } },
	
	{ "r11b", lex::Register { lex::REGTYPE_GPR8,  lex::R11B } },
	{ "r11w", lex::Register { lex::REGTYPE_GPR16, lex::R11W } },
	{ "r11d", lex::Register { lex::REGTYPE_GPR32, lex::R11D } },
	{ "r11" , lex::Register { lex::REGTYPE_GPR64, lex::R11  } },
	
	{ "r12b", lex::Register { lex::REGTYPE_GPR8,  lex::R12B } },
	{ "r12w", lex::Register { lex::REGTYPE_GPR16, lex::R12W } },
	{ "r12d", lex::Register { lex::REGTYPE_GPR32, lex::R12D } },
	{ "r12" , lex::Register { lex::REGTYPE_GPR64, lex::R12  } },
	
	{ "r13b", lex::Register { lex::REGTYPE_GPR8,  lex::R13B } },
	{ "r13w", lex::Register { lex::REGTYPE_GPR16, lex::R13W } },
	{ "r13d", lex::Register { lex::REGTYPE_GPR32, lex::R13D } },
	{ "r13" , lex::Register { lex::REGTYPE_GPR64, lex::R13  } },
	
	{ "r14b", lex::Register { lex::REGTYPE_GPR8,  lex::R14B } },
	{ "r14w", lex::Register { lex::REGTYPE_GPR16, lex::R14W } },
	{ "r14d", lex::Register { lex::REGTYPE_GPR32, lex::R14D } },
	{ "r14" , lex::Register { lex::REGTYPE_GPR64, lex::R14  } },
	
	{ "r15b", lex::Register { lex::REGTYPE_GPR8,  lex::R15B } },
	{ "r15w", lex::Register { lex::REGTYPE_GPR16, lex::R15W } },
	{ "r15d", lex::Register { lex::REGTYPE_GPR32, lex::R15D } },
	{ "r15" , lex::Register { lex::REGTYPE_GPR64, lex::R15  } },
	
	{ "cr0", lex::Register { lex::REGTYPE_CONTROL, lex::CR0 } },
	{ "cr2", lex::Register { lex::REGTYPE_CONTROL, lex::CR2 } },
	{ "cr3", lex::Register { lex::REGTYPE_CONTROL, lex::CR3 } },
	{ "cr4", lex::Register { lex::REGTYPE_CONTROL, lex::CR4 } },
	
	{ "ss", lex::Register { lex::REGTYPE_SEG, lex::SS } },
	{ "cs", lex::Register { lex::REGTYPE_SEG, lex::CS } },
	{ "ds", lex::Register { lex::REGTYPE_SEG, lex::DS } },
	{ "es", lex::Register { lex::REGTYPE_SEG, lex::ES } },
	{ "fs", lex::Register { lex::REGTYPE_SEG, lex::FS } },
	{ "gs", lex::Register { lex::REGTYPE_SEG, lex::GS } },
};

extern const unsigned lex::INSN_OPERANDS[] = {
	2, 2, 1, 1,
	2, 2, 1, 1, 2, 2,
	2, 2, 2, 1, 2, 2,
	1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 0, 0,
};

std::string lex::file_name;

const std::unordered_set<char> DELIMS = {
	' ', ',', '*', '+', '-', '/', '[', ']', ':', '$'
};

// no need to match uppercase because split_line lowercases string
const std::regex HEX_REGEX = std::regex("0x[0-9a-f]+");
const std::regex DEC_REGEX = std::regex("-?[0-9]+");

inline lex::Immediate64 make_imm(unsigned bits, std::variant<int64_t, uint64_t> val) {
	uint64_t uval;
	if (val.index() == 0)
		uval = (uint64_t) (-std::get<int64_t>(val));
	else
		uval = std::get<uint64_t>(val);
	return lex::Immediate64 { bits, uval };
}

void syntax_error(uint32_t line_num, std::string msg) {
	std::stringstream ss;
	ss << "syntax error on " << lex::file_name << ":" << line_num << ": " << msg;
	throw std::runtime_error(ss.str());
}

std::string to_lower(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), [](char c) {
		return (c >= 'A' && c <= 'Z') ? c - ('A' - 'a') : c;
	});
	return str;
}

std::vector<std::string> split_line(std::string line, uint32_t line_num) {
	bool is_string_lit = false;
	std::vector<std::string> ret;
	std::string current = "";
	for (uint32_t i = 0; i < line.length(); i++) {	
		char letter = line[i];
		if (!is_string_lit && DELIMS.count(letter)) {
			if (current.length() > 0)
				ret.push_back(to_lower(current));
			std::string delim(1, letter);
			ret.push_back(delim);
			current = "";
			continue;
		}
		if (letter == '"') {
			is_string_lit = !is_string_lit;
			continue;
		}
		current += letter;
	}
	if (is_string_lit)
		syntax_error(line_num, "unclosed string literal");
	if (current.length() > 0)
		ret.push_back(to_lower(current));
	return ret;
}

void lex::lex(std::vector<std::vector<lex::Lexeme>> &tokens, std::string file_name) {
	std::ifstream file(file_name);
	std::string line;

	lex::file_name = file_name;

	for (unsigned line_num = 1; std::getline(file, line); line_num++) {
		uint32_t line_start, line_end;
		for (line_start = 0; line[line_start] == ' ' || line[line_start] == '\t'; line_start++);
		for (line_end = line_start; line[line_end] != ';' && line_end < line.length(); line_end++);

		line = line.substr(line_start, line_end - line_start);
		std::vector<std::string> line_tokens_str = split_line(line, line_num);
		std::vector<lex::Lexeme> ltokens;

		for (const std::string &token : line_tokens_str) {
			if (token == ",")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_COMMA, line_num, std::monostate{} });
			else if (token == "*")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_ASTERISK, line_num, std::monostate{} });
			else if (token == "+")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_ADD_SIGN, line_num, std::monostate{} });
			else if (token == "-")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_MINUS_SIGN, line_num, std::monostate{} });
			else if (token == "/")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_SLASH, line_num, std::monostate{} });
			else if (token == "[")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_OPEN_BRACKET, line_num, std::monostate{} });
			else if (token == "]")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_CLOSE_BRACKET, line_num, std::monostate{} });
			else if (token == ":")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_COLON, line_num, std::monostate{} });
			else if (token == "$")
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_DOLLAR, line_num, std::monostate{} });
			else if (INSNS.count(token))
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_INSN, line_num, INSNS.find(token)->second });
			else if (DIRECTIVES.count(token))
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_DIRECTIVE, line_num, DIRECTIVES.find(token)->second });
			else if (REGS.count(token))
				ltokens.emplace_back(lex::Lexeme { LEXTYPE_REG, line_num, REGS.find(token)->second });
			else {
				bool is_dec = std::regex_match(token, DEC_REGEX);
				bool is_hex = std::regex_match(token, HEX_REGEX);
				if (is_dec || is_hex) {
					size_t len;
					lex::LexemeType type;  
					if ((len = ltokens.size()) >= 1 && ltokens[len - 1].type == LEXTYPE_MINUS_SIGN &&
							(len >= 2 || (ltokens[len - 2].type != LEXTYPE_IMM &&
							ltokens[len - 2].type != LEXTYPE_REG))) {

						ltokens.pop_back();
						int64_t num = std::stoll(token, nullptr, is_hex ? 16 : 10);
						if (num >= INT8_MIN && num <= INT8_MAX) 
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(8, -num) });
						else if (num >= INT16_MIN && num <= INT16_MAX)
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(16, -num) });
						else if (num >= INT32_MIN && num <= INT32_MAX)
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(32, -num) });
						else if (num >= INT64_MIN && num <= INT64_MAX)
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(64, -num) });
					}
					else {
						uint64_t num = std::stoull(token, nullptr, is_hex ? 16 : 10);
						if (num <= UINT8_MAX)
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(8, num) });
						else if (num <= UINT16_MAX)
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(16, num) });
						else if (num <= UINT32_MAX)
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(32, num) });
						else if (num <= UINT64_MAX) 
							ltokens.emplace_back(lex::Lexeme { LEXTYPE_IMM, line_num, make_imm(64, num) });
					}
				}
				else if (token.length() >= 2 && token[0] == '"' && token[token.length() - 1] == '"')
					ltokens.emplace_back(lex::Lexeme { LEXTYPE_STR_LIT, line_num, token });
				else if (token != " ")
					ltokens.emplace_back(lex::Lexeme { LEXTYPE_SYMBOL, line_num, token });
			}
		}
		if (ltokens.size())
			tokens.emplace_back(ltokens);
	}

	file.close();

	for (std::vector<lex::Lexeme> &ltokens : tokens) {
		uint32_t bracket_count = 0;
		for (int i = 0; i < ltokens.size(); i++) {
			lex::Lexeme lexeme = ltokens[i];   
			bool is_add = lexeme.type == LEXTYPE_ADD_SIGN;
			bool is_sub = lexeme.type == LEXTYPE_MINUS_SIGN;
			bool is_mul = lexeme.type == LEXTYPE_ASTERISK;
			bool is_div = lexeme.type == LEXTYPE_SLASH;

			if (is_add || is_sub || is_mul || is_div) {
				if (i == 0 || i == ltokens.size() - 1)
					syntax_error(lexeme.line_num, "");
				if (ltokens[i - 1].type == LEXTYPE_IMM && ltokens[i + 1].type == LEXTYPE_IMM) {
					uint64_t o1 = std::get<lex::Immediate64>(ltokens[i - 1].data).val;
					uint64_t o2 = std::get<lex::Immediate64>(ltokens[i + 1].data).val;
					uint64_t result;
					if (is_add) result = o1 + o2;
					if (is_sub) result = o1 - o2;
					if (is_mul) result = o1 * o2;
					if (is_div) result = o1 / o2;

					uint32_t size = 64;
					if (result <= UINT8_MAX) size = 8;
					else if (result <= UINT16_MAX) size = 16;
					else if (result <= UINT32_MAX) size = 32;
					
					ltokens[i + 1].data.emplace<lex::Immediate64>(lex::Immediate64 { size, result });
					ltokens.erase(std::next(ltokens.begin(), i - 1), std::next(ltokens.begin(), i + 1));
					i -= 2;  
				} 
				else if ((ltokens[i - 1].type != LEXTYPE_IMM && ltokens[i + 1].type != LEXTYPE_IMM) &&
						 (ltokens[i - 1].type != LEXTYPE_REG && ltokens[i + 1].type != LEXTYPE_REG) &&
						 (ltokens[i - 1].type != LEXTYPE_DOLLAR && ltokens[i + 1].type != LEXTYPE_DOLLAR))
					syntax_error(lexeme.line_num, "");

				continue;
			}

			if (lexeme.type == LEXTYPE_OPEN_BRACKET)
				bracket_count++;
			if (lexeme.type == LEXTYPE_CLOSE_BRACKET)
				bracket_count--;

			if (bracket_count != 0 && bracket_count != 1)
				syntax_error(lexeme.line_num, "bracket error");
			if (i == ltokens.size() - 1) {
				if (bracket_count)
					syntax_error(lexeme.line_num, "bracket error");
				bracket_count = 0;
				continue;
			}

			if (lexeme.type == LEXTYPE_COLON) {
				if (i == 0)
					syntax_error(lexeme.line_num, "colon cannot be first character in line");
				if (i != ltokens.size() - 1)
					syntax_error(lexeme.line_num, "label must be followed by new line");
				if (ltokens[i - 1].type != LEXTYPE_SYMBOL)
					syntax_error(lexeme.line_num, "invalid label name");
				if (i >= 2 && ltokens[i - 2].type != LEXTYPE_NEWLINE)
					syntax_error(lexeme.line_num, "label must be only symbol in line");
				continue;
			}

			// TODO: add other syntax checks
		}
	}
}

