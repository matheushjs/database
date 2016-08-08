CC = gcc
CFLAGS = -Wall
INCLUDE = -I ./include
SRC = ./src/shell.c ./src/myregex.c ./src/utils.c ./src/globals.c ./src/table_op.c ./src/table_kernel.c ./src/table_high.c
RM = rm -f

shell: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(INCLUDE)

all:
	make clean
	make shell

run:
	make clean
	./shell

test:
	make all
	cp shell ../test_env
	cd ../test_env
	sh test.sh
	sh check.sh

debug:
	make clean
	valgrind --leak-check=full ./shell

clean:
	$(RM) *~ src/*~ include/*~ workshop/*~ dotsh/*~ \
		*.dat *.tmp *.idx allindexes.txt alltables.txt

#fullclean erases .un files.
fullclean:
	make clean
	$(RM) .*~ src/.*~ include/.*~ workshop/.*~ dotsh/.*~ shell
	$(RM) .*.un src/.*.un include/.*.un workshop/.*.un dotsh/.*.un
