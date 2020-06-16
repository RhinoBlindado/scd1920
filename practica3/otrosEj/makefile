.SUFFIXES:
.PHONY:    clean

compilador := mpicxx -std=c++11
flagsc     := -g -Wall
nm := foo
n := 5

pc: $(nm)_exe
	mpirun --oversubscribe -np $(n) ./$<

%_exe: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
