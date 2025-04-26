%require "3.2"
%language "c++"

%defines
%define api.namespace { mfuse }
%define api.parser.class { Parser }
%define api.value.type { stype_t }
%define parse.trace
%define parse.error verbose
%define parse.assert true

%locations
%define api.location.type { mfuse::location }

%parse-param { mfuse::Lexer& lexer } { mfuse::ParseTree& pt }

%code requires
{
    #include <Parser/parsetree.h>
    #include <Parser/Location.hpp>

    using yyscan_t = void*;

    namespace mfuse {
        class Lexer;
    
        struct stype_t {
            sval_t val;
        };
    }
}

%code top
{
    #include "yyParser.hpp"
    #include <Parser/Lexer.hpp>
    #include <morfuse/Script/ScriptOpcodes.h>
    using namespace mfuse;

    #define YYLLOC pt.node_pos(pt.pos - lexer.YYLeng())
    #define TOKPOS(n) pt.node_pos(n)
    #undef yylex
    #define yylex lexer.yylex
}

%precedence TOKEN_EOF 0 "end of file"
%precedence TOKEN_EOL

%precedence TOKEN_IF
%right THEN TOKEN_ELSE
%precedence TOKEN_WHILE TOKEN_FOR TOKEN_DO
%precedence <val.stringValue> TOKEN_IDENTIFIER

%precedence TOKEN_LEFT_BRACES TOKEN_RIGHT_BRACES
%left TOKEN_LEFT_BRACKET TOKEN_RIGHT_BRACKET
%token TOKEN_LEFT_SQUARE_BRACKET TOKEN_RIGHT_SQUARE_BRACKET

%right  TOKEN_ASSIGNMENT
        TOKEN_PLUS_EQUALS TOKEN_MINUS_EQUALS TOKEN_MULTIPLY_EQUALS TOKEN_DIVIDE_EQUALS TOKEN_MODULUS_EQUALS
        TOKEN_AND_EQUALS TOKEN_EXCL_OR_EQUALS TOKEN_OR_EQUALS
        TOKEN_SHIFT_LEFT_EQUALS TOKEN_SHIFT_RIGHT_EQUALS
        TOKEN_TERNARY TOKEN_COLON

%left TOKEN_LOGICAL_OR
%left TOKEN_LOGICAL_AND

%left TOKEN_BITWISE_OR
%left TOKEN_BITWISE_EXCL_OR
%left TOKEN_BITWISE_AND

%left TOKEN_EQUALITY TOKEN_INEQUALITY
%left TOKEN_LESS_THAN TOKEN_LESS_THAN_OR_EQUAL TOKEN_GREATER_THAN TOKEN_GREATER_THAN_OR_EQUAL

%left TOKEN_SHIFT_LEFT TOKEN_SHIFT_RIGHT
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_MODULUS

%token TOKEN_NEG TOKEN_NOT TOKEN_COMPLEMENT

%precedence <val.stringValue> TOKEN_STRING
%precedence <val.longValue> TOKEN_INTEGER
%precedence <val.floatValue> TOKEN_FLOAT

%precedence <val> TOKEN_LISTENER
%precedence TOKEN_NIL TOKEN_NULL

%left TOKEN_DOUBLE_COLON
%left TOKEN_SEMICOLON
%right TOKEN_DOLLAR

%token TOKEN_INCREMENT TOKEN_DECREMENT TOKEN_PERIOD

%right TOKEN_INCREMENT TOKEN_DECREMENT TOKEN_NEG TOKEN_NOT TOKEN_COMPLEMENT
%left TOKEN_LEFT_SQUARE_BRACKET TOKEN_RIGHT_SQUARE_BRACKET TOKEN_PERIOD

%precedence UNARY

%precedence     TOKEN_CATCH TOKEN_TRY
                TOKEN_SWITCH TOKEN_CASE
                TOKEN_BREAK TOKEN_CONTINUE
                TOKEN_SIZE
                TOKEN_END TOKEN_RETURN
                TOKEN_MAKEARRAY TOKEN_ENDARRAY

%type <val> event_parameter_list event_parameter_list_need event_parameter
%type <val> statement_list statement statement_declaration makearray_statement_list makearray_statement statement_for_condition
%type <val> compound_statement selection_statement iteration_statement
%type <val> expr
%type <val> func_prim_expr
%type <val> prim_expr
%type <val> listener_identifier
%type <val> nonident_prim_expr
%type <val> nonident_prim_expr_base
%type <val> const_array_list
%type <val> const_array
%type <val.stringValue> identifier_prim
%type <val.stringValue> identifier

%start program

%%

program
    : statement_list[list] { pt.rootVal = pt.node1(statementType_e::StatementList, $list); }
    | line_opt { pt.rootVal = pt.node0(statementType_e::None); }
    ;

statement_list
    : statement { $$ = pt.linked_list_end($1); }
    | statement_list statement[stmt] { $$ = pt.append_node($1, $stmt); }
    ;

statement
    : line_opt statement_declaration[stmt_decl] line_opt { $$ = $stmt_decl; }
    ;

statement_declaration
    : TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON { $$ = pt.node3(statementType_e::Labeled, $1, $2, TOKPOS(@1)); }
    | TOKEN_PLUS TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON { $$ = pt.node3(statementType_e::Labeled, $2, $3, TOKPOS(@2)); }
    | TOKEN_MINUS TOKEN_IDENTIFIER event_parameter_list TOKEN_COLON { $$ = pt.node3(statementType_e::PrivateLabeled, $2, $3, TOKPOS(@2)); }
    | TOKEN_CASE prim_expr event_parameter_list TOKEN_COLON { $$ = pt.node3(statementType_e::IntLabeled, $2, $3, TOKPOS(@1)); }
    | compound_statement
    | selection_statement
    | iteration_statement
    | TOKEN_TRY compound_statement[C1] TOKEN_CATCH compound_statement[C2] { $$ = pt.node3(statementType_e::Try, $C1, $C2, TOKPOS(@1)); }
    | TOKEN_BREAK { $$ = pt.node1(statementType_e::Break, TOKPOS(@1)); }
    | TOKEN_CONTINUE { $$ = pt.node1(statementType_e::Continue, TOKPOS(@1)); }
    | TOKEN_IDENTIFIER event_parameter_list { $$ = pt.node3(statementType_e::CmdEvent, $1, $2, TOKPOS(@1)); }
    | nonident_prim_expr TOKEN_IDENTIFIER event_parameter_list { $$ = pt.node4(statementType_e::MethodEvent, $1, $2, $3, TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_ASSIGNMENT expr { $$ = pt.node3(statementType_e::Assignment, $1, $3, TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_PLUS_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_PLUS), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_MINUS_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_MINUS), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_MULTIPLY_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_MULTIPLY), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_DIVIDE_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_DIVIDE), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_MODULUS_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_PERCENTAGE), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_AND_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_BITWISE_AND), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_EXCL_OR_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_BITWISE_EXCL_OR), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_OR_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_BITWISE_OR), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_SHIFT_LEFT_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_SHIFT_LEFT), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_SHIFT_RIGHT_EQUALS expr { $$ = pt.node3(statementType_e::Assignment, $1, pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_SHIFT_RIGHT), $1, $3, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_INCREMENT { $$ = pt.node3(statementType_e::Assignment, $1, pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_INC), $1, TOKPOS(@2)), TOKPOS(@2)); }
    | nonident_prim_expr TOKEN_DECREMENT { $$ = pt.node3(statementType_e::Assignment, $1, pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_DEC), $1, TOKPOS(@2)), TOKPOS(@2)); }
    | TOKEN_SEMICOLON { $$ = pt.node0(statementType_e::None); }
    //| TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON TOKEN_IDENTIFIER event_parameter_list { $$ = pt.node3( statementType_e::MethodEvent, pt.node_string( parsetree_string( str( $1.stringValue ) + "::" + $3.stringValue ) ), pt.node1( statementType_e::none, $4 ), TOKPOS(@1) ); }
    //| nonident_prim_expr TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON TOKEN_IDENTIFIER event_parameter_list { $$ = pt.node4( statementType_e::MethodEvent, $1, pt.node_string( parsetree_string( str( $2.stringValue ) + "::" + $4.stringValue ) ), pt.node1( statementType_e::none, $5 ), TOKPOS(@2) ); }
    ;

statement_for_condition
    : line_opt statement_declaration[stmt_decl] line_opt { $$ = $stmt_decl; }
    | line_opt statement_declaration[stmt_decl] TOKEN_SEMICOLON line_opt { $$ = $stmt_decl; }
    ;

compound_statement
    : TOKEN_LEFT_BRACES statement_list TOKEN_RIGHT_BRACES { $$ = pt.node1(statementType_e::StatementList, $2); }
    | TOKEN_LEFT_BRACES line_opt TOKEN_RIGHT_BRACES { $$ = pt.node0(statementType_e::None); }
    | line_opt compound_statement[comp_stmt] line_opt { $$ = $comp_stmt; }
    ;
    
selection_statement
    : TOKEN_IF prim_expr[exp] statement_for_condition[stmt] %prec THEN { $$ = pt.node3(statementType_e::If, $exp, $stmt, TOKPOS(@1)); }
    | TOKEN_IF prim_expr[exp] statement_for_condition[if_stmt] TOKEN_ELSE statement_for_condition[else_stmt] { $$ = pt.node4(statementType_e::IfElse, $exp, $if_stmt, $else_stmt, TOKPOS(@1)); }
    | TOKEN_SWITCH prim_expr[exp] compound_statement[comp_stmt] { $$ = pt.node3(statementType_e::Switch, $exp, $comp_stmt, TOKPOS(@1)); }
    ;

iteration_statement
    : TOKEN_WHILE prim_expr[exp] statement_for_condition[stmt]{ $$ = pt.node4(statementType_e::While, $exp, $stmt, pt.node0(statementType_e::None), TOKPOS(@1)); }
    | TOKEN_FOR TOKEN_LEFT_BRACKET statement[init_stmt] TOKEN_SEMICOLON expr[exp] TOKEN_SEMICOLON statement_list[inc_stmt] TOKEN_RIGHT_BRACKET statement_for_condition[stmt]
    {
        sval_t while_stmt = pt.node4(statementType_e::While, $exp, $stmt, pt.node1(statementType_e::StatementList, $inc_stmt), TOKPOS(@1));
        $$ = pt.node1(statementType_e::StatementList, pt.append_node(pt.linked_list_end($init_stmt), while_stmt));
    }
    | TOKEN_FOR TOKEN_LEFT_BRACKET TOKEN_SEMICOLON expr[exp] TOKEN_SEMICOLON statement_list[inc_stmt] TOKEN_RIGHT_BRACKET statement_for_condition[stmt]
    {
        $$ = pt.node4(statementType_e::While, $exp, $stmt, pt.node1(statementType_e::StatementList, $inc_stmt), TOKPOS(@1));
    }
    | TOKEN_DO statement_for_condition[stmt] TOKEN_WHILE prim_expr[exp]{ $$ = pt.node3(statementType_e::Do, $stmt, $exp, TOKPOS(@1)); }
    ;


expr
    : expr[left] TOKEN_LOGICAL_OR[tok] expr[right] { $$ = pt.node3(statementType_e::LogicalOr, $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_LOGICAL_AND[tok] expr[right] { $$ = pt.node3(statementType_e::LogicalAnd, $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_BITWISE_OR[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_BITWISE_OR), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_BITWISE_EXCL_OR[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_BITWISE_EXCL_OR), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_BITWISE_AND[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_BITWISE_AND), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_EQUALITY[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_EQUALITY), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_INEQUALITY[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_INEQUALITY), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_LESS_THAN[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_LESS_THAN), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_GREATER_THAN[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_GREATER_THAN), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_LESS_THAN_OR_EQUAL[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_LESS_THAN_OR_EQUAL), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_GREATER_THAN_OR_EQUAL[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_GREATER_THAN_OR_EQUAL), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_SHIFT_LEFT[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_SHIFT_LEFT), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_SHIFT_RIGHT[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_SHIFT_RIGHT), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_PLUS[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_PLUS), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_MINUS[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_MINUS), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_MULTIPLY[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_MULTIPLY), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_DIVIDE[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_DIVIDE), $left, $right, TOKPOS(@tok)); }
    | expr[left] TOKEN_MODULUS[tok] expr[right] { $$ = pt.node4(statementType_e::Func2Expr, pt.node1b(OP_BIN_PERCENTAGE), $left, $right, TOKPOS(@tok)); }
    | TOKEN_EOL expr[exp] { $$ = $exp; }
    | nonident_prim_expr
    | func_prim_expr
    | TOKEN_IDENTIFIER { $$ = pt.node2(statementType_e::String, $1, TOKPOS(@1)); }
    ;

func_prim_expr
    : TOKEN_IDENTIFIER event_parameter_list_need { $$ = pt.node3(statementType_e::CmdEventExpr, $1, $2, TOKPOS(@1)); }
    | nonident_prim_expr_base TOKEN_IDENTIFIER event_parameter_list { $$ = pt.node4(statementType_e::MethodEventExpr, $1, $2, $3, TOKPOS(@2)); }
    | TOKEN_NEG func_prim_expr { $$ = pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_MINUS), $2, TOKPOS(@1)); }
    | TOKEN_COMPLEMENT func_prim_expr { $$ = pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_COMPLEMENT), $2, TOKPOS(@1)); }
    | TOKEN_NOT func_prim_expr { $$ = pt.node2(statementType_e::BoolNot, $2, TOKPOS(@1)); }
    | TOKEN_IDENTIFIER TOKEN_DOUBLE_COLON const_array_list
        {
            $$ = pt.node3(statementType_e::ConstArrayExpr, pt.node2(statementType_e::String, $1, TOKPOS(@1)), $3, TOKPOS(@2));
        }
    | nonident_prim_expr TOKEN_DOUBLE_COLON const_array_list
        {
            $$ = pt.node3(statementType_e::ConstArrayExpr, $1, $3, TOKPOS(@2));
        }
    | TOKEN_MAKEARRAY makearray_statement_list[stmt] TOKEN_ENDARRAY
        {
            $$ = pt.node2(statementType_e::MakeArray, $stmt, TOKPOS(@1));
        }
    ;

event_parameter_list
    : { $$ = nullptr; }
    | event_parameter { $$ = $1; }
    ;

event_parameter_list_need
    : event_parameter { $$ = $1; }
    ;

event_parameter
    : prim_expr { $$ = pt.linked_list_end($1); }
    | event_parameter prim_expr { $$ = pt.append_node($1, $2); }
    ;

const_array_list
    : const_array { $$ = pt.linked_list_end($1); }
    | const_array_list TOKEN_DOUBLE_COLON const_array { $$ = pt.append_node($1, $3); }
    ;

const_array
    : nonident_prim_expr { $$ = $1; }
    | identifier_prim { $$ = pt.node2(statementType_e::String, $1, TOKPOS(@1)); }
    ;

prim_expr
    : nonident_prim_expr { $$ = $1; }
    | identifier_prim { $$ = pt.node2(statementType_e::String, $1, TOKPOS(@1)); }
    | const_array TOKEN_DOUBLE_COLON const_array_list
        {
            $$ = pt.node3(statementType_e::ConstArrayExpr, $1, $3, TOKPOS(@2));
        }
    ;

identifier_prim:
    TOKEN_IDENTIFIER { $$ = $1; @$ = @1; }
    ;

identifier
    : TOKEN_IDENTIFIER { $$ = $1; @$ = @1; }
    | TOKEN_STRING { $$ = $1; @$ = @1; }
    ;

listener_identifier
    : identifier { $$ = pt.node2(statementType_e::String, $1, TOKPOS(@1)); }
    | TOKEN_LEFT_BRACKET expr TOKEN_RIGHT_BRACKET { $$ = $2; }
    ;

nonident_prim_expr
    : nonident_prim_expr_base
    | TOKEN_NEG nonident_prim_expr { $$ = pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_MINUS), $2, TOKPOS(@1)); }
    | TOKEN_COMPLEMENT nonident_prim_expr { $$ = pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_COMPLEMENT), $2, TOKPOS(@1)); }
    | TOKEN_NOT nonident_prim_expr { $$ = pt.node2(statementType_e::BoolNot, $2, TOKPOS(@1)); }
    ;

nonident_prim_expr_base
    : TOKEN_DOLLAR listener_identifier { $$ = pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_TARGETNAME), $2, TOKPOS(@1)); }
    | nonident_prim_expr_base TOKEN_PERIOD identifier { $$ = pt.node3(statementType_e::Field, $1, $3, TOKPOS(@3)); }
    | nonident_prim_expr_base TOKEN_PERIOD TOKEN_SIZE { $$ = pt.node3(statementType_e::Func1Expr, pt.node1b(OP_UN_SIZE), $1, TOKPOS(@3)); }
    | nonident_prim_expr_base TOKEN_LEFT_SQUARE_BRACKET expr TOKEN_RIGHT_SQUARE_BRACKET { $$ = pt.node3(statementType_e::ArrayExpr, $1, $3, TOKPOS(@2)); }
    | TOKEN_STRING { $$ = pt.node2(statementType_e::String, $1, TOKPOS(@1)); }
    | TOKEN_INTEGER { $$ = pt.node2(statementType_e::Integer, $1, TOKPOS(@1)); }
    | TOKEN_FLOAT { $$ = pt.node2(statementType_e::Float, $1, TOKPOS(@1)); }
    | TOKEN_LEFT_BRACKET expr[exp1] expr[exp2] expr[exp3] TOKEN_RIGHT_BRACKET { $$ = pt.node4(statementType_e::Vector, $exp1, $exp2, $exp3, TOKPOS(@1)); }
    | TOKEN_LISTENER { $$ = pt.node2(statementType_e::Listener, $1, TOKPOS(@1)); }
    | TOKEN_LEFT_BRACKET expr TOKEN_RIGHT_BRACKET { $$ = $2; }
    | TOKEN_LEFT_BRACKET expr TOKEN_EOL TOKEN_RIGHT_BRACKET { $$ = $2; }
    | TOKEN_NULL { $$ = pt.node1(statementType_e::NULLPTR, TOKPOS(@1)); }
    | TOKEN_NIL { $$ = pt.node1(statementType_e::NIL, TOKPOS(@1)); }
    ;

makearray_statement_list:
    { $$ = pt.node0(statementType_e::None); }
    | makearray_statement_list[list] makearray_statement[ma_stmt] TOKEN_EOL { $$ = pt.append_node($list, pt.node2(statementType_e::MakeArray, $ma_stmt, TOKPOS(@ma_stmt))); }
    | makearray_statement[ma_stmt] TOKEN_EOL { $$ = pt.linked_list_end(pt.node2(statementType_e::MakeArray, $ma_stmt, TOKPOS(@ma_stmt))); }
    | TOKEN_EOL makearray_statement_list { $$ = $2; @$ = @2; }
    ;

makearray_statement
    : prim_expr { $$ = pt.linked_list_end($1); }
    | makearray_statement prim_expr { $$ = pt.append_node($1, $2); }
    ;

line_opt
    : {}
    | TOKEN_EOL
    ;

%%
