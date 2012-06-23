LDFLAGS = -lrt -lm -lfuzzy -lxerces-c
INCLUDES = -I src -I libs/munkres-2
CFLAGS = -fopenmp -g
INSTALL = install
WGET = wget
CC = g++
CP = cp
RM = rm
MPICC = mpic++

SOURCES =	src/Clonewise-Cache.cpp \
		src/Clonewise.cpp \
		src/Clonewise-lib-Cache.cpp \
		libs/munkres-2/munkres.cpp \
		src/Clonewise-main.cpp \
		src/Clonewise-MakeCache.cpp \
		src/Clonewise-BugInferrer.cpp \
		src/Clonewise-ParseDatabase.cpp \
		src/main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: Clonewise

%.o : %.cpp
	$(MPICC) $(CFLAGS) $(INCLUDES) -c $< -o $@

Clonewise: $(OBJECTS)
	$(MPICC) $(CFLAGS) -o bin/Clonewise $(OBJECTS) $(LDFLAGS)

install:
	$(INSTALL) -d $(DESTDIR)/var/lib/Clonewise
	$(CP) -ru config/* $(DESTDIR)/var/lib/Clonewise
	$(WGET) -O $(DESTDIR)/var/lib/Clonewise/bugs/cve/nvdcve-2012.xml http://static.nvd.nist.gov/feeds/xml/cve/nvdcve-2.0-2012.xml
	$(WGET) -O $(DESTDIR)/var/lib/Clonewise/bugs/cve/nvdcve-2011.xml http://static.nvd.nist.gov/feeds/xml/cve/nvdcve-2.0-2011.xml
	$(WGET) -O $(DESTDIR)/var/lib/Clonewise/bugs/cve/nvdcve-2010.xml http://static.nvd.nist.gov/feeds/xml/cve/nvdcve-2.0-2010.xml
	$(INSTALL) ./bin/* $(DESTDIR)/usr/bin

clean:
	$(RM) -f *.o bin/Clonewise bin/Clonewise-BugInferrer
