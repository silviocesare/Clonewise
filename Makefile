LDFLAGS = -lrt -lm -lfuzzy -lxerces-c
INCLUDES = -I src -I libs/munkres-2
CFLAGS = -fopenmp -g
INSTALL = install
CC = g++
MPICC = mpic++

COMMON_SOURCES = src/Clonewise-Cache.cpp src/Clonewise.cpp src/Clonewise-lib-Cache.cpp libs/munkres-2/munkres.cpp src/Clonewise-main.cpp
CLONEWISE_SOURCES = $(COMMON_SOURCES) src/Clonewise-MakeCache.cpp src/main.cpp
BUGINFERRER_SOURCES = $(COMMON_SOURCES) src/Clonewise-BugInferrer.cpp

CLONEWISE_OBJECTS = $(CLONEWISE_SOURCES:.cpp=.o)
BUGINFERRER_OBJECTS = $(BUGINFERRER_SOURCES:.cpp=.o)

all: Clonewise Clonewise-BugInferrer

%.o : %.cpp
	$(MPICC) $(CFLAGS) $(INCLUDES) -c $< -o $@

Clonewise-BugInferrer: $(BUGINFERRER_OBJECTS)
	$(MPICC) $(CFLAGS) -o bin/Clonewise-BugInferrer $(BUGINFERRER_OBJECTS) $(LDFLAGS)

Clonewise: $(CLONEWISE_OBJECTS)
	$(MPICC) $(CFLAGS) -o bin/Clonewise $(CLONEWISE_OBJECTS) $(LDFLAGS)

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
	rm -f *.o bin/Clonewise bin/Clonewise-BugInferrer
