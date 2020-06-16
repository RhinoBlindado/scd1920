.SUFFIXES:
.PHONY: x1, x2, clean

compilador:=g++
opcionesc:= -std=c++11 -pthread -Wfatal-errors
hmonsrcs:= HoareMonitor.h HoareMonitor.cpp Semaphore.h Semaphore.cpp

x1: fumadores_exe
	./$<

x2: barbero_old_exe
	./$<

x3: barbero_exe
	./$<

x4: barberosala_exe
	./$<
x5: fumadores2_exe
	./$<

fumadores_exe: fumadores.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp

barbero_old_exe: barbero_old.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp

barbero_exe: barbero.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp

barberosala_exe: barbero_salaEsperaLimit.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp
fumadores2_exe: fumadores_multiplesEst.cpp $(hmonsrcs)
	$(compilador) $(opcionesc)  -o $@ $<  HoareMonitor.cpp Semaphore.cpp
clean:
	rm -f *_exe
