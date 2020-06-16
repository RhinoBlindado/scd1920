.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

ej1: ej1_exe
	./$<

ej2: ej2_exe
	./$<

ej1-corr: ej1-corr_exe
	./$<

ej2-corr: ej2-corr_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -pthread -o $@ $< Semaphore.cpp 

clean:
	rm -f *_exe
