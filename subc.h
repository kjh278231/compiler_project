/******************************************************
 * File Name   : subc.h
 * Description
 *    This is a header file for the subc program.
 ******************************************************/

#ifndef __SUBC_H__
#define __SUBC_H__

#include <stdio.h>
#include <strings.h>

#define  HASH_TABLE_SIZE   101
#define VAR		1
#define CONST	2
#define FUNC	3
#define TYPE	4

/* define for typeclass */
#define INT		1
#define CHAR	2
#define VOID	3
#define ARRAY	4
#define POINTER	5
#define STRUCT	6

typedef struct id {
	char* name;
	int		tokenType;
} id;

struct ste {
	struct id* name;
	struct decl* decl;
	struct ste* prev;
};

struct decl {
	int				declclass;		// VAR, CONST, FUNC, TYPE
	struct decl* 	type;			// VAR, CONST: pointer to its TYPE decl
	int				value;			// CONST: value of integer constant
//	float			real_value;		// CONST: value of float constant
	struct ste* 	formals;		// FUNC: pointer to formal argument list
	struct decl* 	returntype;		// FUNC: pointer to returrn TYPE decl
	int				typeclass;		// TYPE: type class: INT, CHAR, array, ptr, struct, ...
	struct decl* 	elementvar;		// TYPE (array) : ptr to element VAR decl
	int				num_index;		// TYPE (array) : number of elements
	struct ste* 	fieldlist;		// TYPE (struct) : ptr to field list
	struct decl* 	ptrto;			// TYPE (pointer) : type of the pointer
//	int				size;			// ALL : size in bytes
//	struct ste** 	scope;			// VAR : scope when VAR declared
	struct decl* 	next;			// for list_of_variables declarations or parameter check of function call
};

/* for struct id and hashTable */
static id* hashTable[HASH_TABLE_SIZE];

void initHash();
void nullHash();
id* enter(int tokenType, char* name, int length);
unsigned int hash(char* name);
int do_exist(char* name);

/* stack_scope */
struct ste* scope_stack[128];
int current = 0;

void push_scope();
struct ste* pop_scope();
void init_type();
void declare_scope(struct id* name, struct decl* decl);
void declare_global(struct id* name, struct decl* decl);
struct decl* findcurrentdecl(struct id* name);
struct decl* findglobaldecl(struct id* name);

/* decl */
struct decl* inttype;
struct decl* chartype;
struct decl* voidtype;
struct id* returnid;

struct decl* maketypedecl(int type);
struct decl* makevardecl(struct decl* typeptr);
struct decl* makeconstdecl(struct decl* arrayptr);
struct decl* makearraydecl(int size, struct decl* varptr);
struct decl* makenumconstdecl(struct decl* typeptr, int value);
struct decl* makestructdecl(struct ste* fields);
struct decl* makeptrdecl(struct decl* typeotr);
struct decl* makefuncdecl();
void pushstelist(struct ste* stelist);

/* checker functions */
void check_is_type(struct decl* typeptr);
void check_is_struct_type(struct decl* structptr);
void check_same_type(struct decl* typeptr1, struct decl* typeptr2);
void check_is_func(struct decl* funcptr);
void check_func_arg(struct decl* funcptr, struct decl* actual);

int read_line();

#endif
