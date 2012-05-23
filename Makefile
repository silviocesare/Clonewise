LDFLAGS = -lrt -lm -lfuzzy
INCLUDES = -I libs/snap -I libs/glib -I src -I libs/munkres-2
CFLAGS = -O3 -fopenmp
INSTALL = install

CC = g++

all: Clonewise
#Clonewise-RunTests-Statistics

Clonewise: src/Clonewise.cpp src/main.cpp
	$(CC) $(CFLAGS) -o bin/Clonewise libs/munkres-2/munkres.cpp libs/snap/cliques.cpp libs/snap/Snap.cpp src/main.cpp src/Clonewise.cpp $(INCLUDES) $(LDFLAGS)

Clonewise-RunTests-Statistics: src/Clonewise.cpp src/Clonewise-RunTests-Statistics.cpp
	$(CC) $(CFLAGS) -o bin/Clonewise-RunTests-Statistics libs/snap/cliques.cpp libs/snap/Snap.cpp src/Clonewise-RunTests-Statistics.cpp src/Clonewise.cpp $(INCLUDES) $(LDFLAGS)

install:
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/features
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/signatures
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise/downloads
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) -m644 ./config/* $(DESTDIR)/var/lib/Clonewise
	$(INSTALL) ./bin/* $(DESTDIR)/usr/bin

clean:
	rm -f *.o bin/Clonewise
