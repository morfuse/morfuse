#include "Lexer.hpp"

using namespace mfuse;

Lexer::Lexer(ParseTree& inParsetree, std::istream& arg_yyin, std::ostream* arg_yyout)
	: yyFlexLexer(&arg_yyin, arg_yyout)
	, parsetree(inParsetree)
	, braces_count(0)
	, pos(0)
{
}

Lexer::~Lexer()
{

}

void Lexer::yyllocset(Parser::location_type* loc, uint32_t off)
{
	loc->sourcePos = pos - yyleng + off;
	loc->begin.line = yylineno;
	loc->begin.column = (loc->sourcePos >= loc->lineSourcePos) ? (loc->sourcePos - loc->lineSourcePos) : 0;
	loc->end.line = loc->begin.line;
	loc->end.column = loc->begin.column;
	for (uint32_t i = 0, column = 0; i < (uint32_t)yyleng; ++i, ++column)
	{
		if (yytext[i] == '\n')
		{
			column = 0;
			loc->end.line++;
		}
		loc->end.column = column;
	}
}

void Lexer::yyreducepos(uint32_t off)
{
	pos -= off;
}

int Lexer::get_prev_lex()
{
	return prev_yylex;
}

uint32_t Lexer::get_braces_count() const
{
	return braces_count;
}
