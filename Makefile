
SRCS = test_lispobj.c lispobj.h lispobj.c 

test: $(SRCS)
	gcc -Wall $(SRCS) -o test
	./test

clean:
	rm *.o test
