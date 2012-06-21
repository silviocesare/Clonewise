LDFLAGS = -lrt -lm -lfuzzy -lxerces-c
INCLUDES = -I src -I libs/munkres-2
CFLAGS = -fopenmp -g
INSTALL = install

CC = g++
MPICC = mpic++

all: Clonewise-MakeCache Clonewise Clonewise-Cache Clonewise-BugInferrer

Clonewise-BugInferrer: src/Clonewise-Cache.cpp src/Clonewise.cpp
	$(CC) $(CFLAGS) -o bin/Clonewise-BugInferrer src/Clonewise-BugInferrer.cpp src/Clonewise-lib-Cache.cpp src/Clonewise.cpp libs/munkres-2/munkres.cpp $(INCLUDES) $(LDFLAGS)

Clonewise-Cache: src/Clonewise-Cache.cpp src/Clonewise.cpp
	$(CC) $(CFLAGS) -o bin/Clonewise-Cache src/Clonewise-lib-Cache.cpp src/Clonewise-Cache.cpp src/Clonewise.cpp libs/munkres-2/munkres.cpp $(INCLUDES) $(LDFLAGS)

Clonewise: src/Clonewise.cpp src/Clonewise-main.cpp
	$(CC) $(CFLAGS) -o bin/Clonewise libs/munkres-2/munkres.cpp src/Clonewise-main.cpp src/Clonewise.cpp $(INCLUDES) $(LDFLAGS)

Clonewise-MakeCache: src/Clonewise.cpp src/Clonewise-MakeCache.cpp
	$(MPICC) $(CFLAGS) -o bin/Clonewise-MakeCache libs/munkres-2/munkres.cpp src/Clonewise-MakeCache.cpp src/Clonewise.cpp $(INCLUDES) $(LDFLAGS)

install:
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/clones
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/bugs
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/clones/features
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/clones/signatures
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/clones/downloads
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/clones/cache
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/weka
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) -m644 ./config/default-distribution $(DESTDIR)/var/lib/Clonewise
	$(INSTALL) -m644 ./config/clones/extensions $(DESTDIR)/var/lib/Clonewise/clones
	$(INSTALL) -m644 ./config/clones/weka/model $(DESTDIR)/var/lib/Clonewise/clones/weka
	$(INSTALL) ./bin/* $(DESTDIR)/usr/bin

clean:
	rm -f *.o bin/Clonewise
