DEBUG = -g 
release : override DEBUG = -O3

# if not g++ we assume clang++
DEFAULT_LIBS = -stdlib=libc++ -lc++abi -std=c++20
DEFAULT_OPTS = -std=c++20 -stdlib=libc++ -D_RESUMABLE_FUNCTIONS_SUPPORTED 

ifeq ($(CXX),g++)
DEFAULT_OPTS = -std=c++20
DEFAULT_LIBS = 
endif

OPTIONS = -Wno-write-strings -pthread -D_REENTRANT $(DEFAULT_OPTS)
CPPFLAGS = -Wall -I../include $(DEBUG) $(OPTIONS) 

TESTFLAGS = -g

TEST_SRC   = .
TEST_BUILD = ./build
TEST_LIBS  = /usr/lib/x86_64-linux-gnu/libgtest.a -lpthread $(DEFAULT_LIBS)

# Objects for the test executable
TEST_SRCFILESABS = $(shell ls $(TEST_SRC)/*.cpp)
TEST_SRCFILES =  $(notdir $(TEST_SRCFILESABS))
TEST_OBJFILES = $(TEST_SRCFILES:%.cpp=$(TEST_BUILD)/%.o)
TEST_BINS = $(TEST_SRCFILES:%.cpp=$(TEST_BUILD)/%.bin)

#################################################
# rule to compile all (default rule)
#################################################

all: $(TEST_BINS)

#################################################
# rules to compile .o files from .cpp source
#################################################

$(TEST_BUILD)/%.o: $(TEST_SRC)/%.cpp 
	-mkdir -p $(TEST_BUILD)
	$(CXX) -c $^ -o $@ $(CPPFLAGS) $(TESTFLAGS)


#################################################
# rules to compile the artifacts from .o files
#################################################

$(TEST_BUILD)/%.bin: $(TEST_BUILD)/%.o 
	$(CXX) $(LIB_OBJFILES) $^ $(TEST_LIBS) -o $@ 
			
#################################################
# make clean
#################################################

clean:
	-find -name "*.o" -exec rm {} \;
	-find -name "*.bin" -exec rm {} \;
	-find -name "*~" -exec rm {} \;


release : all


