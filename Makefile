LDFLAGS = -lrt -lm -lfuzzy -lxerces-c
INCLUDES = -I src -I libs/munkres-2 -I src/Core -I src/SimilarityMeasures/StringMetricsAndMatching -I src/FuzzyHashing -I src/Launcher
CFLAGS = -fopenmp -g
INSTALL = install
WGET = wget
CC = g++
CP = cp
RM = rm
MPICC = mpic++

SOURCES =	src/Core/Clonewise-Cache.cpp \
		src/Core/Clonewise-query-embedded-cache.cpp \
		src/Core/Clonewise.cpp \
		src/Core/Clonewise-lib-Cache.cpp \
		libs/munkres-2/munkres.cpp \
		src/Core/Clonewise-main.cpp \
		src/Core/Clonewise-MakeCache.cpp \
		src/Core/Clonewise-make-embedded-cache.cpp \
		src/Core/Clonewise-query-source.cpp \
		src/Core/Clonewise-query-embedded.cpp \
		src/Core/Clonewise-find-file.cpp \
		src/Core/Clonewise-find-license-problems.cpp \
		src/Core/Clonewise-BugInferrer.cpp \
		src/Core/Clonewise-ParseDatabase.cpp \
		src/Launcher/main.cpp \
		src/SimilarityMeasures/StringMetricsAndMatching/StringMetrics.cpp

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
