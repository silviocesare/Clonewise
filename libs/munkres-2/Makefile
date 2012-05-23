CXX = g++
# change CXXTESTGEN and CXXTESTINCLUDEDIR to appropriate directories.
CXXTESTGEN = cxxtest/cxxtestgen.pl
CXXTESTGENFLAGS = --abort-on-fail --have-eh
CXXTESTINCLUDEDIR = cxxtest

CXXFLAGS = -Wall -I. # -ggdb3 -DDEBUG

PROGRAM = demo

SOURCES = main.cpp munkres.cpp
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
HEADERS := $(filter-out main.h,$(patsubst %.cpp,%.h,$(SOURCES))) matrix.h
UNITTESTS = unittests/singlesolution.h unittests/validsolution.h

# implementation

.SUFFIXES:      .o .cpp

.cpp.o : $(HEADERS)
	$(CXX) $(CXXFLAGS) -c  -o $@ $<

all:	$(PROGRAM)

$(PROGRAM): $(OBJECTS) $(HEADERS)
	$(CXX) -o $(PROGRAM) $(OBJECTS)

test:	testsuite
	@echo "Running test suite."
	@./testsuite

testsuite: testsuite.cpp $(filter-out main.o,$(OBJECTS))
	$(CXX) -I$(CXXTESTINCLUDEDIR) $(CXXFLAGS) -o $@ $^

testsuite.cpp:	$(UNITTESTS)
	$(CXXTESTGEN) $(CXXTESTGENFLAGS) -o $@ --error-printer $^

clean:
	rm -f $(OBJECTS)
	rm -f $(PROGRAM)
	rm -f testsuite
	rm -f testsuite.cpp

