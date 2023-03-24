.PHONY: all clean

all: build/widgets build/widgets_with_fakemalloc build/widgets_fast build/widgets_fast_with_fakemalloc build/widgets_fast_int build/widgets_fast_int_with_fakemalloc

clean:
	rm -r build

run: all
	./build/widgets
	./build/widgets
	./build/widgets_with_fakemalloc
	./build/widgets_with_fakemalloc
	./build/widgets_fast
	./build/widgets_fast
	./build/widgets_fast_with_fakemalloc
	./build/widgets_fast_with_fakemalloc
	./build/widgets_fast_int
	./build/widgets_fast_int
	./build/widgets_fast_int_with_fakemalloc
	./build/widgets_fast_int_with_fakemalloc

build/widgets:
	mkdir -p build
	c++ widgets.cpp -o build/widgets -std=c++17 -O3 -w

build/libfakemalloc.so:
	mkdir -p build
	c++ fakemalloc.cpp -o $(CURDIR)/build/libfakemalloc.so -std=c++17 -shared -fPIC -O3 -D_GNU_SOURCE -ldl

build/widgets_with_fakemalloc: build/libfakemalloc.so
	c++ widgets.cpp -o build/widgets_with_fakemalloc -std=c++17 -O3 -w $(CURDIR)/build/libfakemalloc.so

build/widgets_fast:
	mkdir -p build
	c++ widgets_fast.cpp -o build/widgets_fast -std=c++17 -O3 -w

build/widgets_fast_with_fakemalloc: build/libfakemalloc.so
	c++ widgets_fast.cpp -o build/widgets_fast_with_fakemalloc -std=c++17 -O3 -w $(CURDIR)/build/libfakemalloc.so

build/widgets_fast_int:
	mkdir -p build
	c++ widgets_fast_int.cpp -o build/widgets_fast_int -std=c++17 -O3 -w

build/widgets_fast_int_with_fakemalloc: build/libfakemalloc.so
	c++ widgets_fast_int.cpp -o build/widgets_fast_int_with_fakemalloc -std=c++17 -O3 -w $(CURDIR)/build/libfakemalloc.so
