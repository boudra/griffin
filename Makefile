C=clang
C_FLAGS=-O0 -Wall -Wno-long-long -g -pedantic -std=c11

app: *.c *.h
	$(C) *.c ${C_FLAGS} -o app

run: app
	./app

debug: app
	gdb app

clean:
	rm app
