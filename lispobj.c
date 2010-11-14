
#include "lispobj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


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
   return (char *)(s->value[0]);
}

/* integer */
/*@null@*/
integer *new_integer(int x)
{
   integer *i = (integer*)malloc(sizeof(integer));
   int *p = (int *)malloc(sizeof(int));
   i->tid = INTEGER;
   *p = x;
   i->value[0] = p;
   return i;
}

bool is_integer(integer *i)
{
   return i == NULL ? 0 : i->tid == INTEGER;
}

int integer_to_int(integer *i)
{
   return *(int *)(i->value[0]);
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
   chrctr->value[0] = d;
   chrctr->value[1] = NULL;
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
         c->value[1] = d;
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

/* generic equal */
bool (*equalf_pointers[NUM_OF_TYPES])(lispobj *, lispobj *)
 = {NULL};

static void init_equal_func()
{
   equalf_pointers[SYMBOL] = equal_symbol;
   equalf_pointers[CELL] = equal_cell;
   equalf_pointers[INTEGER] = equal_integer;
}

bool equal(lispobj *l, lispobj *r)
{
   bool result = false;
   if(equalf_pointers[0] == NULL)
   {
      init_equal_func();
   }

   if(l == r)
   {
      result = true;
   }
   else if(l == NULL || r == NULL)
   {
      result = false;
   }
   else
   {
      result = equalf_pointers[l->tid](l, r);
   }

   return result;
}

/* environment */
/*@null@*/
static list *new_frame(list *vars, list *vals)
{
   list *result = NULL;
   if(vars != NULL && vals != NULL)
   {
      result = cons(cons(car(vars), car(vals)), 
      new_frame(cdr(vars), cdr(vals)));
   }
   return result;
}

/*@null@*/
environment *extend_env(
   list *vars, list *vals, environment *env)
{
   return cons(new_frame(vars, vals), env);
}

/*@null@*/
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

int is_prim_proc(lispobj *obj)
{
   return obj->tid == PRIM_PROC;
}

/*@null@*/
prim_proc *new_prim_proc(lispobj *(*p)(list *))
{
   prim_proc *proc = (prim_proc*)malloc(sizeof(prim_proc));
   proc->tid = PRIM_PROC;
   proc->value[0] = (void*)p;

   return proc;
}

/*@null@*/
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

/*@null@*/
lispobj *eval_syntax(syntax *s, list *exp, environment *env)
{
   lispobj *(*evaluate)(list *, environment *);
   evaluate = s->value[0];
   return evaluate(exp, env);
}

/*@null@*/
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
   lispobj *val = car(cdr(exp));
   define_var_val(var, val, env);
   return NULL;
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
   l->value[0] = arg_body;
   l->value[1] = env;
   return l;
}

/*@null@*/
lispobj *apply_lambda(lambda *l, list *vals)
{
   list *vars = car((list*)(l->value[0]));
   list *body = cdr((list*)(l->value[0]));
   environment *env = l->value[1];
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
   if(exp[0] == ' ' || exp[0] == '\0')
   {
      return NULL;
   }
   else if(
      exp[1] == ' ' || 
      exp[1] == '\0'||
      exp[1] == '(' ||
      exp[1] == ')')
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
         result = tokenize(exp + 1);
         break;
      case '(':
         result = cons("(", tokenize(exp + 1));
         break;
      case ')':
         result = cons(")", tokenize(exp + 1));
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

int delete_tokens(list *tokens)
{
   char *s = NULL;
   if(tokens != NULL)
   {
      s = car(tokens);
      if(s[0] != '(' && s[0] != ')')
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
   lispobj *result = NULL;
   if(string_is_num(exp))
   {
      result = new_integer(atoi(exp));
   }
   else
   {
      result = new_symbol(exp);
   }
   return result;
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
      cell *closer;
      get_current_exp(tokens, &closer);
      obj = read_listtokens(cdr(tokens), closer);
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
      obj = cons(
         new_lispobj(car(tokens)),
         read_listtokens(cdr(tokens), tail));
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
   else
   {
      printf("typeid=%d ", obj->tid);
   }
   return true;
}

bool print_result(lispobj *obj)
{
   if(obj == NULL || is_cell(obj))
   {
      print_cell(obj, true);
   }
   else
   {
      print_lispobj(obj);
   }
   return true;
}

