run:
	g++ -o maxc main.cpp token.cpp lexer.cpp parser.cpp ast.cpp program.cpp error.cpp type.cpp env.cpp  -g -Wall -Wextra
