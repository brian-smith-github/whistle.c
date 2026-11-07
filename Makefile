OBJS  = nd_whistle.o nd_cact.o nd_tokenizer.o nd_quant.o  main.o


CC=gcc 
CFLAGS= -O2
LDFLAGS=  -O -lm

all:    $(OBJS)
	$(CC) -o go nd_whistle.o nd_cact.o  nd_tokenizer.o nd_quant.o main.o -lm



clean: 
	rm -f *.o *~

.c.o:	$<
	$(CC) $(CFLAGS) -c $<

