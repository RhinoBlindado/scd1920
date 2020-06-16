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


//  FUNCIONES SECUNDARIAS
//      GENERAR NUM ALEATORIOS
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// MONITOR
class Foo : public HoareMonitor
{
    private:

    public:
        Foo();
        void proc_1();
        void proc_2();
} ;

// PROCEDIMIENTOS MONITOR

Foo::Foo()
{

}

void Foo::proc_1()
{

}

void Foo::proc_2()
{

}

// HEBRAS

void espera(int i)
{
    chrono::milliseconds Bar ( aleatorio<400,500>() );

    this_thread::sleep_for(Bar);

    cout << "HEBRA "<< i <<" ACCION "<< endl;
}


void funcion_hebra_A(MRef<Foo> monitor, int A)
{
    while (true)
    {
        /* code */
    }
    
}

// MAIN

int main()
{
    MRef<Foo> monitor = Create<Foo>();
    thread hebra_A[totalA], hebra_X[totalX];

    for (size_t i = 0; i < totalA; i++)
    {
        hebra_A[i] = thread(funcion_hebra_A,monitor,i);
    }
    
    for (size_t i = 0; i < totalX; i++)
    {
        hebra_X[i] = thread(funcion_hebra_X,monitor,i);
    }
    

    for (size_t i = 0; i < totalA; i++)
    {
        hebra_A[i].join();
    }

    for (size_t i = 0; i < totalX; i++)
    {
        hebra_X[i].join();
    }
    
    
}