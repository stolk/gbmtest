CFLAGS = -Wall -Wextra -g 

run: nv12test
	echo `./nv12test`

nv12test: nv12test.o
	#$(CC) nv12test.o -o nv12test -L. -lgbm
	$(CC) nv12test.o -o nv12test -L. -lminigbm.pie -lgbm -ldrm
	#$(CC) nv12test.o -o nv12test -L. -lminigbm -lgbm -ldrm

preloadtest: nv12test
	LD_PRELOAD=/usr/src/minigbm/libminigbm.so.1.0.0 ./nv12test


mesatest: nv12test
	LD_LIBRARY_PATH=/opt/mesa/lib/x86_64-linux-gnu ./nv12test

clean:
	rm -f *.o nv12test

