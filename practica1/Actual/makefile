.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

pc: prodcons_exe
	./$<

fu: fumadores_exe
	./$<
#para compilar: make <nombre-prog>_exe <nombre-prog>.cpp
%_exe: %.cpp
		$(compilador) -std=c++11 -pthread -o $@ $< Semaphore.cpp 

clean:
	rm -f *_exe
