all: build/Makefile
	make -C build

build/Makefile:
	mkdir -p build
	cd build && cmake ..

test:
	mkdir -p build
	cd build && cmake .. -DENABLE_TESTS=1
	make -C build
	for test in bin/tests/*; do ./$$test; done
	echo -e "\e[32mTESTS OK\e[0m"

dev:
	fswatch -m poll_monitor -o -0 src/* tests/* | xargs -0 -n1 -I{} make test | true

clean:
	rm -rf build/*
