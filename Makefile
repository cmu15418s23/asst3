OUTPUTDIR := bin/

CFLAGS := -std=c++14 -fvisibility=hidden -lpthread

ifeq (,$(CONFIGURATION))
	CONFIGURATION := release
endif

ifeq (debug,$(CONFIGURATION))
CFLAGS += -g
else
CFLAGS += -O2 -fopenmp
endif

SOURCES := src/*.cpp
HEADERS := src/*.h

TARGETBIN := nbody-$(CONFIGURATION)

.SUFFIXES:
.PHONY: all clean

all: $(TARGETBIN)

$(TARGETBIN): $(SOURCES) $(HEADERS)
	$(CXX) -o $@ $(CFLAGS) $(SOURCES) 

format:
	clang-format -i src/*.cpp src/*.h

clean:
	rm -rf ./nbody-$(CONFIGURATION)

check:	default
	./checker.pl

FILES = src/*.cpp \
		src/*.h

handin.tar: $(FILES)
	tar cvf handin.tar $(FILES)
