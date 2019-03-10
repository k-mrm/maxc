run:
	g++ -o maxc main.cpp token.cpp lexer.cpp parser.cpp ast.cpp vmcode.cpp vm.cpp error.cpp type.cpp env.cpp  -Wall -Wextra
