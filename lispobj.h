#ifndef _LISPOBJ_H_
#define _LISPOBJ_H_

#include <stdbool.h>

typedef enum type_id 
{
   SYMBOL, CELL, INTEGER, CHARACTER, SYNTAX, MACRO, 
   PRIM_PROC, LAMBDA, NUM_OF_TYPES
} type_id;

enum lispobj_define
{
   NUM_OF_VALUES = 2
};

typedef struct lispobj
{
      int tid;
      void *value[NUM_OF_VALUES];
} lispobj;

/* cell */
typedef lispobj cell;
void *car(cell *c);
void *cdr(cell *c);
void *set_car(cell *c, void *obj);
void *set_cdr(cell *c, void *obj);
cell *cons(void *l, void *r);
bool is_cell(lispobj *o);
bool equal_cell(cell *l, cell* r);

/* symbol */
typedef lispobj symbol;
symbol *new_symbol(char *name);
bool equal_symbol(symbol *l, symbol *r);
bool is_symbol(lispobj *l);
char *sym_to_string(symbol *s);

/* integer */
typedef lispobj integer;
integer *new_integer(int x);
bool is_integer(integer *i);
bool equal_integer(integer *l, integer *r);
int integer_to_int(integer *i);

/* char */
typedef lispobj character;
character* new_character(char c);
bool is_character(character *c);
bool equal_character(character *l, character *r);
char character_to_char(character *c);

/* list */
typedef lispobj list;
bool is_list(lispobj *obj);
cell *assoc(symbol *s, list *l);
list *append(list *lhs, lispobj *rhs);
int list_length(list *l);

/* string */
typedef list string;
string *new_string(char *s);
bool is_string(lispobj *s);
bool equal_string(string *l, string *r);
char *string_to_chars(string *l);

/* generic equal */
bool equal(lispobj *l, lispobj *r);

/* environment */
typedef list environment;
environment *extend_env(list *vars, list *vals, environment* env);
environment *define_var_val(symbol *var, lispobj *val, environment* env);
environment *set_var_val(symbol *var, lispobj *val, environment* env);
cell *lookup_var_val(symbol *var, environment *env);
environment *new_env();

/* eval */
list *list_of_values(lispobj *exps, environment *env);
lispobj *eval(lispobj *exp, environment *env);

/* primitive procedures */
typedef lispobj prim_proc;
prim_proc *new_prim_proc(lispobj *(*p)(list *));
int is_prim_proc(lispobj *obj);
lispobj *apply_prim_proc(prim_proc *proc, list *arg);
integer *proc_plus_integer(list *integers);
lispobj *prim_car(lispobj *obj);
lispobj *prim_cdr(lispobj *obj);

/* syntax */
typedef lispobj syntax;
syntax *new_syntax(lispobj *(*p)(list *, environment *));
int is_syntax(lispobj* obj);
lispobj *eval_syntax(syntax *s, list *exp, environment *env);
lispobj *syntax_begin(list *exp, environment *env);
lispobj *syntax_define(list *exp, environment *env);
lispobj *syntax_lambda(list *exp, environment *env);

lispobj *syntax_quote(list *operands, environment *env);
lispobj *syntax_unquote_splicing(list *operands, environment *env);
lispobj *syntax_quasiquote(list *operands, environment *env);
lispobj *syntax_unquote(list *operands, environment *env);

/* lambda */
typedef lispobj lambda;
lambda *new_lambda(list *arg_body, environment *env);
lispobj *apply_lambda(lambda *l, list *arg);
bool is_lambda(lispobj *l);

bool char_is_num(char c);
bool string_is_num(char *s);
lispobj *new_lispobj(char *exp);

/* tokenize & parse */
char *new_word(char *head, char *tail);
char *wordtail(char *exp);
list *tokenize(char *exp);
int print_token(list *tokens);
int delete_tokens(list *tokens);
list *read_tokens(list *tokens);
list *get_closer(list *tokens, int count);
list *close_token(list *tokens, int count);
list *read_listtokens(cell *head, cell *tail);
bool print_cell(lispobj* obj, bool is_list_head);
bool print_lispobj(lispobj* obj);
bool print_result(lispobj *obj);
bool print_tokens(list *l);
bool get_current_exp(list *tokens, cell **tail);
list *expand_readmacro(list *tokens);

#endif
