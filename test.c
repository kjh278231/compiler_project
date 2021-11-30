#include "stdio.h"

#define  HASH_TABLE_SIZE   101
#define ID		1
#define VAR		2
#define CONST	3
#define FUNC	4
#define TYPE	5

/* define for typeclass */
#define INT		1
#define CHAR	2
#define VOID	3
#define ARRAY	4
#define POINTER	5
#define STRUCT	6

typedef struct id {
	char*	name;
	int		tokenType;
} id;

struct ste {
	struct id*		name;
	struct decl*	decl;
	struct ste*		prev;
};

struct decl {
	int				declclass;		// VAR, CONST, FUNC, TYPE
	struct decl*	type;			// VAR, CONST: pointer to its TYPE decl
	int				value;			// CONST: value of integer constant
	float			real_value;		// CONST: value of float constant
	struct ste*		formals;		// FUNC: pointer to formal argument list
	struct decl*	returntype;		// FUNC: pointer to returrn TYPE decl
	int				typeclass;		// TYPE: type class: INT, CHAR, array, ptr, struct, ...
	struct decl*	elementvar;		// TYPE (array) : ptr to element VAR decl
	int				num_index;		// TYPE (array) : number of elements
	struct ste*		fieldlist;		// TYPE (struct) : ptr to field list
	struct decl*	ptrto;			// TYPE (pointer) : type of the pointer
	int				size;			// ALL : size in bytes
	struct ste**	scope;			// VAR : scope when VAR declared
	struct decl*	next;			// for list_of_variables declarations or parameter check of function call
};

/* struct id and hashTable */
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

void print() {
	struct ste* steptr = scope_stack[current];
	int stack_it = current;
	while (steptr != NULL) {
		if (steptr == scope_stack[stack_it]) {
			printf("stack top of %d\n", stack_it);
			stack_it--;
		}
		
		printf("name : %s\n", steptr->name->name);

		switch (steptr->decl->declclass) {
		case(VAR):
			printf("declclass : VAR\n");
			printf("type : %d\n", steptr->decl->type->typeclass);
			break;
		case(CONST):
			printf("declclass : CONST\n");
			printf("type : %d\n", steptr->decl->type->typeclass);
			break;
		case(FUNC):
			printf("declclass : FUNC\n");

			break;
		case(TYPE):
			printf("declclass : TYPE\n");

			break;
		default:
			break;
		}
		
		steptr = steptr->prev;
	}
}

int main(){
	initHash();
	init_type();

	push_scope();
	// char a;
	declare_scope(enter(ID, "a", 1), makevardecl(findcurrentdecl(enter(ID, "char", 4))));

	// int b[10];
	declare_scope(enter(ID, "b", 1), makeconstdecl(makearraydecl(makenumconstdecl(inttype, 10), makevardecl(findcurrentdecl(enter(ID, "int", 3))))));

	// int* c;
	declare_scope(enter(ID, "c", 1), makevardecl(makeptrdecl(findcurrentdecl(enter(ID, "int", 3)))));

	// char* d[5];
	declare_scope(enter(ID, "d", 1), makeconstdecl(makearraydecl(makenumconstdecl(inttype, 5), makevardecl(makeptrdecl(findcurrentdecl(enter(ID, "char", 4)))))));

	// struct temp { int x; int y[20]; } w;
	// STRUCT ID {
	push_scope();
	// def list
	declare_scope(enter(ID, "x", 1), makevardecl(findcurrentdecl(enter(ID, "int", 3))));
	declare_scope(enter(ID, "y", 1), makeconstdecl(makearraydecl(makenumconstdecl(inttype, 20), makevardecl(findcurrentdecl(enter(ID, "int", 3))))));
	// }
	struct ste* fields = pop_scope();
	declare_global(enter(ID, "temp", 4), makestructdecl(fields));
	// type_specifier w ;
	declare_scope(enter(ID, "w", 1), makevardecl(findglobaldecl(enter(ID, "temp", 4))));

	/* 
	int foo(int x, int y){
		int z, w;
	}
	*/
	// type_specifier pointers ID (
	struct decl* funcdecl = makefuncdecl();
	declare_global(enter(ID, "foo", 3), funcdecl);
	push_scope();
	declare_scope(returnid, findglobaldecl(enter(ID, "int", 3)));

	// param_list
	declare_scope(enter(ID, "x", 1), makevardecl(findcurrentdecl(enter(ID, "int", 3))));
	declare_scope(enter(ID, "y", 1), makevardecl(findcurrentdecl(enter(ID, "int", 3))));

	// )
	struct ste* formals = pop_scope();
	funcdecl->returntype = formals->decl;
	funcdecl->formals = formals->prev;

	// {
	push_scope();
	declare_scope(returnid, funcdecl->returntype);
	pushstelist(funcdecl->formals);

	// compound_stmt }
	pop_scope();

	print();
}



/* Implementation - hashTable */
void initHash()
{
	int i;
	nullHash();					// set all elements to null
	/* array of all keywords */
	char* keyWord[] = {
		"auto","break", "case", "char", "continue", "default", "do",
		"else", "extern", "for", "goto", "if", "int", "register",
		"return", "sizeof", "static", "struct", "switch", "typedef",
		"union", "unsigned", "while", "void", "\0"
	};

	for (i = 0; strcmp(keyWord[i], "\0") != 0; i++) {
		enter(ID, keyWord[i], strlen(keyWord[i]));
	}
}

void nullHash()
{	// set all elements to null
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		hashTable[i] = NULL;
	}
}

id* enter(int tokenType, char* name, int length) {
	/* implementation is given here */
	// if name exist in hashTable, return id*
	int i = do_exist(name);
	if (i != -1) {
		return hashTable[i];
	}

	// get an empty node
	i = hash(name);
	// save data in new id
	id* id_ptr = malloc(sizeof(id));
	id_ptr->name = malloc(length + 1);	// +1 for \0
	strcpy(id_ptr->name, name);
	id_ptr->tokenType = tokenType;
	// insert new nlist in hashTable
	hashTable[i] = id_ptr;
	return id_ptr;
}

unsigned int hash(char* name)
{	// find empty node for name
	unsigned int i = 0;
	for (int j = 0; name[j] != '\0'; j++) {
		i += name[j];
	}
	i %= HASH_TABLE_SIZE;
	while (hashTable[i] != NULL) i++;
	return i;
}

int do_exist(char* name)
{	// search if name is already in the hashTable
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		if (hashTable[i] != NULL) {
			if (strcmp(hashTable[i]->name, name) == 0)
				return i;
		}
	}
	return -1;
}

/* Implementation - stack_scope */
void push_scope() {
	current++;
	scope_stack[current] = scope_stack[current - 1];
}

struct ste* pop_scope() {
	// reduce stack
	struct ste* res = scope_stack[current];		// pointer to return linked list
	scope_stack[current] = NULL;
	current--;
	
	// reverse linked list
	struct ste* prev = NULL;
	struct ste* next = res->prev;
	while (res != scope_stack[current]) {	// && res->prev != NULL
		res->prev = prev;
		prev = res;
		res = next;
		next = res->prev;
	}
	return res;
}

void init_type()
{	// initialize symbol table with keywords
	inttype = maketypedecl(INT);
	chartype = maketypedecl(CHAR);
	voidtype = maketypedecl(VOID);
	
	declare_global(enter(ID, "int", 3), inttype);
	declare_global(enter(ID, "char", 4), chartype);
	declare_global(enter(ID, "void", 4), voidtype);
	returnid = enter(ID, "*return", 7);
	
}

void declare_scope(struct id* name, struct decl* decl)
{	// insert new ste at top of scope_stack[current]
	struct ste* new_ste = malloc(sizeof(struct ste));
	new_ste->name = name;
	new_ste->decl = decl;
	new_ste->prev = scope_stack[current];
	scope_stack[current] = new_ste;
}

void declare_global(struct id* name, struct decl* decl)
{	// insert new ste at top of scope_stack[0]
	if (current == 0) {
		declare_scope(name, decl);
	}
	else {
		struct ste* new_ste = malloc(sizeof(struct ste));
		new_ste->name = name;
		new_ste->decl = decl;
		new_ste->prev = scope_stack[0];

		// insert new_ste in top of stack 0
		// search first ste of stack 1 and change prev to new_ste
		struct ste* prev = scope_stack[1];
		while (prev->prev != scope_stack[0]) {
			prev = prev->prev;
		}
		prev->prev = new_ste;
		scope_stack[0] = new_ste;
	}
}

struct decl* findcurrentdecl(struct id* name)
{	// search from the top of scope_stack[current]
	// return NULL if not found
	struct ste* steptr = scope_stack[current];
	while (steptr != NULL && steptr->name != name) {
		steptr = steptr->prev;
	}
	return steptr;
}

struct decl* findglobaldecl(struct id* name)
{	// search from the top of scope_stack[0]
	// return NULL if not found
	struct ste* steptr = scope_stack[0];
	while (steptr != NULL && steptr->name != name) {
		steptr = steptr->prev;
	}
	return steptr;
}

struct decl* findscopedecl(struct id* name)
{	// search for name only in the current scope
	// return NULL if not found
	if(current == 0) return findcurrentdecl(name);
	struct ste* steptr = scope_stack[current];
	while (steptr != NULL && steptr->name != name && steptr != scope_stack[current - 1]) {
		steptr = steptr->prev;
	}
	if(steptr == scope_stack[current - 1]){
		return NULL;
	}
	else return steptr;
}

/* Implementation - makedecl */

struct decl* maketypedecl(int type) {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->declclass = TYPE;
	new_decl->typeclass = type;
	return new_decl;
}

struct decl* makevardecl(struct decl* typeptr) {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->declclass = VAR;
	new_decl->type = typeptr;
	return new_decl;
}

struct decl* makeconstdecl(struct decl* arrayptr) {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->declclass = CONST;
	new_decl->type = arrayptr;
	return new_decl;
}

struct decl* makearraydecl(int size, struct decl* varptr) {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->declclass = TYPE;
	new_decl->typeclass = ARRAY;
	new_decl->elementvar = varptr;
	new_decl->num_index = size;
	return new_decl;
}

struct decl* makenumconstdecl(struct decl* typeptr, int value) {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->type = typeptr;
	new_decl->value = value;
	return new_decl;
}

struct decl* makeptrdecl(struct decl* typeptr) {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->declclass = TYPE;
	new_decl->typeclass = POINTER;
	new_decl->ptrto = typeptr;
	return new_decl;
}

struct decl* makestructdecl(struct ste* fields) {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->declclass = TYPE;
	new_decl->typeclass = STRUCT;
	new_decl->fieldlist = fields;
	return new_decl;
}

struct decl* makefuncdecl() {
	struct decl* new_decl = malloc(sizeof(struct decl));
	new_decl->declclass = FUNC;
	// new_decl->formals, returntype is later assigned
	return new_decl;
}

void pushstelist(struct ste* stelist)
{	// declare_scope all variables in stelist
	struct ste* steptr = stelist;
	while(steptr != NULL){
		declare_scope(steptr->name, steptr->decl);
		steptr = steptr->prev;
	}
}

/* Implementation - checker functions */
void check_is_type(struct decl* typeptr)
{	// check if typeptr points to type decl 
	if(!typeptr->typeclass){
		// error
	}
}

void check_is_struct_type(struct decl* structptr)
{	// check if structptr is struct type decl
	if(structptr == NULL){	// undeclared name
		// error
	}
	else if(structptr->typeclass != STRUCT){	// is declared but not a name for struct
		// error
	}
}

void check_same_type(struct decl* typeptr1, struct decl* typeptr2)
{	// check if typeptr1 and typept2 are same type
	if(typeptr1 != typeptr2){
		// in case of pointer -> always make new ptr decl
		if(typeptr1->typeclass == POINTER && typeptr2->typeclass == POINTER){
			check_same_type(typeptr1->ptrto, typeptr2->ptrto);
		}
		else{
			// error
		}
	}
}

void check_is_func(struct decl* funcptr)
{	// check if funcptr is a pointer of function decl
	if(funcptr->declclass != FUNC){
		// error
	}
}

void check_func_arg(struct decl* funcptr, struct decl* actual)
{	// check parameter types corresponds to each other
	struct ste* formal = funcptr->formals;
	while(formal != NULL && actual != NULL){
		// check_is_var(formal->decl)
		check_same_type(formal->decl, actual->type);
		formal = formal->prev;
		actual = actual->next;
	}
	// number of parmeters do not match
	if(formal != NULL || actual != NULL){
		// error
	}
}

/* not yet implemented */

/* 
TODOs
0. test struct, pointer, function
1. test push_score() and pop_score()

*/