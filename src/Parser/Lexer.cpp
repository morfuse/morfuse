#include "Lexer.hpp"

using namespace mfuse;

Lexer::Lexer(ParseTree& inParsetree, std::istream& arg_yyin, std::ostream* arg_yyout)
	: yyFlexLexer(&arg_yyin, arg_yyout)
	, parsetree(inParsetree)
	, braces_count(0)
	, pos(0)
	, eof_reached(false)
{
}

Lexer::~Lexer()
{

}

int Lexer::yylex()
{
	return 0;
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

int Lexer::LexerInput(char* buf, int max_size)
{
	int gcount = yyFlexLexer::LexerInput(buf, max_size);
	if (yyin.eof() && !eof_reached)
	{
		if (gcount < max_size)
		{
			eof_reached = true;

			// append EOF
			if (buf[gcount - 1] == 0)
			{
				buf[gcount - 1] = '\n';
				buf[gcount] = 0;
				++gcount;
			}
		}
	}

	return gcount;
}
