/**/

// Librerias
#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

//	Variables globales
int 	numEsc = 3,
	numLec = 3;


//	Funciones Auxiliares
//	Funcion RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//	Monitor
class LecEsc : public HoareMonitor
{
    private:
	CondVar colaLec,colaEsc;
	int leyendo;
	bool escribiendo;
    public:
	LecEsc();
	void iniLec(int i);
	void finLec(int i);
	void iniEsc(int i);
	void finEsc(int i);

} ;

//	Funciones Monitor
LecEsc::LecEsc()
{
	leyendo = 0;
	escribiendo = false;
	colaLec = newCondVar();
	colaEsc = newCondVar();
}

void LecEsc::iniLec(int i)
{
	cout << "Lector "<<i<<" : Deseo leer."<<endl;
	if(escribiendo)
	{
		cout << "Lector "<<i<<" : Hay escritor escribiendo. Espero."<<endl;
		colaLec.wait();
	}


	leyendo++;
	colaLec.signal();
	cout << "Lector "<<i<<" : Voy a leer."<<endl;
}

void LecEsc::finLec(int i)
{
	leyendo--;
	cout << "Lector "<<i<<" : Termine de leer."<<endl;
	cout << "Leyendo: "<<leyendo<<endl;
	if(leyendo == 0)
	{
		cout << "Lector "<<i<<" : No hay lectores, aviso a escritor."<<endl;
		colaEsc.signal();
	}
	
}

void LecEsc::iniEsc(int i)
{
	cout << "Escritor "<<i<<" : Deseo escribir."<<endl;
	if(escribiendo || leyendo > 0)
	{
		cout << "Escritor "<<i<<" : Hay escritor o lectores. Espero."<<endl;
		colaEsc.wait();
	}
	cout << "Escritor "<<i<<" : Voy a escribir."<<endl;
	escribiendo=true;
}

void LecEsc::finEsc(int i)
{
	cout << "Escritor "<<i<<" : Finalizo de escribir."<<endl;
	escribiendo=false;
	
	if(!colaLec.empty())
	{
		cout << "Escritor "<<i<<" : Hay lectores en espera, aviso a uno."<<endl;
		colaLec.signal();	
	}
	else
	{
		cout << "Escritor "<<i<<" : No hay lectores en espera, aviso a escritor."<<endl;
		colaEsc.signal();
	}
}

//	Funciones Hebras
//	Escritor
void escribir(int i)
{
	chrono::milliseconds escritura( aleatorio<400,1000>() );
	cout << "Escritor "<<i<<" : Empieza a escribir"<<endl;
	this_thread::sleep_for(escritura);
}

void funEsc(MRef<LecEsc> monitor, int esc)
{
	while(true)
	{
		monitor->iniEsc(esc);
		escribir(esc);
		monitor->finEsc(esc);
	}
}


//	Lector
void leer(int i)
{
	chrono::milliseconds lectura( aleatorio<0,0>() );
	cout << "Lector "<<i<<" : Empieza a leer"<<endl;
	this_thread::sleep_for(lectura);
}
void funLec(MRef<LecEsc> monitor, int lec)
{	
	while(true)
	{
		monitor->iniLec(lec);
		leer(lec);
		monitor->finLec(lec);
	}
}

//	Funcion principal
int main()
{
    MRef<LecEsc> monitor = Create<LecEsc>();
    thread hebEsc[numEsc],hebLec[numLec];

    for(int i=0;i<numEsc;i++)
	{
		hebEsc[i] = thread(funEsc, monitor, i);
	}

	for(int i=0;i<numLec;i++)
	{
		hebLec[i] = thread(funLec, monitor,i);
	}


	for(int i=0;i<numEsc;i++)
	{
		hebEsc[i].join();
	}

	for(int i=0;i<numLec;i++)
	{
		hebLec[i].join();
	}
}
