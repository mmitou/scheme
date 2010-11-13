#include "lispobj.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int test_symbol()
{
   symbol *s[3];
   char *hello = "hello";
   char *world = "world";
   s[0] = new_symbol(hello);
   s[1] = new_symbol(hello);
   s[2] = new_symbol(world);

   assert(equal_symbol(s[0], s[1]));
   assert(!equal_symbol(s[0], s[2]));
   assert(is_symbol(s[0]));
   assert(strcmp(sym_to_string(s[0]), hello) == 0);
   assert(strcmp(sym_to_string(s[0]), world) != 0);
   return 1;
}

int test_integer()
{
   integer *i[3];
   int x = 10;
   int y = 20;
   i[0] = new_integer(x);
   i[1] = new_integer(x);
   i[2] = new_integer(y);

   assert(is_integer(i[0]));
   assert(equal_integer(i[0], i[1]));
   assert(!equal_integer(i[0], i[2]));
   assert(integer_to_int(i[0]) == x);
   assert(integer_to_int(i[2]) == y);
   return 1;
}

int test_cell()
{
   cell *c[3];
   symbol *x = new_symbol("x");
   symbol *y = new_symbol("y");
   integer *i = new_integer(10);
   integer *j = new_integer(20);
   c[0] = cons(x, i);
   c[1] = cons(x, i);
   c[2] = cons(y, j);

   assert(equal_symbol(car(c[0]), car(c[1])));
   assert(!equal_symbol(car(c[0]), car(c[2])));

   assert(equal_integer(cdr(c[0]), cdr(c[1])));
   assert(!equal_integer(cdr(c[0]), cdr(c[2])));

   set_car(c[1], y);
   assert(!equal_symbol(car(c[0]), car(c[1])));
   assert(equal_symbol(car(c[1]), car(c[2])));

   set_cdr(c[1], j);
   assert(!equal_integer(cdr(c[0]), cdr(c[1])));
   assert(equal_integer(cdr(c[1]), cdr(c[2])));

   assert(!equal_cell(c[0], c[1]));
   assert(equal_cell(c[1], c[2]));
   
   return 1;
}

int test_list()
{
   cell *c[3];
   symbol *x = new_symbol("x");
   symbol *y = new_symbol("y");
   symbol *z = new_symbol("z");
   integer *i = new_integer(10);
   integer *j = new_integer(20);
   list *l;
   c[0] = cons(x, i);
   c[1] = cons(x, i);
   c[2] = cons(y, j);

   l = cons(c[0], cons(c[1], cons(c[2], NULL)));

   assert(is_list(l));
   assert(is_list(NULL));
   assert(!is_list(c[0]));

   assert(equal(assoc(x, l), c[0]));
   assert(equal(assoc(y, l), c[2]));
   assert(equal(assoc(z, l), NULL));

   return 1;
}

int test_environment()
{
   symbol *sa = new_symbol("a");
   symbol *sb = new_symbol("b");
   symbol *sc = new_symbol("c");

   symbol *sx = new_symbol("x");
   symbol *sy = new_symbol("y");
   symbol *sz = new_symbol("z");

   symbol *sn = new_symbol("n");

   integer *i10 = new_integer(10);
   integer *i20 = new_integer(20);
   integer *i30 = new_integer(30);

   integer *i40 = new_integer(40);
   integer *i50 = new_integer(50);
   integer *i60 = new_integer(60);

   list *vara = cons(sa, cons(sb, cons(sc, NULL)));
   list *varx = cons(sx, cons(sy, cons(sz, NULL)));
   list *val10 = cons(i10, cons(i20, cons(i30, NULL)));
   list *val40 = cons(i40, cons(i50, cons(i60, NULL)));

   environment *env = NULL;

   env = extend_env(vara, val10, env);

   assert(equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(equal(lookup_var_val(sc, env), cons(sc, i30)));
   /*assert(equal(lookup_var_val(sx, env), NULL));*/

   env = define_var_val(sx, i40, env);

   /*
   printf("%s %d\n", 
   sym_to_string(car(lookup_var_val(sx, env))),
   integer_to_int(cdr(lookup_var_val(sx, env))));
   */

   assert(equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(equal(lookup_var_val(sc, env), cons(sc, i30)));
   assert(equal(lookup_var_val(sx, env), cons(sx, i40)));
   assert(equal(lookup_var_val(sy, env), NULL));

   env = set_var_val(sx, i50, env);

   assert(equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(equal(lookup_var_val(sc, env), cons(sc, i30)));
   assert(equal(lookup_var_val(sx, env), cons(sx, i50)));
   assert(equal(lookup_var_val(sy, env), NULL));

   env = extend_env(varx, val40, env);
   assert(equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(equal(lookup_var_val(sc, env), cons(sc, i30)));
   assert(equal(lookup_var_val(sx, env), cons(sx, i40)));
   assert(equal(lookup_var_val(sy, env), cons(sy, i50)));
   assert(equal(lookup_var_val(sz, env), cons(sz, i60)));
   assert(equal(lookup_var_val(sn, env), NULL));

   return 1;
}

int test_eval()
{
   symbol *sa = new_symbol("a");
   symbol *sb = new_symbol("b");
   symbol *sc = new_symbol("c");
   symbol *plus = new_symbol("+");

   prim_proc *proc = new_prim_proc(proc_plus_integer);

   integer *i10 = new_integer(10);
   integer *i20 = new_integer(20);
   integer *i30 = new_integer(30);
   integer *i60 = new_integer(60);

   list *vara = cons(sa, cons(sb, cons(sc, NULL)));
   list *val10 = cons(i10, cons(i20, cons(i30, NULL)));
   list *plus_i = cons(plus, val10);

   environment *env = NULL;

   env = extend_env(vara, val10, env);

   assert(equal(eval(sa, env), i10));
   assert(equal(eval(i10, env), i10));

   define_var_val(plus, proc, env);

   assert(equal(car(list_of_values(val10, env)), i10));
   assert(equal(eval(plus_i, env), i60)); 

   return 1;
   
}

int test_plus_integer()
{
   integer *i10 = new_integer(10);
   integer *i20 = new_integer(20);
   integer *i30 = new_integer(30);
   integer *i60 = new_integer(60);
   list *val10 = cons(i10, cons(i20, cons(i30, NULL)));

   assert(equal(proc_plus_integer(cons(i10,NULL)), i10));
   assert(equal(proc_plus_integer(val10), i60));

   return 1;
}

int test_begin()
{
   integer *i10 = new_integer(10);
   integer *i20 = new_integer(20);
   integer *i30 = new_integer(30);

   list *val10 = cons(i10, cons(i20, cons(i30, NULL)));
   environment *env = new_env();
   assert(equal(syntax_begin(val10, env), i30));

   return 1;
}

int test_quote()
{
   symbol *sa = new_symbol("a");
   symbol *sb = new_symbol("b");
   symbol *sc = new_symbol("c");
   list *va = cons(sa, cons(sb, cons(sc, NULL)));
   environment *env = new_env();

   list *ql = syntax_quote(va, env);

   assert(equal(car(ql), sa));
   assert(equal(car(cdr(ql)), sb));
   assert(equal(car(cdr(cdr(ql))), sc));

   return 1;
}

int test_string()
{
   string *s = new_string("hello!");
   char *c = string_to_chars(cdr(s));

   printf("%s\n", c);
   return 1;
}

int test_tokenize()
{
   list *l = tokenize(" abc   123  ");
   assert(list_length(l) == 2);
   delete_tokens(l);

   l = tokenize(" (( 1   2    3)   (hello world) )");
   assert(list_length(l) == 11);
   delete_tokens(l);

   return 1;
}

int test_newlispobj()
{
   integer *i10 = new_lispobj("10");
   symbol *s = new_symbol("hello");

   assert(integer_to_int(i10) == 10);
   assert(strcmp(sym_to_string(s), "hello") == 0);

   return 1;
}

int test_read_tokens()
{
   environment *env = new_env();
   integer *i;
   list *l;
   list *r;

   l = tokenize("10");
   i = read_tokens(l);
   print_lispobj(i);
   assert(integer_to_int(i) == 10);


   l = tokenize("(+ 5 5)");
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(i) == 10);
   printf("----\n");
   printf("----\n");

   l = tokenize("(+ (+ 1 4) (+ 2 3))");
   r = read_tokens(l);
   print_lispobj(car(r));
   printf("-\n");
   print_lispobj(cdr(r));  
   printf("-\n");
   print_lispobj(car(cdr(r)));  
   printf("-\n");

   i = eval(r, env);
   assert(integer_to_int(i) == 10);
   printf("----\n");


   return 1;
}

int test_close_token()
{
   list *l = tokenize("(test (close token) hello world)");
   printf("--test close_token\n");
   print_token(close_token(l, 0));
   print_token(close_token(cdr(l), 0));

   return 1;
}




int main()
{
   test_symbol();
   test_integer();
   test_cell();
   test_list();
   test_environment();

   test_eval();

   test_plus_integer();
   test_begin();

   test_string();
   test_tokenize();
   test_newlispobj();
   test_close_token();
   test_read_tokens();

   return 0;
}
