CFLAGS = -Wall -Wextra -g 

run: nv12test
	echo `./nv12test`

nv12test: nv12test.o
	$(CC) nv12test.o -o nv12test -lgbm

