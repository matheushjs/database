CC = gcc
CFLAGS = -Wall
LIBS = -lm
INCLUDE = -I ./include
SRC = ./src/shell.c ./src/myregex.c ./src/utils.c ./src/globals.c ./src/table_op.c ./src/table_kernel.c
RM = rm -f

all:
	make shell

shell: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(INCLUDE)

debug:
	make clean
	make shell
	cp shell test_environment
	sh dotsh/shelldebug.sh

test:
	make clean
	make shell
	cp shell test_environment
	sh dotsh/shelltest.sh

itest:
	make clean
	make shell
	cp shell test_environment
	cp input test_environment
	sh dotsh/shellitest.sh

idebug:
	make clean
	make shell
	cp shell test_environment
	cp input test_environment
	sh dotsh/shellidebug.sh

clean:
	$(RM) *~ src/*~ include/*~ workshop/*~ dotsh/*~ shell
	make clean -C test_environment

#fullclean erases .un files.
fullclean:
	make clean
	$(RM) .*~ src/.*~ include/.*~ workshop/.*~ dotsh/.*~
	$(RM) .*.un src/.*.un include/.*.un workshop/.*.un dotsh/.*.un
