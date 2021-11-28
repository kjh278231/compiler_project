%{
/*
 * File Name   : subc.y
 * Description : a skeleton bison input
 */

#include "subc.h"
#include "subc.tab.h"

int    yylex ();
int    yyerror (char* s);

%}

/* yylval types */
%union {
	int		intVal;
	char	*stringVal;
}

/* Precedences and Associativities */
%left   ','
%right  '='
%left   LOGICAL_OR
%left   LOGICAL_AND
%left   '&'
%left   EQUOP
%left   RELOP
%left   '+' '-'
%left   '*' '/' '%'
%right  '!' INCOP DECOP
%left   STRUCTOP '(' ')' '[' ']' '.'

%nonassoc IFSP
%nonassoc ELSE

/* Token and Types */
%token TYPE STRUCT RETURN IF ELSE WHILE FOR BREAK CONTINUE LOGICAL_OR LOGICAL_AND INCOP DECOP STRUCTOP VOID STRING CHAR_CONST ID INTEGER_CONST NULL

%%
program
    : ext_def_list
    ;

ext_def_list
		: ext_def_list ext_def
		| /* empty */
		;

ext_def
		: type_specifier pointers ID ';'
		| type_specifier pointers ID '[' const_expr ']' ';'
		| func_decl ';'
		| type_specifier ';'
		| func_decl
		{
			push_scope();
			declare_scop(returnid, $1->returntype);		// make returnid for checking return type
			pushstelist($1->formals);
		}
		compound_stmt
		{
			pop_scope();
		}

type_specifier
		: TYPE
		{
			struct decl* typeptr = findglobaldecl(yytext);
			check_is_type(typeptr);
			$$ = typeptr;
		}
		| VOID
		| struct_specifier

struct_specifier
		: STRUCT ID '{' 
		{ push_scope(); }
		def_list 
		{
			struct ste* fields = pop_scope();
			declare_global($2, ($$ = makestructdecl(fields)));
		}
		'}'
		| STRUCT ID
		{
			struct decl* structptr = findglobaldecl($2);
			check_is_struct_type(structptr);
			$$ = structptr;
		}

func_decl
		: type_specifier pointers ID '(' ')'
		| type_specifier pointers ID '(' VOID ')'
		| type_specifier pointers ID '(' 
		{
			struct decl* funcdecl = makefuncdecl();
			declare_global($3, funcdecl);
			push_scope();
			if($2){
				declare_scope(returnid, makeptrdecl($1));
			}
			else{
				declare_scope(returnid, $1);
			}
			$<declptr>$ = funcdecl;		// becomes $5
		}
		param_list 
		')'
		{
			struct ste* formals;
			decl* funcdecl = $<declptr>5;
			formals = pop_scope;
			funcdecl->returntype = formals->decl;
			funcdecl->formals = formals->prev;		// formals point returnid
			$$ = funcdecl;
		}

pointers
		: '*'
		{
			// return 1;
			// $$ = 1;
		}
		| /* empty */
		{
			// return 0;
			// $$ = 0;
		}

param_list  /* list of formal parameter declaration */
		: param_decl
		| param_list ',' param_decl

param_decl  /* formal parameter declaration */
		: type_specifier pointers ID
		{
			if($2){
				declare_scope($3, makevardecl(makeptrdecl($1)));
			}
			else{
				declare_scope($3, makevardecl($1));
			}
		}
		| type_specifier pointers ID '[' const_expr ']'
		{
			if($2){
				declare_scope($3, makeconstdecl(makearraydecl($5, makevardecl($1))));
			}
			else{
				declare_scope($3, makeconstdecl(makearraydecl($5, makevardecl(makeptrdecl($1)))));
			}
		}

def_list    /* list of definitions, definition can be type(struct), variable, function */
		: def_list def
		| /* empty */

def
		: type_specifier pointers ID ';'
		{
			if($2){
				declare_scope($3, makevardecl(makeptrdecl($1)));
			}
			else{
				declare_scope($3, makevardecl($1));
			}
		}
		| type_specifier pointers ID '[' const_expr ']' ';'
		{
			if($2){
				declare_scope($3, makeconstdecl(makearraydecl($5, makevardecl($1))));
			}
			else{
				declare_scope($3, makeconstdecl(makearraydecl($5, makevardecl(makeptrdecl($1)))));
			}
		}
		| type_specifier ';'
		| func_decl ';'

compound_stmt
		: '{' local_defs stmt_list '}'

local_defs  /* local definitions, of which scope is only inside of compound statement */
		:	def_list

stmt_list
		: stmt_list stmt
		| /* empty */

stmt
		: expr ';'
		| compound_stmt
		| RETURN ';'
		| RETURN expr ';'
		{
			// check return type
			check_same_type(findcurrentdecl(returnid), $2);		// expr should return decl* typeptr
		}
		| ';'
		| IF '(' expr ')' stmt %prec IFSP
		| IF '(' expr ')' stmt ELSE stmt
		| WHILE '(' expr ')' stmt
		| FOR '(' expr_e ';' expr_e ';' expr_e ')' stmt
		| BREAK ';'
		| CONTINUE ';'

expr_e
		: expr
		| /* empty */

const_expr
		: expr

expr
		: unary '=' expr
		| or_expr

or_expr
		: or_list

or_list
		: or_list LOGICAL_OR and_expr
		| and_expr

and_expr
		: and_list

and_list
		: and_list LOGICAL_AND binary
		| binary

binary
		: binary RELOP binary
		| binary EQUOP binary
		| binary '+' binary
		| binary '-' binary
		| unary %prec '='

unary
		: '(' expr ')'
		| '(' unary ')' 
		| INTEGER_CONST
		| CHAR_CONST
		| STRING
		| ID
		| '-' unary	%prec '!'
		| '!' unary
		| unary INCOP
		| unary DECOP
		| INCOP unary
		| DECOP unary
		| '&' unary	%prec '!'
		| '*' unary	%prec '!'
		| unary '[' expr ']'
		| unary '.' ID
		| unary STRUCTOP ID
		| unary '(' args ')'
		| unary '(' ')'

args    /* actual parameters(function arguments) transferred to function */
		: expr
		| args ',' expr
    /*fill in here*/
    
%%

/*  Additional C Codes here */

int    yyerror (char* s)
{
	fprintf (stderr, "%s\n", s);
}

