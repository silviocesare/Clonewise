LDFLAGS = -lrt -lm -lfuzzy
INCLUDES = -I libs/snap -I libs/glib -I src -I libs/munkres-2
CFLAGS = -fopenmp -g
INSTALL = install

CC = g++
MPICC = mpic++

all: Clonewise-MakeCache Clonewise Clonewise-Cache

Clonewise-Cache: src/Clonewise-Cache.cpp
	$(CC) -o bin/Clonewise-Cache src/Clonewise-Cache.cpp $(INCLUDES) $(LDFLAGS)

Clonewise: src/Clonewise.cpp src/main.cpp
	$(CC) $(CFLAGS) -o bin/Clonewise libs/munkres-2/munkres.cpp libs/snap/cliques.cpp libs/snap/Snap.cpp src/main.cpp src/Clonewise.cpp $(INCLUDES) $(LDFLAGS)

Clonewise-MakeCache: src/Clonewise.cpp src/Clonewise-MakeCache.cpp
	$(MPICC) $(CFLAGS) -o bin/Clonewise-MakeCache libs/munkres-2/munkres.cpp libs/snap/cliques.cpp libs/snap/Snap.cpp src/Clonewise-MakeCache.cpp src/Clonewise.cpp $(INCLUDES) $(LDFLAGS)

install:
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/features
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/signatures
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/downloads
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/cache
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/weka
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) -m644 ./config/* $(DESTDIR)/var/lib/Clonewise
	$(INSTALL) ./bin/* $(DESTDIR)/usr/bin

clean:
	rm -f *.o bin/Clonewise
