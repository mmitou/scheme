
HDRS = lispobj.h
SRCS = test_lispobj.c lispobj.c 


all: test 
test: $(SRCS)
	gcc -Wall -g $(HDRS) $(SRCS) -o test
	gtags -v

lint:
	splint -unqualifiedtrans -compdef lispobj.h lispobj.c

run:
	./test

clean:
	rm *.o test
