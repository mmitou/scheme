
#include "lispobj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum type_id 
{
   SYMBOL, CELL, INTEGER, CHARACTER, BOOLEAN, 
   SYNTAX, MACRO, PRIM_PROC, LAMBDA,  NUM_OF_TYPES
} type_id;

/* lower index is higher priority */
enum SPCL_CHRS {UNQUOTE_SPLICING, QUASIQUOTE, QUOTE, UNQUOTE, NUM_OF_SPCIL_CHRS };
char* special_chars[] = {",@", "`", "'", ","};
char* readmacro_symbols[] = {"unquote-splicing", "quasiquote", "quote", "unquote"};
char* brackets_chars[] = {"(", ")"};

/* generic equal */
bool (*equalf_pointers[NUM_OF_TYPES])(lispobj *, lispobj *)
= {equal_symbol, equal_cell, equal_integer, equal_character,
   equal_boolean, NULL};


/* cell */
/*@out@*/
static void *get_val(cell *c, int i)
{
   if(c == NULL)
   {
      fprintf(stderr, "get_val(): cell is NULL\n");
      abort();
   }
   if(!(0 <= i &&i < NUM_OF_VALUES))
   {
      fprintf(stderr, "get_val(): index error\n");
      abort();
   }
   return c->value[i];   
}

/*@null@*/
static void *set_val(cell *c, int i, lispobj *obj)
{
   void *result = NULL;
   result = c->value[i];
   c->value[i] = obj;
   return result;
}

/*@null@*/
void *car(cell *c)
{
   return get_val(c, 0);
}

/*@null@*/
void *cdr(cell *c)
{
   return get_val(c, 1);
}

/*@null@*/
void *set_car(cell *c, void *obj)
{
   return set_val(c, 0, obj);
}

/*@null@*/
void *set_cdr(cell *c, void *obj)
{
   return set_val(c, 1, obj);
}

/*@null@*/
cell *cons(void *l, void *r)
{
   cell *c = (cell *)malloc(sizeof(cell));
   c->tid = CELL;
   set_car(c, l);
   set_cdr(c, r);
   return c;
}

bool is_cell(lispobj *obj)
{
   return obj == NULL ? 0 :obj->tid == CELL;
}

bool equal_cell(cell *l, cell* r)
{
   bool result = false;
   if(is_cell(l) && is_cell(r))
   {
      result = 
         (generic_equal(car(l), car(r)) && 
         generic_equal(cdr(l), cdr(r)));
   }
   return result;
}

/* symbol */
symbol *new_symbol(char *name)
{
   symbol *s = (symbol *)malloc(sizeof(symbol));
   char *symbol_name;
   int name_size;
   char *p;
   s->tid = SYMBOL;
   for(name_size = 1, p = name;  *p != '\0';
       name_size++, p++);
   symbol_name = (char *)malloc(name_size);
   strcpy(symbol_name, name);
   set_car(s, symbol_name);
   return s;
}

bool is_symbol(lispobj *l)
{
   return l == NULL ? 0 : l->tid == SYMBOL;
}

bool equal_symbol(symbol *l, symbol *r)
{
   bool result = 0;
   if(is_symbol(l) && is_symbol(r))
   {
      result = 
         strcmp(sym_to_string(l), sym_to_string(r)) == 0;
   }
   return result;
}

/*@null@*/
char *sym_to_string(symbol *s)
{
   return (char *)(car(s));
}

/* integer */
/*@null@*/
integer *new_integer(int x)
{
   integer *i = (integer*)malloc(sizeof(integer));
   int *p = (int *)malloc(sizeof(int));
   i->tid = INTEGER;
   *p = x;
   set_car(i, p);
   return i;
}

bool is_integer(integer *i)
{
   return i == NULL ? 0 : i->tid == INTEGER;
}

int integer_to_int(integer *i)
{
   return *(int *)(car(i));
}

bool equal_integer(integer *l, integer *r)
{
   bool result = 0;
   if(is_integer(l) && is_integer(r))
   {
      result = integer_to_int(l) == integer_to_int(r);
   }
   return result;
}

/* char */
/*@null@*/
character *new_character(char c)
{
   character *chrctr = (character*)malloc(sizeof(character));
   char *d = (char *)malloc(sizeof(char));
   chrctr->tid = CHARACTER;
   *d = c;
   set_car(chrctr, d);
   set_cdr(chrctr, NULL);
   return chrctr;
}

bool is_character(character *c)
{
   return c->tid == CHARACTER;
}

bool equal_character(character *l, character *r)
{
   return (is_character(l) && is_character(r)) && 
      ((*(char*)(l->value[0])) == (*(char*)(r->value[0])));
}

char character_to_char(character *c)
{
   return *(char*)(c->value[0]);
}

/* list */

bool is_list(lispobj *obj)
{
   bool result = false;
   if(obj == NULL)
   {
      result = true;
   }
   else if(is_cell(obj))
   {
      result = is_list(cdr(obj));
   }
   return result;
}

/*@null@*/
cell *assoc(symbol *s, list *l)
{
   cell *result = NULL;
   cell *pair = NULL;
   if(l != NULL && is_list(l))
   {
      pair = car(l);
      if(generic_equal(s, car(pair)))
      {
         result = pair;
      }
      else
      {
         result = assoc(s, cdr(l));
      }
   }
   return result;
}

cell *last_cell(list *l)
{
   if(l == NULL)
   {
      return NULL;
   }
   else if(cdr(l) == NULL)
   {
      return l;
   }
   else
   {
      return last_cell(cdr(l));
   }
}

lispobj *append(list *lhs, lispobj *rhs)
{
   if(lhs == NULL)
   {
      return rhs;
   }
   else if(is_list(lhs))
   {
      cell *lc = last_cell(lhs);
      if(lc != NULL)
      {
         set_cdr(lc, rhs);
         return lhs;
      }
   }

   fprintf(stderr,"append error\n");
   abort();
   return NULL;
}

int list_length(list *l)
{
   return l == NULL ? 0 : 1 + list_length(cdr(l));
}

/* string */
/*@null@*/
string *new_string(char *s)
{
   char *i = s;
   character *c = NULL;
   character *d = NULL;
   string *result = NULL;

   if(*i != '\0')
   {
      result = new_character(*i);
      c = result;
      i++;
      while(*i != '\0')
      {
         d = new_character(*i);
         set_cdr(c, d);
         c = d;
         ++i;
      }
   }
   
   return result;
}

bool is_string(lispobj *s)
{
   bool result = false;
   if(s == NULL)
   {
      result = true;
   }
   else if(is_character(s))
   {
      result = is_string(cdr(s));
   }
   else
   {
      result = false;
   }
   return result;
}

bool equal_string(string *l, string *r)
{
   bool result = false;
   if(l == NULL && r == NULL)
   {
      result = true;
   }
   else if( l != NULL && r != NULL)
   {
      if(equal_character(l, r))
      {
         result = equal_string(cdr(l), cdr(r));
      }
   }
   return result;
}

/*@null@*/
char *string_to_chars(string *l)
{
   int len = list_length(l);
   char *result = (char*)malloc(sizeof(char)*len + 1);
   char *i = result;
   character *c = l;

   while(c != NULL)
   {
      *i = *(char*)(c->value[0]);
      ++i;
      c = c->value[1];
   }
   *i = '\0';
   return result;
}

bool generic_equal(lispobj *l, lispobj *r)
{
   if(l == r)
   {
      return true;
   }
   else if(l == NULL || r == NULL)
   {
      return false;
   }
   else if(l->tid == r->tid)
   {
      return equalf_pointers[l->tid](l, r);
   }
   else
   {
      return false;
   }
}

/* environment */
/*@null@*/
static list *new_frame(lispobj *vars, list *vals)
{
   list *result = NULL;

   if(vars == NULL || vals == NULL)
   {
      result = NULL;
   }
   else if(is_symbol(vars))
   {
      result = 
         cons(cons(vars, vals), NULL);
   }
   else
   {
      lispobj *var = car(vars);
      lispobj *val = car(vals);

      result = 
         cons(
            cons(var, val),
            new_frame(cdr(vars), cdr(vals)));
   }

   return result;
}

/*@null@*/
environment *extend_env(list *vars, list *vals, environment *env)
{
   return cons(new_frame(vars, vals), env);
}

/*@null@*/
environment *define_var_val(symbol *var, lispobj *val, environment *env)
{
   environment *result = NULL;
   cell *c = NULL;
   if(env == NULL)
   {
      result = 
         cons(new_frame(cons(var, NULL), 
         cons(val, NULL)), env);
   }
   else
   {
      c = cons(var, val);
      set_car(env, cons(c, car(env)));
   }
   return env;
}

/*@null@*/
cell *lookup_var_val(symbol *var, environment *env)
{
   cell *result = NULL;
   if(env != NULL)
   {
      result = assoc(var, car(env));
      if(result == NULL)
      {
         result = lookup_var_val(var, cdr(env));
      }
   }
   return result;
}

/*@null@*/
environment *set_var_val(
   symbol *var, lispobj *val, environment* env)
{
   cell *c = lookup_var_val(var, env);
   if(c != NULL)
   {
      set_cdr(c, val);
   }
   return env;
}

/*@null@*/
environment *new_env()
{
   symbol *begin = new_symbol("begin");
   symbol *define = new_symbol("define");
   symbol *lambda = new_symbol("lambda");
   symbol *quote = new_symbol("quote");
   symbol *plus = new_symbol("+");
   symbol *symbol_car = new_symbol("car");
   symbol *symbol_cdr = new_symbol("cdr");
   symbol *symbol_defmacro = new_symbol("defmacro");
   symbol *symbol_gequal = new_symbol("gequal");
   symbol *quasiquote = new_symbol(readmacro_symbols[QUASIQUOTE]);
   symbol *symbol_true = new_symbol("#t");
   symbol *symbol_false = new_symbol("#f");

   syntax *s_begin = new_syntax(syntax_begin);
   syntax *s_define = new_syntax(syntax_define);
   syntax *s_lambda = new_syntax(syntax_lambda);
   syntax *s_quote = new_syntax(syntax_quote);
   syntax *s_quasiquote = new_syntax(syntax_quasiquote);
   syntax *s_defmacro = new_syntax(syntax_defmacro);
   syntax *s_gequal = new_syntax(syntax_gequal);

   prim_proc *p_plus = new_prim_proc(proc_plus_integer);
   prim_proc *p_car = new_prim_proc(prim_car);
   prim_proc *p_cdr = new_prim_proc(prim_cdr);

   boolean *b_true = new_boolean(true);
   boolean *b_false = new_boolean(false);

   list *vars = 
      cons(begin,
      cons(define,
      cons(lambda,
      cons(quote,
      cons(plus,
      cons(quasiquote,
      cons(symbol_car,
      cons(symbol_cdr,
      cons(symbol_defmacro,
      cons(symbol_gequal,
      cons(symbol_true,
      cons(symbol_false,
      NULL))))))))))));
   
   list *vals =
      cons(s_begin,
      cons(s_define,
      cons(s_lambda,
      cons(s_quote,
      cons(p_plus,
      cons(s_quasiquote,
      cons(p_car,
      cons(p_cdr,
      cons(s_defmacro,
      cons(s_gequal,
      cons(b_true,
      cons(b_false,
      NULL))))))))))));

   return extend_env(vars, vals, NULL);
}

/* eval */
/*@null@*/
list *list_of_values(list *exps, environment *env)
{
   list *result = NULL;
   if(exps != NULL)
   {
      result = cons(eval(car(exps), env), 
      list_of_values(cdr(exps), env));
   }
   return result;
}

/*@null@*/
lispobj *eval(lispobj *exp, environment *env)
{
   lispobj *result;
   lispobj *operator;
   list *operands;

   if(exp == NULL)
   {
      result = NULL;
   }
   else if(is_integer(exp))
   {
      result = exp;
   }
   else if(is_prim_proc(exp))
   {
      result = exp;
   }
   else if(is_lambda(exp))
   {
      result = exp;
   }
   else if(is_character(exp))
   {
      result = exp;
   }
   else if(is_string(exp))
   {
      result = exp;
   }
   else if(is_symbol(exp))
   {
      lispobj *obj = lookup_var_val(exp, env);
      result = cdr(obj);
   }
   else if(is_list(exp))
   {
      operator = eval(car(exp), env);

      if(is_prim_proc(operator))
      {
         operands = list_of_values(cdr(exp), env);
         result = apply_prim_proc(operator, operands);
      }
      else if(is_lambda(operator))
      {
         operands = list_of_values(cdr(exp), env);
         result = apply_lambda(operator, operands);
      }
      else if(is_syntax(operator))
      {
         operands = cdr(exp);
         result = eval_syntax(operator, operands, env);
      }
      else if(is_macro(operator))
      {
         lispobj *expanded_exp;
         operands = cdr(exp);
         expanded_exp = eval_macro(operator, operands, env);
         result = eval(expanded_exp, env);
      }
      else
      {
         fprintf(stderr, "eval error: not applicable\n");
         abort();
      }
   }
   else
   {
      fprintf(stderr, "eval error: can not evaluate\n");
      abort();
   }

   return result;
}

/*@null@*/
lispobj *apply_prim_proc(prim_proc *proc, list *arg)
{
   lispobj *(*p)(list *) = car(proc);
   return p(arg);
}


/* primitive procedures */

/** plus_integers **/
int plus_integers_to_int(list *integers)
{
   int result = 0;
   if(integers != NULL)
   {
      if(is_integer(car(integers)))
      {
         result = integer_to_int(car(integers)) 
            + plus_integers_to_int(cdr(integers));
      }
   }
   return result;
}

/*@null@*/
integer *proc_plus_integer(list *integers)
{
   int result = 0;
   if(integers != NULL)
   {
      result = plus_integers_to_int(integers);
   }
   else
   {
      fprintf(stderr,"plus error: no arg");
   }

   return new_integer(result);
}

lispobj *prim_car(lispobj *obj)
{
   if(obj == NULL)
   {
      fprintf(stderr,"car error: obj == NULL");
      abort();
   }
   else if(list_length(obj) != 1)
   {
      fprintf(stderr,"car error: arg error ");
      abort();
   }
   else
   {
      return car(car(obj));
   }
}

lispobj *prim_cdr(lispobj *obj)
{
   if(obj == NULL)
   {
      fprintf(stderr,"cdr error: obj == NULL");
      abort();
   }
   else if(list_length(obj) != 1)
   {
      fprintf(stderr,"cdr error: arg error ");
      abort();
   }
   else
   {
      return cdr(car(obj));
   }
}


int is_prim_proc(lispobj *obj)
{
   return obj->tid == PRIM_PROC;
}

/*@null@*/
prim_proc *new_prim_proc(lispobj *(*p)(list *))
{
   prim_proc *proc = (prim_proc*)malloc(sizeof(prim_proc));
   proc->tid = PRIM_PROC;
   set_car(proc, (void*)p);

   return proc;
}

/*@null@*/
syntax *new_syntax(lispobj *(*p)(list *,environment *))
{
   syntax *s = (syntax *)malloc(sizeof(syntax));
   s->tid = SYNTAX;
   set_car(s, (void*)p);

   return s;
}

int is_syntax(lispobj *obj)
{
   return obj->tid == SYNTAX;
}

/*@null@*/
lispobj *eval_syntax(syntax *s, list *operands, environment *env)
{
   lispobj *(*evaluate)(list *, environment *);


   evaluate = car(s);
   return evaluate(operands, env);
}

/*@null@*/
lispobj *syntax_quote(list *operands, environment *env)
{
   lispobj *result = operands;

   if(operands != NULL && (cdr(operands) == NULL))
   {
      result = car(operands);
   }
   return result;
}

lispobj *syntax_unquote_splicing(list *operands, environment *env)
{
   return NULL;
}

boolean *syntax_gequal(list *operands, environment *env)
{
   if(list_length(operands) != 2)
   {
      fprintf(stderr, "gequal error\n");
      abort();
   }
   else
   {
      bool result = generic_equal(car(operands), car(cdr(operands)));
      return new_boolean(result);
   }
}

lispobj *evaluate_quasiquote(lispobj *exp, environment *env)
{
   if(exp == NULL)
   {
      return NULL;
   }
   else if(is_cell(exp))
   {
      lispobj *obj = car(exp);
      if(obj == NULL)
      {
         return cons(NULL, evaluate_quasiquote(cdr(exp), env));
      }
      else if(is_symbol(obj) && strcmp(sym_to_string(obj), readmacro_symbols[UNQUOTE]) == 0)
      {
         return eval(car(cdr(exp)), env);
      }
      else if(is_symbol(obj) && strcmp(sym_to_string(obj), readmacro_symbols[UNQUOTE_SPLICING]) == 0)
      {
         return cons(obj, eval(car(cdr(exp)), env));
      }
      else
      {
         lispobj *evaluated_car = evaluate_quasiquote(car(exp), env);

         if(
            evaluated_car != NULL && 
            is_cell(evaluated_car) && 
            is_symbol(car(evaluated_car)) &&
            strcmp(sym_to_string(car(evaluated_car)), readmacro_symbols[UNQUOTE_SPLICING]) == 0)
         {
            return append(cdr(evaluated_car), evaluate_quasiquote(cdr(exp), env));
         }
         else
         {
            return cons(evaluated_car, evaluate_quasiquote(cdr(exp), env));
         }
      }
   }
   else
   {
      return exp;
   }
}

lispobj *syntax_quasiquote(list *operands, environment *env)
{
   if(operands == NULL)
   {
      return NULL;
   }
   else if(cdr(operands) == NULL)
   {
      return evaluate_quasiquote(car(operands), env);
   }
   else
   {
      return cons(
         evaluate_quasiquote(car(operands), env),
         evaluate_quasiquote(cdr(operands), env));
   }
}

lispobj *syntax_unquote(list *operands, environment *env)
{
   return NULL;
}



/*@null@*/
lispobj *syntax_begin(list *exp, environment *env)
{
   lispobj* result = NULL;
   if(exp == NULL)
   {
      fprintf(stderr, "begin error\n");
   }
   else if(cdr(exp) == NULL)
   {
      result = eval(car(exp), env);
   }
   else
   {
      eval(car(exp), env);
      result = syntax_begin(cdr(exp), env);
   }
   return result;
}

/*@null@*/
lispobj *syntax_define(list *exp, environment *env)
{
   lispobj *var = car(exp);
   lispobj *val = eval(car(cdr(exp)),env);
   define_var_val(var, val, env);
   return var;
}

/*@null@*/
lispobj *syntax_lambda(list *exp, environment *env)
{
   return new_lambda(exp, env);
}

/* lambda */
/*@null@*/
lambda *new_lambda(list *arg_body, environment *env)
{
   lambda *l = (lambda *)malloc(sizeof(lambda));
   l->tid = LAMBDA;
   set_car(l, arg_body);
   set_cdr(l, env);
   return l;
}

/*@null@*/
lispobj *apply_lambda(lambda *l, list *vals)
{
   list *vars = car(car(l));
   list *body = cdr(car(l));
   environment *env = cdr(l);
   env = extend_env(vars, vals, env);
   return syntax_begin(body, env);
}

bool is_lambda(lispobj *l)
{
   return l->tid == LAMBDA;
}

/*@null@*/
char *new_word(char *head, char *tail)
{
   int wordlength = (tail - head)/ sizeof(char);
   int wordsize = (wordlength + 1) * sizeof(char);
   char *word = NULL;
   int i;

   if(head <= tail)
   {
      word = (char*)malloc(wordsize);
      for(i = 0; i <= wordlength; ++i)
      {
         word[i] = head[i];
      }
      word[wordlength + 1] = '\0';
   }

   return word;
}

/*@null@*/
char *wordtail(char *exp)
{
   if(
      exp[0] == ' ' || 
      exp[0] == '\0'||
      exp[0] == '\t'||
      exp[0] == '\n')
   {
      return NULL;
   }
   else if(
      exp[1] == ' ' || 
      exp[1] == '\0'||
      exp[1] == '\t'||
      exp[1] == '\n'||
      exp[1] == '(' ||
      exp[1] == ')')
   {
      return exp;
   }
   return wordtail(++exp);
}   


/* return number of equal chars. */
int equal_head_string(char *lhs, char *rhs)
{
   int i = 0;
   if(lhs == NULL || rhs == NULL)
   {
      return i;
   }

   for(i = 0; lhs[i] != '\0' || rhs[i] != '\0'; ++i)
   {
      if(lhs[i] != rhs[i]) break;
   }

   return i;
}

bool string_has_any_symbol(char *string, char **symbols, int num_of_syms, int *len, int *index)
{
   int i;
   int len_symbol;
   int len_string;

   for(i = 0; i < num_of_syms; ++i)
   {
      len_symbol = strlen(symbols[i]);
      len_string = equal_head_string(symbols[i], string);
      if(len_symbol == len_string)
      {
         *len = len_symbol;
         *index = i;
         return true;
      }
   }
   return false;
}

list *tokenize(char *exp)
{
   list *result = NULL;
   char *tail = NULL;
   char c = exp[0];
   int len, index;

   if(c == '\0')
   {
      return result;
   }

   if(
      c == '\n' ||
      c == '\t' ||
      c == ' ' )
   {
      result = tokenize(exp + 1);
      return result;
   }

   if(string_has_any_symbol(exp, brackets_chars, sizeof(brackets_chars)/sizeof(char*), &len, &index))
   {
      result = cons((void*)brackets_chars[index], tokenize(exp + len));
      return result;
   }

   if(string_has_any_symbol(exp, special_chars, sizeof(special_chars)/sizeof(char*), &len, &index))
   {
      result = cons((void*)special_chars[index], tokenize(exp + len));
      return result;
   }

   tail = wordtail(exp);
   result = cons(new_word(exp, tail), tokenize(tail + 1));
   return result;
}


bool index_of_equal_string(char *s, char **strings, int size, int *index)
{
   int i;
   bool result = false;
   for(i = 0; i < size; ++i)
   {
      if(strcmp(s, strings[i]) == 0)
      {
         *index = i;
         result = true;
         break;
      }
   }
   return result;
}

list *expand_readmacro(list *tokens)
{
   int index;
   list *result = NULL;
   if(tokens == NULL)
   {
      return result;
   }

   if(index_of_equal_string(car(tokens), special_chars, sizeof(special_chars)/sizeof(char*), &index))
   {
      cell *closer;
      get_current_exp(cdr(tokens), &closer);
      set_cdr(closer, cons(")", cdr(closer)));
      result = cons("(", cons(readmacro_symbols[index], expand_readmacro(cdr(tokens))));
   }
   else
   {
      set_cdr(tokens, expand_readmacro(cdr(tokens)));
      result = tokens;
   }
   
   return result;
}

int print_token(list *tokens)
{
   char *s = NULL;
   if(tokens != NULL)
   {
      s = car(tokens);
      printf("%s\n", s);
      print_token(cdr(tokens));
   }
   return 1;
}

bool has_any_pointers(char *s, char **pointers, int size)
{
   int i = 0;
   for(i = 0; i < size; ++i)
   {
      if(s == pointers[i])
      {
         return true;
      }
   }
   return false;
}

int delete_tokens(list *tokens)
{
   if(tokens != NULL)
   {
      char *s = car(tokens);

      if(
         !has_any_pointers(s, special_chars, sizeof(special_chars)/sizeof(char*)) &&
         !has_any_pointers(s, brackets_chars, sizeof(brackets_chars)/sizeof(char*)) &&
         !has_any_pointers(s, readmacro_symbols, sizeof(readmacro_symbols)/sizeof(char*)))
      {
         free(s);
      }

      delete_tokens(cdr(tokens));
      free(tokens);
   }
   return 1;
}

bool get_current_exp(list *tokens, cell **tail)
{
   bool result;
   char *s;

   if(tokens == NULL)
   {
      *tail = NULL;
      result = false;
   }
   else
   {
      s = car(tokens);
      if(s[0] == '(')
      {
         *tail = close_token(tokens, 0);
      }
      else
      {
         *tail = tokens;
      }
      result = true;
   }
   return result;
}

list *close_token(list *tokens, int count)
{
   list *result = NULL;
   char *s = NULL;

   if(tokens != NULL && 0 <= count)
   {
      s = car(tokens);
      if(s[0] == ')')
      {
         int crnt_count = count -1;
         if(crnt_count == 0)
         {
            result = tokens;
         }
         else
         {
            result = close_token(cdr(tokens), crnt_count);
         }
      }
      else if(s[0] == '(')
      {
         result = close_token(cdr(tokens), count + 1);
      }
      else
      {
         result = close_token(cdr(tokens), count);
      }
   }
   return result;
}

bool char_is_num(char c)
{
   return (
      c == '1' ||
      c == '2' ||
      c == '3' ||
      c == '4' ||
      c == '5' ||
      c == '6' ||
      c == '7' ||
      c == '8' ||
      c == '9' ||
      c == '0' );
}

bool string_is_num(char *s)
{
   int i;
   int result = 1;
   for(i = 0; s[i] != '\0'; ++i)
   {
      if(!char_is_num(s[i]))
      {
         result = 0;
         break;
      }
   }
   return result;
}

lispobj *new_lispobj(char *exp)
{
   if(string_is_num(exp))
   {
      return new_integer(atoi(exp));
   }
   else
   {
      return new_symbol(exp);
   }
}

lispobj* read_tokens(list *tokens)
{
   lispobj *obj;
   char *s;

   if(tokens == NULL)
   {
      return NULL;
   }

   s = car(tokens);

   if(s[0] == '(')
   {
      s = car(cdr(tokens)); 
      if(s[0] == ')')
      {
         return NULL;
      }
      else
      {
         cell *closer;
         get_current_exp(tokens, &closer);
         obj = read_listtokens(cdr(tokens), closer);
      }
   }
   else
   {
      obj = new_lispobj(car(tokens));
   }

   return obj;
}

list *read_listtokens(list *tokens, cell *tail)
{
   lispobj *obj;
   cell *closer;

   if(tokens == NULL)
   {
      return NULL;
   }
   else if(tokens == tail)
   {
      return NULL;
   }

   get_current_exp(tokens, &closer);

   if(tokens == closer)
   {
      lispobj *new_car = new_lispobj(car(tokens));
      lispobj *new_cdr;

      if(strcmp(car(cdr(tokens)), ".") == 0)
      {
         new_cdr = new_lispobj(car(cdr(cdr(tokens))));
      }
      else
      {
         new_cdr = read_listtokens(cdr(tokens), tail);
      }

      obj = cons(new_car, new_cdr);
   }
   else
   {
      obj = cons(
         read_listtokens(cdr(tokens), closer),
         read_listtokens(cdr(closer), tail));
   }

   return obj;
}

bool print_cell(cell* c, bool is_list_head)
{
   lispobj *first;
   lispobj *second;

   if(c == NULL)
   {
      fprintf(stderr, "cell is NULL\n");
      abort();
   }

   if(is_list_head)
   {
      printf("(");
   }

   first = car(c);
   second = cdr(c);

   if(first == NULL)
   {
      printf("'() ");
   }
   else if(is_cell(first))
   {
      print_cell(first, true);
   }
   else
   {
      print_lispobj(first);
   }

   if(second == NULL)
   {
      printf(")");
   }
   else if(is_cell(second))
   {
      print_cell(second, false);
   }
   else
   {
      printf(". ");
      print_lispobj(second);
      printf(")");
   }
   return true;
}

bool print_lispobj(lispobj *obj)
{
   int tid = obj->tid;
   if(tid == SYMBOL)
   {
      printf("%s ", sym_to_string(obj));
   }
   else if(tid == INTEGER)
   {
      printf("%d ", integer_to_int(obj));
   }
   else if(tid == BOOLEAN)
   {
      printf("%s ", (*((bool*)car(obj)))? "#t" : "#f");
   }
   else
   {
      printf("typeid=%d ", obj->tid);
   }
   return true;
}

bool print_result(lispobj *obj)
{
   if(obj == NULL)
   {
      printf("'()");
   }
   else if(is_cell(obj))
   {
      print_cell(obj, true);
   }
   else
   {
      print_lispobj(obj);
   }
   return true;
}

bool print_tokens(list *l)
{
   if(l == NULL)
   {
      printf("\n");
      return true;
   }
   else
   {
      printf("%s ", (char*)car(l));
      return print_tokens(cdr(l));
   }
}

/*MACRO*/
macro *new_macro(list *arg, list *body)
{
   macro *m = (macro *)malloc(sizeof(macro));
   m->tid = MACRO;
   set_car(m, arg);
   set_cdr(m, body);
   return m;
}

lispobj *eval_macro(macro *m, lispobj *operands, environment *env)
{
   list *vars = car(m);
   list *body = cdr(m);
   env = extend_env(vars, operands, env);
   return eval(body, env);
}

macro *syntax_defmacro(list *exp, environment *env)
{
   lispobj *macroname = car(exp);
   lispobj *arg = car(cdr(exp));
   lispobj *body = car(cdr(cdr(exp)));
   lispobj *macro = new_macro(arg, body);
   define_var_val(macroname, macro, env);
   return macroname;
}

bool is_macro(lispobj *l)
{
   return l->tid == MACRO;
}

bool is_boolean(lispobj* obj)
{
   return obj->tid == BOOLEAN;
}

boolean* new_boolean(bool b)
{
   boolean *nwbln = (boolean*)malloc(sizeof(boolean));
   bool *nwb = (bool *)malloc(sizeof(bool));
   nwbln->tid = BOOLEAN;
   *nwb = b;
   set_car(nwbln, nwb);
   return nwbln;
}

bool equal_boolean(boolean *lhs, boolean *rhs)
{
   if(lhs == rhs)
   {
      return true;
   }
   else if(lhs == NULL || rhs == NULL)
   {
      return false;
   }
   else 
   {
      return *((bool*)car(lhs)) == *((bool*)car(rhs));
   }
}

#ifdef __MAIN__
int main()
{
   char buf[256];
   environment *env = new_env();
   lispobj *obj_in;
   lispobj *obj_out;
   list *tokens;

   while(printf("> ") && fgets(buf, 256, stdin))
   {
      tokens = tokenize(buf);
      if(tokens != NULL)
      {
         tokens = expand_readmacro(tokens);
         obj_in = read_tokens(tokens);
         obj_out = eval(obj_in, env);
         print_result(obj_out);
      }
      printf("\n");
   }
   return 0;
}
#endif
