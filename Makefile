CC = gcc -fPIC 
LDFLAGS = -lm

#ISDEBUG = 1
# set DEBUG options
ifdef ISDEBUG
CFLAGS = -Wall -Wextra -ggdb -pg -DDEBUG
else
CFLAGS = -Wall -O0 -g
endif

#name all the object files
OBJS = light.o hash_table/hashtable.o poll.o util.o log.o

all : light hash_table

light : $(OBJS)
	$(CC) $(LDFLAGS) -o light $^

hash_table : 
	make -C hash_table

debug :
	make all DEBUG=1

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $^

doxy :
	doxygen Doxyfile
	sh update_doc.sh	

clean :
	rm -rf $(OBJS) light doc/ manual.pdf

cs :
	cscope -bRv

cscope :
	cscope -bRv

