CONTRIB_DIR = ..
LLQUEUE_DIR = $(CONTRIB_DIR)/CLinkedListQueue

GCOV_OUTPUT = *.gcda *.gcno *.gcov 
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
SHELL  = /bin/bash
CC     = gcc
CCFLAGS = -g -O2 -Wall -Werror -W -fno-omit-frame-pointer -fno-common -fsigned-char $(GCOV_CCFLAGS) -I$(LLQUEUE_DIR)

all: tests

clinkedlistqueue:
	mkdir -p $(LLQUEUE_DIR)/.git
	git --git-dir=$(LLQUEUE_DIR)/.git init 
	pushd $(LLQUEUE_DIR); git pull git@github.com:willemt/CLinkedListQueue.git; popd

download-contrib: clinkedlistqueue

main.c:
	sh make-tests.sh > main.c

tests: main.c quadtree.c $(LLQUEUE_DIR)/linked_list_queue.c CuTest.c 
	$(CC) $(CCFLAGS) -o $@ $^
	./tests

#quadtree.o: quadtree.c 
#	$(CC) -g -c -Iclinkedlistqueue -o $@ $^

clean:
	rm -f main.c quadtree.o tests $(GCOV_OUTPUT)
