%option warn
%option noyywrap
%option nounistd
%option never-interactive
%option batch
%option yylineno
%option c++
%option yyclass = "Lexer"
%option prefix = "mfuse_"
%option debug

%option noyyalloc
%option noyyrealloc
%option noyyfree

%top{
	#include <cstdint>
	#include <cstdlib>
}

%{
	#include <cstdio>
	#include <cstdarg>
	#include <Parser/Lexer.hpp>

	#include <Parser/parsetree.h>
	using namespace mfuse;

	#define YYLEX(n) { yyllocset(loc, 0); prev_yylex = mfuse::Parser::token::n; return mfuse::Parser::token::n; }
	#define YYLEXOFF(n, off) { yyllocset(loc, off); prev_yylex = mfuse::Parser::token::n; return mfuse::Parser::token::n; }

	#define YY_USER_ACTION \
	{ \
		pos += yyleng - yy_more_len; \
		if (loc->lineno != (uint32_t)yylineno) { \
			loc->lineno = yylineno; \
			loc->lineSourcePos = pos; \
		} \
	}

	#define YY_FATAL_ERROR(n) ((Lexer*)this)->yylexerror(n)

	#undef YY_DECL
	#define YY_DECL int mfuse::Lexer::yylex(mfuse::Parser::semantic_type* const lval, Parser::location_type* const loc)

	#undef yyalloc
	#undef yyrealloc
	#undef yyfree

	#define yyalloc(n) mfuse_alloc_c(((Lexer*)this)->parsetree, n)
	#define yyrealloc(ptr, n) mfuse_realloc_c(((Lexer*)this)->parsetree, ptr, n)
	#define yyfree(n) mfuse_free_c(((Lexer*)this)->parsetree, n)

	void* mfuse_alloc_c(ParseTree& parsetree, yy_size_t size);
	void* mfuse_realloc_c(ParseTree& parsetree, void* ptr, yy_size_t size);
	void mfuse_free_c(ParseTree& parsetree, void* ptr);
%}

%x C_COMMENT
%x C_LINE_COMMENT
%x VARIABLES
%x IDENTIFIER

string		([^\\\"\r\n]|\\.)*
identifier	[^\{\}\(\)\[\]\r\n\,:; \t]
alphanum	[a-zA-Z0-9_]+
varname		[a-zA-Z0-9_\"]+

%%

%{
	yylval = lval;
%}

"/*"							{ BEGIN(C_COMMENT); }
<C_COMMENT>"*/"					{ BEGIN(INITIAL); }
<C_COMMENT>[\r\n]				{ ; }
<C_COMMENT>.					{ ; }
"*/"							{ YY_FATAL_ERROR("'*/' found outside of comment"); }


\\[\r\n]+						{ ; }
"//"[^\r\n]*					{ if (prev_yylex != mfuse::Parser::token::TOKEN_EOL) YYLEX(TOKEN_EOL); }

<VARIABLES>"size"					{ BEGIN(INITIAL); YYLEX(TOKEN_SIZE); }
<VARIABLES>[ ]*\./([0-9]?[^0-9[:space:]])	{ YYLEX(TOKEN_PERIOD); }
<VARIABLES>\"{string}\"						{ BEGIN(INITIAL); TextEscapeValue(parsetree, yytext + 1, strlen( yytext ) - 2 ); YYLEX(TOKEN_STRING); }
<VARIABLES>{varname}						{
												TextValue(parsetree, yytext, strlen(yytext));
												YYLEX(TOKEN_IDENTIFIER);
											}
<VARIABLES>[ \t\r\n]						{
												BEGIN(INITIAL);
												unput(yytext[yyleng - 1]);
												yyreducepos(1);
											}
<VARIABLES>.								{
												BEGIN(INITIAL);
												unput(yytext[yyleng - 1]);
												yyreducepos(1);
											}

\"{string}\"					{ TextEscapeValue(parsetree, yytext + 1, strlen( yytext ) - 2 ); YYLEX(TOKEN_STRING); }


"?"								{ YYLEX(TOKEN_TERNARY); }
"if"							{ YYLEX(TOKEN_IF); }
"else"							{ YYLEX(TOKEN_ELSE); }
"while"							{ YYLEX(TOKEN_WHILE); }
"for"							{ YYLEX(TOKEN_FOR); }
"do"							{ YYLEX(TOKEN_DO); }

"game"							{ BEGIN(VARIABLES); yylval->val = parsetree.node1_(method_game); YYLEX(TOKEN_LISTENER); }
"group"							{ BEGIN(VARIABLES); yylval->val = parsetree.node1_(method_group); YYLEX(TOKEN_LISTENER); }
"level"							{ BEGIN(VARIABLES); yylval->val = parsetree.node1_(method_level); YYLEX(TOKEN_LISTENER); }
"local"							{ BEGIN(VARIABLES); yylval->val = parsetree.node1_(method_local); YYLEX(TOKEN_LISTENER); }
"parm"							{ BEGIN(VARIABLES); yylval->val = parsetree.node1_(method_parm); YYLEX(TOKEN_LISTENER); }
"owner"							{ BEGIN(VARIABLES); yylval->val = parsetree.node1_(method_owner); YYLEX(TOKEN_LISTENER); }
"self"							{ BEGIN(VARIABLES); yylval->val = parsetree.node1_(method_self); YYLEX(TOKEN_LISTENER); }

"{"								{ ++braces_count; YYLEX(TOKEN_LEFT_BRACES); }
"}"								{ --braces_count; YYLEX(TOKEN_RIGHT_BRACES); }
"("								{ YYLEX(TOKEN_LEFT_BRACKET); }
")"								{ BEGIN(VARIABLES); YYLEX(TOKEN_RIGHT_BRACKET); }
"["								{ YYLEX(TOKEN_LEFT_SQUARE_BRACKET); }
"]"								{ BEGIN(VARIABLES); YYLEX(TOKEN_RIGHT_SQUARE_BRACKET); }

"="								{ YYLEX(TOKEN_ASSIGNMENT); }
":"								{ YYLEX(TOKEN_COLON); }
"::"							{ YYLEX(TOKEN_DOUBLE_COLON); }
";"								{ YYLEX(TOKEN_SEMICOLON); }

"=="							{ YYLEX(TOKEN_EQUALITY); }
"||"							{ YYLEX(TOKEN_LOGICAL_OR); }
"&&"							{ YYLEX(TOKEN_LOGICAL_AND); }

"|"								{ YYLEX(TOKEN_BITWISE_OR); }
"^"								{ YYLEX(TOKEN_BITWISE_EXCL_OR); }
"&"								{ YYLEX(TOKEN_BITWISE_AND); }
"!="							{ YYLEX(TOKEN_INEQUALITY); }
"<"								{ YYLEX(TOKEN_LESS_THAN); }
">"								{ YYLEX(TOKEN_GREATER_THAN); }
"<="							{ YYLEX(TOKEN_LESS_THAN_OR_EQUAL); }
">="							{ YYLEX(TOKEN_GREATER_THAN_OR_EQUAL); }
[ ]"-"							{ YYLEX(TOKEN_NEG); }

"+"								{ YYLEX(TOKEN_PLUS); }
"+="							{ YYLEX(TOKEN_PLUS_EQUALS); }
"++"|[ ]"++"					{ YYLEX(TOKEN_INCREMENT); }
"-"|"-"[ ]|[ ]"-"[ ]			{ YYLEX(TOKEN_MINUS); }
"-="|[ ]"-="					{ YYLEX(TOKEN_MINUS_EQUALS); }
"--"|[ ]"--"					{ YYLEX(TOKEN_DECREMENT); }
"*"								{ YYLEX(TOKEN_MULTIPLY); }
"*="							{ YYLEX(TOKEN_MULTIPLY_EQUALS); }
"/"								{ YYLEX(TOKEN_DIVIDE); }
"/="							{ YYLEX(TOKEN_DIVIDE_EQUALS); }
"%"								{ YYLEX(TOKEN_MODULUS); }
"%="							{ YYLEX(TOKEN_MODULUS_EQUALS); }
"<<"							{ YYLEX(TOKEN_SHIFT_LEFT); }
"<<="							{ YYLEX(TOKEN_SHIFT_LEFT_EQUALS); }
">>"							{ YYLEX(TOKEN_SHIFT_RIGHT); }
">>="							{ YYLEX(TOKEN_SHIFT_RIGHT_EQUALS); }
"&="							{ YYLEX(TOKEN_AND_EQUALS); }
"^="							{ YYLEX(TOKEN_EXCL_OR_EQUALS); }
"|="							{ YYLEX(TOKEN_OR_EQUALS); }
"$"								{ BEGIN(VARIABLES); YYLEX(TOKEN_DOLLAR); }
"!"								{ YYLEX(TOKEN_NOT); }
"~"								{ YYLEX(TOKEN_COMPLEMENT); }

"."								{ YYLEX(TOKEN_PERIOD); }

"#"								{ YYLEX(TOKEN_NUMBER); }

"NULL"							{ YYLEX(TOKEN_NULL); }
"NIL"							{ YYLEX(TOKEN_NIL); }

"try"							{ YYLEX(TOKEN_TRY); }
"catch"							{ YYLEX(TOKEN_CATCH); }
"switch"						{ YYLEX(TOKEN_SWITCH); }

"case"							{ YYLEX(TOKEN_CASE); }
"break"							{ YYLEX(TOKEN_BREAK); }
"continue"						{ YYLEX(TOKEN_CONTINUE); }

"makearray"|"makeArray"			{ YYLEX(TOKEN_MAKEARRAY); }
"endarray"|"endArray"			{ YYLEX(TOKEN_ENDARRAY); }

[\r\n]*							{ if (prev_yylex != mfuse::Parser::token::TOKEN_EOL) YYLEX(TOKEN_EOL); }
[ \t]							{ ; }

[0-9^`]+								{
											char* p = nullptr;
											yylval->val.longValue = std::strtoll(yytext, &p, 10);
											YYLEX(TOKEN_INTEGER);
										}
[0-9\.]+|[0-9\.]+("e+"|"e-")+[0-9\.]	{
											char* p = nullptr;
											yylval->val.floatValue = std::strtof(yytext, &p);
											YYLEX(TOKEN_FLOAT);
										}

<IDENTIFIER>{identifier}*		{
									BEGIN(INITIAL);
									TextEscapeValue(parsetree, yytext, yyleng);
									YYLEX(TOKEN_IDENTIFIER);
								}
<IDENTIFIER>[ \t\r\n]			{
									BEGIN(INITIAL);
									unput(yytext[yyleng - 1]);
									yyreducepos(1);
									TextEscapeValue(parsetree, yytext, yyleng - 1);
									YYLEXOFF(TOKEN_IDENTIFIER, 1);
								}
<IDENTIFIER>.					{
									BEGIN(INITIAL);
									unput(yytext[yyleng - 1]);
									yyreducepos(1);
									TextEscapeValue(parsetree, yytext, yyleng - 1);
									YYLEXOFF(TOKEN_IDENTIFIER, 1);
								}

{identifier}					{
									BEGIN(IDENTIFIER);
									yymore();
								}

[a-zA-Z0-9_]+					{
									BEGIN(IDENTIFIER);
									yymore();
								}

<<EOF>>							{ YYLEX(TOKEN_EOF); }

.								{ YY_FATAL_ERROR("bad token:\n"); }

%{
%}

%%

void* mfuse_alloc_c(ParseTree& parsetree, yy_size_t size)
{
	return parsetree.alloc(size);
}

void* mfuse_realloc_c(ParseTree& parsetree, void* ptr, yy_size_t size)
{
	void* newmem = parsetree.alloc(size);
	mfuse_free_c(parsetree, ptr);
	return newmem;
}

void mfuse_free_c(ParseTree& parsetree, void* ptr)
{
	return parsetree.free(ptr);
}

void Lexer::TextEscapeValue(ParseTree& parsetree, char* str, size_t len)
{
	char* to = parsetree.alloc(len + 1);

	yylval->val.stringValue = to;

	while (len)
	{
		if (*str == '\\')
		{
			len--;

			if (!len)
				break;

			str++;
			if (*str == 'n')
			{
				*to = '\n';
			}
			else if (*str == 't')
			{
				*to = '\t';
			}
			else if (*str == '"')
			{
				*to = '\"';
			}
			else
			{
				*to = *str;
			}
		}
		else
		{
			*to = *str;
		}

		len--;
		str++;
		to++;
	}

	*to = 0;
}

void Lexer::TextValue(ParseTree& parsetree, char* str, size_t len)
{
	char* s = parsetree.alloc(len + 1);
	strncpy(s, str, len);
	s[len] = 0;
	yylval->val.stringValue = s;
}

bool Lexer::UseField()
{
	return prev_yylex == Parser::token::TOKEN_PERIOD
		|| prev_yylex == Parser::token::TOKEN_DOLLAR;
	/*
	return ( !strncmp( yytext, "game.", 5 ) ||
	!strncmp( yytext, "group.", 6 ) ||
	!strncmp( yytext, "level.", 6 ) ||
	!strncmp( yytext, "local.", 6 ) ||
	!strncmp( yytext, "parm.", 5 ) ||
	!strncmp( yytext, "owner.", 6 ) ||
	!strncmp( yytext, "self.", 5 ) ||
	*yytext == '$' || *yytext == '-' || *yytext == '/' );
	*/
}

void Lexer::yylexerror(const char*)
{
	// FIXME
}
