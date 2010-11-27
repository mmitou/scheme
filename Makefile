
HDRS = lispobj.h
SRCS =  lispobj.c 
TESTSRCS = test_lispobj.c

all: scheme test tag


scheme: $(HDRS) $(SRCS)
	gcc -Wall -g -D__MAIN__ $(HDRS) $(SRCS) -o scheme

test: $(SRCS) $(TESTSRCS)
	gcc -Wall -g $(HDRS) $(SRCS) $(TESTSRCS) -o test

tag:
	@gtags -v

lint:
	splint -unqualifiedtrans -compdef lispobj.h lispobj.c

testrun:
	./test

run:
	./scheme


clean:
	rm *.o test scheme
