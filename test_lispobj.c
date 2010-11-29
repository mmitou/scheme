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

   print_result(c[0]);
   printf("\n");
   print_result(c[1]);
   printf("\n");
   print_result(c[2]);
   printf("\n");
   print_result(l);
   printf("\n");

   assert(is_list(l));
   assert(is_list(NULL));
   assert(!is_list(c[0]));

   assert(generic_equal(assoc(x, l), c[0]));
   assert(generic_equal(assoc(y, l), c[2]));
   assert(generic_equal(assoc(z, l), NULL));

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

   assert(generic_equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(generic_equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(generic_equal(lookup_var_val(sc, env), cons(sc, i30)));

   env = define_var_val(sx, i40, env);

   assert(generic_equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(generic_equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(generic_equal(lookup_var_val(sc, env), cons(sc, i30)));
   assert(generic_equal(lookup_var_val(sx, env), cons(sx, i40)));
   assert(generic_equal(lookup_var_val(sy, env), NULL));

   env = set_var_val(sx, i50, env);

   assert(generic_equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(generic_equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(generic_equal(lookup_var_val(sc, env), cons(sc, i30)));
   assert(generic_equal(lookup_var_val(sx, env), cons(sx, i50)));
   assert(generic_equal(lookup_var_val(sy, env), NULL));

   env = extend_env(varx, val40, env);
   assert(generic_equal(lookup_var_val(sa, env), cons(sa, i10)));
   assert(generic_equal(lookup_var_val(sb, env), cons(sb, i20)));
   assert(generic_equal(lookup_var_val(sc, env), cons(sc, i30)));
   assert(generic_equal(lookup_var_val(sx, env), cons(sx, i40)));
   assert(generic_equal(lookup_var_val(sy, env), cons(sy, i50)));
   assert(generic_equal(lookup_var_val(sz, env), cons(sz, i60)));
   assert(generic_equal(lookup_var_val(sn, env), NULL));

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

   assert(generic_equal(eval(sa, env), i10));
   assert(generic_equal(eval(i10, env), i10));

   define_var_val(plus, proc, env);

   assert(generic_equal(car(list_of_values(val10, env)), i10));
   assert(generic_equal(eval(plus_i, env), i60)); 

   return 1;
   
}

int test_plus_integer()
{
   integer *i10 = new_integer(10);
   integer *i20 = new_integer(20);
   integer *i30 = new_integer(30);
   integer *i60 = new_integer(60);
   list *val10 = cons(i10, cons(i20, cons(i30, NULL)));

   assert(generic_equal(proc_plus_integer(cons(i10,NULL)), i10));
   assert(generic_equal(proc_plus_integer(val10), i60));

   return 1;
}

int test_begin()
{
   integer *i10 = new_integer(10);
   integer *i20 = new_integer(20);
   integer *i30 = new_integer(30);

   list *val10 = cons(i10, cons(i20, cons(i30, NULL)));
   environment *env = new_env();
   assert(generic_equal(syntax_begin(val10, env), i30));

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

   assert(generic_equal(car(ql), sa));
   assert(generic_equal(car(cdr(ql)), sb));
   assert(generic_equal(car(cdr(cdr(ql))), sc));

   return 1;
}

int test_string()
{
   string *s = new_string("hello!");
   assert(is_string(s));
   assert(strcmp(car(s), "hello!") == 0);
   return 1;
}

int test_tokenize()
{
   list *l = tokenize(" abc   123  ");
   char *s;

   assert(list_length(l) == 2);
   delete_tokens(l);

   l = tokenize(" (( 1   2    3)   (hello world) )");
   assert(list_length(l) == 11);
   delete_tokens(l);

   l = tokenize("'x");
   s = (char*)car(l);
   assert(s[0] == '\'');
   delete_tokens(l);

   l = tokenize("`x");
   s = (char*)car(l);
   assert(s[0] == '`');
   assert(list_length(l) == 2);
   delete_tokens(l);

   l = tokenize(",x");
   s = (char*)car(l);
   assert(s[0] == ',');
   assert(list_length(l) == 2);
   delete_tokens(l);

   l = tokenize(",@x");
   s = (char*)car(l);
   assert(s[0] == ',');
   assert(s[1] == '@');
   delete_tokens(l);

   l = tokenize("`(xxx ,y ,@a )");
   assert(list_length(l) == 8);
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
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   assert(integer_to_int(i) == 10);

   l = tokenize("(+ 5 5)");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   assert(integer_to_int(i) == 10);

   l = tokenize("(+ (+ 1 4) (+ 2 3))");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   assert(integer_to_int(i) == 10);

   l = tokenize("(+ (+ 1 2) (+ 2 3) (+ 3 4))");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   assert(integer_to_int(i) == 15);

   l = tokenize("(+ (+ (+ 1 2) (+ 2 3)) (+ 3 4))");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   assert(integer_to_int(i) == 15);

   l = tokenize("((lambda (x) (+ x 1)) 10)");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   assert(integer_to_int(i) == 11);

   l = tokenize("(define x 10)");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);

   l = tokenize("x");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   assert(integer_to_int(i) == 10);

   l = tokenize("(define y (lambda (a) (+ a x)))");
   r = read_tokens(l);
   r = expand_readmacro(r);
   i = eval(r, env);
   l = tokenize("(y 1)");
   r = expand_readmacro(r);
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(i) == 11);

   l = tokenize("'(1 . 2)");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(car(i)) == 1);
   assert(integer_to_int(cdr(i)) == 2);

   l = tokenize("`(1 2)");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(car(i)) == 1);
   assert(integer_to_int(car(cdr(i))) == 2);

   l = tokenize("`(,x ,x)");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(car(i)) == 10);
   assert(integer_to_int(car(cdr(i))) == 10);

   l = tokenize("`(,(+ 5 5) ,(+ 5 5))");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(car(i)) == 10);
   assert(integer_to_int(car(cdr(i))) == 10);


   
   l = tokenize("(define y `(,x ,x))");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);

   l = tokenize("`(,@y)");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(car(i)) == 10);
   assert(integer_to_int(car(cdr(i))) == 10);

   l = tokenize("`(1 ,@y 3)");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(integer_to_int(car(i)) == 1);
   assert(integer_to_int(car(cdr(i))) == 10);
   assert(integer_to_int(car(cdr(cdr(i)))) == 10);
   assert(integer_to_int(car(cdr(cdr(cdr(i))))) == 3);

   l = tokenize("()");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(i == NULL);

   l = tokenize("'(   ) ");
   l = expand_readmacro(l);
   r = read_tokens(l);
   i = eval(r, env);
   assert(i == NULL);


   return 1;
}

bool test_expandreadmacro()
{
   list *l = tokenize(" abc   123  ");
   char *s;

   assert(list_length(l) == 2);
   delete_tokens(l);

   l = tokenize(" (( 1   2    3)   (hello world) )");
   l = expand_readmacro(l);
   print_tokens(l);
   delete_tokens(l);

   l = tokenize("'x");
   l = expand_readmacro(l);
   print_tokens(l);
   delete_tokens(l);

   l = tokenize("`x");
   l = expand_readmacro(l);
   print_tokens(l);
   s = (char*)car(l);
   delete_tokens(l);

   l = tokenize(",x");
   l = expand_readmacro(l);
   print_tokens(l);
   delete_tokens(l);

   l = tokenize(",@x");
   l = expand_readmacro(l);
   print_tokens(l);
   delete_tokens(l);

   l = tokenize("`(xxx ,y ,@a )");
   l = expand_readmacro(l);
   print_tokens(l);
   delete_tokens(l);

   l = tokenize("`(xxx (,y (,@a) . z ))");
   l = expand_readmacro(l);
   print_tokens(l);
   delete_tokens(l);

   l = tokenize("'(1 . 2)");
   l = expand_readmacro(l);
   print_tokens(l);
   delete_tokens(l);

   return true;
}

bool test_macro()
{
   list *l;
   environment *env = new_env();
   macro *m;
   list *arg;
   list *body;

   l = read_tokens(expand_readmacro(
         tokenize("(defmacro m (x) `(,x ,x))")));
   m = eval(eval(l, env), env);
   
   arg = car(m);
   body = cdr(m);
   assert(equal_symbol(car(arg), new_symbol("x")));
   assert(equal_symbol(car(body), new_symbol("quasiquote")));
   return true;
}

bool test_equal()
{
   symbol *s[3];
   integer *i[3];
   boolean *b[3];
   
   s[0] = new_symbol("foo");
   s[1] = new_symbol("bar");
   s[2] = new_symbol("foo");

   i[0] = new_integer(0);
   i[1] = new_integer(1);
   i[2] = new_integer(0);

   b[0] = new_boolean(true);
   b[1] = new_boolean(false);
   b[2] = new_boolean(true);

   assert(generic_equal(s[0], s[0]) == true);
   assert(generic_equal(s[0], s[1]) == false);
   assert(generic_equal(s[0], s[2]) == true);

   assert(generic_equal(i[0], i[0]) == true);
   assert(generic_equal(i[0], i[1]) == false);
   assert(generic_equal(i[0], i[2]) == true);

   assert(generic_equal(b[0], b[0]) == true);
   assert(generic_equal(b[0], b[1]) == false);
   assert(generic_equal(b[0], b[2]) == true);

   assert(generic_equal(b[0], s[0]) == false);
   assert(generic_equal(b[0], i[1]) == false);
   assert(generic_equal(s[0], b[1]) == false);

   return true;
}

bool test_cond()
{
   list *l;
   environment *env = new_env();
   macro *m;
   
   l = read_tokens(expand_readmacro(
         tokenize("(cond ((+ 1 2) (+ 2 3)) (else 0))")));
   m = eval(eval(l, env), env);
   
   assert(integer_to_int(m) == 5);

   l = read_tokens(expand_readmacro(
         tokenize("(cond (#f (+ 2 3)) (#f (+ 2 3))(else 0))")));
   m = eval(eval(l, env), env);
   assert(integer_to_int(m) == 0);

   return true;
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
   test_expandreadmacro();
   test_read_tokens();
   test_macro();
   test_equal();
   test_cond();

   return 0;
}
