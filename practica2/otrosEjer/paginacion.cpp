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

// VARIABLES AUXILIARES
int M = 2048,
    totalA=5;


//  FUNCIONES SECUNDARIAS
//      GENERAR NUM ALEATORIOS
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// MONITOR
class Paginacion : public HoareMonitor
{
    private:
        int memL;
        CondVar esperaMem, otraCola;
    public:
        Paginacion();
        void adquirir(int n,int i);
        void liberar(int n, int i);
} ;

// PROCEDIMIENTOS MONITOR

Paginacion::Paginacion()
{
    memL = M;
    esperaMem = newCondVar();
    otraCola = newCondVar();
}

void Paginacion::adquirir(int n,int i)
{
    cout << "Hebra "<< i <<": Reviso si puedo pedir memoria."<< endl;
    if(!esperaMem.empty())
    {
        cout << "Hebra "<< i <<": Otro proceso esta esperando, yo espero."<< endl;
        otraCola.wait();
    }

    cout << "Hebra "<< i <<": Reviso si hay memoria suficiente. Hay "<<memL<<" y yo pido "<<n<<" ."<< endl;

    while (n > memL)
    {
        cout << "Hebra "<< i <<": No hay. Espero."<< endl;
        esperaMem.wait();
    }
    
    cout << "Hebra "<< i <<": Obtengo la memoria que pido."<< endl;
    memL -= n;
    otraCola.signal();
    
}

void Paginacion::liberar(int n, int i)
{
    memL += n;
    cout << "Hebra "<< i <<": Libero "<<n<<" KB de memoria. Total libre: "<<memL<<" ."<< endl;
    esperaMem.signal();
}

// HEBRAS

void usoMemoria(int i)
{
    chrono::milliseconds Bar ( aleatorio<400,500>() );
    cout << "Hebra "<< i <<": Usa la memoria..."<< endl;
    this_thread::sleep_for(Bar);
    cout << "Hebra "<< i <<": Finaliza de usar la memoria..."<< endl;
}

int usoMemoria()
{
    return aleatorio<1,2048>();
}


void funcion_hebra_A(MRef<Paginacion> monitor, int A)
{
    int i;
    while (true)
    {
        i = usoMemoria();
        monitor->adquirir(i,A);
        usoMemoria(A);
        monitor->liberar(i,A);
    }
}

// MAIN

int main()
{
    MRef<Paginacion> monitor = Create<Paginacion>();
    thread hebra_A[totalA];

    for (size_t i = 0; i < totalA; i++)
    {
        hebra_A[i] = thread(funcion_hebra_A,monitor,i);
    }

    for (size_t i = 0; i < totalA; i++)
    {
        hebra_A[i].join();
    }
}