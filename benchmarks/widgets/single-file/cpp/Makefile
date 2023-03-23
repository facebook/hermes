.PHONY: all clean

all: build/widgets build/widgets_with_fakemalloc

clean:
	rm -r build

build/widgets:
	mkdir -p build
	c++ widgets.cpp -o build/widgets -std=c++17 -O3 -w

build/libfakemalloc.so:
	mkdir -p build
	c++ fakemalloc.cpp -o build/libfakemalloc.so -std=c++17 -shared -fPIC -O3 -D_GNU_SOURCE -ldl

build/widgets_with_fakemalloc: build/libfakemalloc.so
	c++ widgets.cpp -o build/widgets_with_fakemalloc -std=c++17 -O3 -w $(CURDIR)/build/libfakemalloc.so
