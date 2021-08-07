#pragma once

#if ! defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer mfuse_FlexLexer
#include <FlexLexer.h>
#endif

#include <Parser/yyParser.hpp>
#include <morfuse/Common/str.h>

namespace mfuse
{
	class ParseTree;

	struct parseException_t {
		str text;
		str msg;
		sourceLocation_t loc;
	};

	class Lexer : public yyFlexLexer
	{
	public:
		Lexer(ParseTree& parsetree, std::istream& arg_yyin, std::ostream* arg_yyout = nullptr);
		~Lexer();

		virtual int yylex(Parser::semantic_type* const lval, Parser::location_type* const loc);
		void yylexerror(const char* msg);
		int get_prev_lex();
		uint32_t get_braces_count() const;

		void TextEscapeValue( ParseTree& parsetree, char* str, size_t len);
		void TextValue(ParseTree& parsetree, char* str, size_t len);
		bool UseField(ParseTree& parsetree);

		void yyllocset(Parser::location_type* loc, uint32_t off);
		void yyreducepos(uint32_t off);

	public:
		Parser::semantic_type* yylval;
		parseException_t exception;
		ParseTree& parsetree;
		uint32_t braces_count;
		uint32_t pos;
		int prev_yylex;
	};
}