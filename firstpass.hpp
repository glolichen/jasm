#ifndef FIRSTPASS_HPP
#define	FIRSTPASS_HPP

#include <string>
#include <cstdint>
#include <vector>
#include "lex.hpp"

namespace firstpass {
	enum Segment {
		Code, Data
	};

	struct Symbol {
		std::string symbol;
		uint64_t offset;
		Segment segment;
		unsigned size;
	};

	void firstpass(std::vector<Symbol> &symtab, const std::vector<lex::Lexeme> &tokens);
}

#endif
