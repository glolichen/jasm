#include <stdexcept>
#include <iostream>
#include <vector>

#include "lex.hpp"
#include "parse.hpp"

int main(int argc, char *argv[]) {
	if (argc == 1)
		throw std::runtime_error("pass a file through command line args");
	// https://stackoverflow.com/a/50520093/15246561
	std::vector<std::string> arg_list(argv + 1, argv + argc);

	std::vector<std::vector<lex::Lexeme>> tokens = lex::lex(arg_list[0]);
	std::vector<parse::Statement> stmts = parse::parse(tokens);

	// for (const parse::Statement &stmt : stmts) {
	// 	if (stmt.type != parse::STMTYPE_INSN)
	// 		continue;
	// 	std::vector<parse::Operand> ops = std::get<parse::Instruction>(stmt.val).operands;
	// 	for (const parse::Operand &op : ops) {
	// 		if (op.type != parse::OPTYPE_SIB)
	// 			continue;
	// 		std::cout << "sib found\n";
	// 	}
	// }

	return 0;
}

