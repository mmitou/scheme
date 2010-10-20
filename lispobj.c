#include "lispobj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* cell */
void *get_val(cell *c, int i)
{
   return c->value[i];
}

void *set_val(cell *c, int i, lispobj *obj)
{
   void *result = NULL;
   if(is_cell(c))
   {
      result = c->value[i];
      c->value[i] = obj;
   }
   else
   {
      fprintf(stderr, "ERROR: %p is not cell\n", c);
   }
   return result;
}

void *car(cell *c)
{
   return get_val(c, 0);
}

void *cdr(cell *c)
{
   return get_val(c, 1);
}

void *set_car(cell *c, void *obj)
{
   return set_val(c, 0, obj);
}

void *set_cdr(cell *c, void *obj)
{
   return set_val(c, 1, obj);
}

cell *cons(void *l, void *r)
{
   cell *c = (cell *)malloc(sizeof(cell));
   c->tid = CELL;
   set_car(c, l);
   set_cdr(c, r);
   return c;
}

int is_cell(lispobj *obj)
{
   return obj == NULL ? 0 :obj->tid == CELL;
}

int equal_cell(cell *l, cell* r)
{
   int result = 0;
   if(is_cell(l) && is_cell(r))
   {
      result = 
         (equal(car(l), car(r)) && equal(cdr(l), cdr(r)));
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
   s->value[0] = symbol_name;
   return s;
}

int is_symbol(lispobj *l)
{
   return l == NULL ? 0 : l->tid == SYMBOL;
}

int equal_symbol(symbol *l, symbol *r)
{
   int result = 0;
   if(is_symbol(l) && is_symbol(r))
   {
      result = 
         strcmp(sym_to_chars(l), sym_to_chars(r)) == 0;
   }
   return result;
}

char *sym_to_chars(symbol *s)
{
   return (char *)(s->value[0]);
}

/* integer */

integer *new_integer(int x)
{
   integer *i = (integer*)malloc(sizeof(integer));
   int *p = (int *)malloc(sizeof(int));
   i->tid = INTEGER;
   *p = x;
   i->value[0] = p;
   return i;
}

int is_integer(integer *i)
{
   return i == NULL ? 0 : i->tid == INTEGER;
}

int integer_to_int(integer *i)
{
   return *(int *)(i->value[0]);
}

int equal_integer(integer *l, integer *r)
{
   int result = 0;
   if(is_integer(l) && is_integer(r))
   {
      result = integer_to_int(l) == integer_to_int(r);
   }
   return result;
}

/* char */

character *new_character(char c)
{
   character *chrctr = (character*)malloc(sizeof(character));
   char *d = (char *)malloc(sizeof(char));
   chrctr->tid = CHARACTER;
   *d = c;
   chrctr->value[0] = d;
   chrctr->value[1] = NULL;
   return chrctr;
}

int is_character(character *c)
{
   return c->tid == CHARACTER;
}

int equal_character(character *l, character *r)
{
   return (is_character(l) && is_character(r)) && 
      ((*(char*)(l->value[0])) == (*(char*)(r->value[0])));
}

char character_to_char(character *c)
{
   return *(char*)(c->value[0]);
}

/* list */

int is_list(lispobj *obj)
{
   int result = 0;
   if(obj == NULL)
   {
      result = 1;
   }
   else if(is_cell(obj))
   {
      result = is_list(cdr(obj));
   }
   return result;
}

cell *assoc(symbol *s, list *l)
{
   cell *result = NULL;
   cell *pair = NULL;
   if(l != NULL && is_list(l))
   {
      pair = car(l);
      if(equal(s, car(pair)))
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

int list_length(list *l)
{
   return l == NULL ? 0 : 1 + list_length(cdr(l));
}

/* string */
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
         c->value[1] = d;
         c = d;
         ++i;
      }
   }
   
   return result;
}

int is_string(lispobj *s)
{
   int result = 0;
   if(s == NULL)
   {
      result = 1;
   }
   else if(is_character(s))
   {
      result = is_string(cdr(s));
   }
   else
   {
      result = 0;
   }
   return result;
}

int equal_string(string *l, string *r)
{
   int result = 0;
   if(l == NULL && r == NULL)
   {
      result = 1;
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

/* generic equal */
int (*equalf_pointers[NUM_OF_TYPES])(lispobj *, lispobj *)
 = {NULL};

void init_equal_func()
{
   equalf_pointers[SYMBOL] = equal_symbol;
   equalf_pointers[CELL] = equal_cell;
   equalf_pointers[INTEGER] = equal_integer;
}

int equal(lispobj *l, lispobj *r)
{
   int result = 0;
   if(equalf_pointers[0] == NULL)
   {
      init_equal_func();
   }

   if(l == r)
   {
      result = 1;
   }
   else if(l == NULL || r == NULL)
   {
      result = 0;
   }
   else
   {
      result = equalf_pointers[l->tid](l, r);
   }

   return result;
}

/* environment */
list *new_frame(list *vars, list *vals)
{
   list *result = NULL;
   if(vars != NULL && vals != NULL)
   {
      result = cons(cons(car(vars), car(vals)), 
      new_frame(cdr(vars), cdr(vals)));
   }
   return result;
}

environment *extend_env(
   list *vars, list *vals, environment *env)
{
   return cons(new_frame(vars, vals), env);
}

environment *define_var_val(
   symbol *var, lispobj *val, environment *env)
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

environment *init_env()
{
   symbol *begin = new_symbol("begin");
   symbol *define = new_symbol("define");
   symbol *lambda = new_symbol("lambda");
   symbol *quote = new_symbol("quote");
   symbol *plus = new_symbol("+");

   syntax *s_begin = new_syntax(syntax_begin);
   syntax *s_define = new_syntax(syntax_define);
   syntax *s_lambda = new_syntax(syntax_lambda);
   syntax *s_quote = new_syntax(syntax_quote);
   prim_proc *p_plus = new_prim_proc(proc_plus_integer);

   list *vars = 
      cons(begin,
      cons(define,
      cons(lambda,
      cons(quote,
      cons(plus,
      NULL)))));
   
   list *vals =
      cons(s_begin,
      cons(s_define,
      cons(s_lambda,
      cons(s_quote,
      cons(p_plus,
      NULL)))));

   return extend_env(vars, vals, NULL);
}

/* eval */
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

lispobj *eval(lispobj *exp, environment *env)
{
   lispobj *result;
   lispobj *operator;
   list *operands;

   if(is_integer(exp))
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
      result = cdr(lookup_var_val(exp, env));
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
      else
      {
         fprintf(stderr, "eval error: not applicable\n");
      }
   }
   else
   {
      fprintf(stderr, "eval error: can not evaluate\n");
   }

   return result;
}


lispobj *apply_prim_proc(prim_proc *proc, list *arg)
{
   lispobj *(*p)(list *) = proc->value[0];
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

int is_prim_proc(lispobj *obj)
{
   return obj->tid == PRIM_PROC;
}

prim_proc *new_prim_proc(lispobj *(*p)(list *))
{
   prim_proc *proc = (prim_proc*)malloc(sizeof(prim_proc));
   proc->tid = PRIM_PROC;
   proc->value[0] = (void*)p;

   return proc;
}

syntax *new_syntax(lispobj *(*p)(list *,environment *))
{
   syntax *s = (syntax *)malloc(sizeof(syntax));
   s->tid = SYNTAX;
   s->value[0] = (void*)p;

   return s;
}

int is_syntax(lispobj *obj)
{
   return obj->tid == SYNTAX;
}

lispobj *eval_syntax(syntax *s, list *exp, environment *env)
{
   lispobj *(*evaluate)(list *, environment *);
   evaluate = s->value[0];
   return evaluate(exp, env);
}

lispobj *syntax_quote(list *exp, environment *env)
{
   lispobj *result = NULL;
   if(exp != NULL && cdr(exp) == NULL)
   {
      result = car(exp);
   }
   else
   {
      fprintf(stderr, "quote error\n");
   }
   return result;
}

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

lispobj *syntax_define(list *exp, environment *env)
{
   lispobj *var = car(exp);
   lispobj *val = car(cdr(exp));
   define_var_val(var, val, env);
   return NULL;
}

lispobj *syntax_lambda(list *exp, environment *env)
{
   return new_lambda(exp, env);
}


/* lambda */
lambda *new_lambda(list *arg_body, environment *env)
{
   lambda *l = (lambda *)malloc(sizeof(lambda));
   l->tid = LAMBDA;
   l->value[0] = arg_body;
   l->value[1] = env;
   return l;
}

lispobj *apply_lambda(lambda *l, list *vals)
{
   list *vars = car((list*)(l->value[0]));
   list *body = cdr((list*)(l->value[0]));
   environment *env = l->value[1];
   env = extend_env(vars, vals, env);
   return syntax_begin(body, env);
}

int is_lambda(lispobj *l)
{
   return l->tid == LAMBDA;
}

char *new_word(char *head, char *tail)
{
   int wordlength = ((tail - head)/ sizeof(char)) + 1;
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

char *wordtail(char *exp)
{
   if(exp[0] == ' ' || exp[0] == '\0')
   {
      return NULL;
   }
   else if(exp[1] == ' ' || exp[1] == '\0')
   {
      return exp;
   }
   return wordtail(++exp);
}   


list *tokenize(char *exp)
{
   list *result = NULL;
   char *tail = NULL;

   switch(exp[0])
   {
      case '\0':
         break;
      case ' ':
         result = tokenize(++exp);
         break;
      case '(':
         result = cons("(", tokenize(++exp));
         break;
      case ')':
         result = cons(")", tokenize(++exp));
         break;
      default:
         tail = wordtail(exp);
         result = 
            cons(new_word(exp, tail), tokenize(tail + 1));
         break;
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
