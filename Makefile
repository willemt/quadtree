SHELL = /bin/bash
CC     = gcc


all: tests

clinkedlistqueue:
	mkdir -p clinkedlistqueue/.git
	git --git-dir=clinkedlistqueue/.git init 
	pushd clinkedlistqueue; git pull git@github.com:willemt/CLinkedListQueue.git; popd

download-contrib: clinkedlistqueue

main.c:
	sh make-tests.sh > main.c

tests: main.c quadtree.c clinkedlistqueue/linked_list_queue.c CuTest.c 
	$(CC) -g -Iclinkedlistqueue -o $@ $^
	./tests

#quadtree.o: quadtree.c 
#	$(CC) -g -c -Iclinkedlistqueue -o $@ $^

clean:
	rm -f main.c quadtree.o tests
