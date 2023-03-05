CC = g++
BASE_FLAGS = -O2 -Wall
# CMSSW_FLAGS = -I$(CMSSW_BASE)/src -L$(CMSSW_BASE)/lib/$(SCRAM_ARCH) -lFWCoreFWLite -lDataFormatsFWLite
ROOT_FLAGS = `root-config --cflags --libs`
LOCAL_INCLUDE = -I$(HOME)/local/include
LOCAL_LIB = -L$(HOME)/local/lib -lyaml-cpp
PROJECT_INCLUDE = -I./include
LOCAL_FLAGS = $(LOCAL_INCLUDE) $(LOCAL_LIB) $(PROJECT_INCLUDE)
CFLAGS = $(BASE_FLAGS) $(ROOT_FLAGS) $(LOCAL_FLAGS) #$(CMSSW_FLAGS)

SOURCES = $(wildcard src/*.cc)
OBJECTS = $(patsubst src/%.cc, obj/%.o, $(SOURCES))
HEADERS = $(wildcard include/*.h)

$(shell mkdir -p obj)
$(shell mkdir -p bin)

bin/main.exe: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o bin/main.exe

obj/%.o: src/%.cc $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean run

run: bin/main.exe
	./bin/main.exe

clean:
	rm -f *.exe obj/*.o bin/*.exe
