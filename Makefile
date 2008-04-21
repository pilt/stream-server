CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -Os -I. -fPIC \
		   -Wno-long-long -DNDEBUG
LDFLAGS = 
SH=bash

LIVE_LIB_DIR = live-media
LIVE_LIBS = -L$(LIVE_LIB_DIR)/liveMedia -lliveMedia
LIVE_LIBS += -L$(LIVE_LIB_DIR)/UsageEnvironment -lUsageEnvironment
LIVE_LIBS += -L$(LIVE_LIB_DIR)/BasicUsageEnvironment -lBasicUsageEnvironment
LIVE_LIBS += -L$(LIVE_LIB_DIR)/groupsock -lgroupsock
LIVE_INCLUDES = -I$(LIVE_LIB_DIR)/liveMedia/include
LIVE_INCLUDES += -I$(LIVE_LIB_DIR)/UsageEnvironment/include
LIVE_INCLUDES += -I$(LIVE_LIB_DIR)/BasicUsageEnvironment/include
LIVE_INCLUDES += -I$(LIVE_LIB_DIR)/groupsock/include

CXXFLAGS += $(LIVE_INCLUDES)
LDFLAGS += $(LIVE_LIBS) 

# Try changing to `python2.4' if you can not build
CXXFLAGS += -I/usr/include/python2.5 
LDFLAGS += -lpython2.5

SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)
EXE = server
SHARED = _streamServer.so

all: live $(SRCS) $(EXE) $(SHARED)
RM=rm
tests: all

%.o: %.cc
	$(CXX) -c -o $*.o $(CXXFLAGS) $*.cc

_streamServer.so: $(OBJS)
	$(CXX) -fPIC -shared -o $@ $(OBJS) $(LDFLAGS)

$(EXE): $(OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(OBJS) 

clean:
	$(RM) $(OBJS) $(EXE) $(SHARED) 
	cd $(LIVE_LIB_DIR) ; make clean

bindings:
	swig -c++ -python -o streamServer.cc server.i

live:
	cd $(LIVE_LIB_DIR) ; make

fresh:
	$(RM) web.log *.pyc cherrypy/*.pyc cherrypy/lib/*.pyc cherrypy/wsgiserver/*.pyc

tests:
	@echo ======================================================================
	@echo TESTS
	@echo ======================================================================
	@echo
	@echo "Running C++ tests (no output means success)..."
	@./server
	@echo Done.
	@echo
	@echo Running Python unittests...
	@python test.py
	@echo

