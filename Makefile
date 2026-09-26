CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -Iinclude
CORE = src/model.cpp src/format.cpp src/view.cpp
HOST = src/library.cpp src/eink.cpp
HEADERS = $(wildcard include/beattab/*.hpp)
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)

.PHONY: all test run validate clean sanitize
all: build/beattab build/tests
build:
	mkdir -p build
build/beattab: $(CORE) $(HOST) src/main.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SDL_CFLAGS) $(CORE) $(HOST) src/main.cpp $(SDL_LIBS) -o $@
build/tests: $(CORE) $(HOST) tests/tests.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) $(HOST) tests/tests.cpp -o $@
test: build/tests
	./build/tests
validate: build/beattab
	./build/beattab --validate
run: build/beattab
	./build/beattab
sanitize: | build
	$(CXX) $(CPPFLAGS) -std=c++17 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer $(CORE) $(HOST) tests/tests.cpp -o build/tests-sanitize
	./build/tests-sanitize
clean:
	rm -f build/beattab build/tests build/tests-sanitize

build/snapshots: $(CORE) src/library.cpp tests/snapshots.cpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) src/library.cpp tests/snapshots.cpp -o $@
.PHONY: snapshots golden
snapshots: build/snapshots
	./build/snapshots > build/snapshots.txt
golden: snapshots
	diff -u tests/golden.txt build/snapshots.txt
