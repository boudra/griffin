all: build/Makefile
	make -C build

build/Makefile:
	mkdir -p build
	cd build && cmake ..

test:
	mkdir -p build
	cd build && cmake .. -DENABLE_TESTS=1
	make -C build
	for test in build/tests/*; do ./$$test; done

dev: test
	fswatch src/* tests/* | xargs -n1 make test

clean:
	rm -rf build/*
