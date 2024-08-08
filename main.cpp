#include <vector>

#include "lex.hpp"
#include "parse.hpp"

int main() {
	std::vector<std::vector<lex::Lexeme>> tokens;
	lex::lex(tokens, "file.s");

	std::vector<parse::Statement> stmts;
	parse::parse(stmts, tokens);

	return 0;
}
