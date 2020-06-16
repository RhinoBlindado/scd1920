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

// VARIABLES GLOBALES
int totalBarb = 1, totalCli = 3;


// VARIABLES AUXILIARES


//  FUNCIONES AUXILIARES
//      GENERAR NUM ALEATORIOS
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void check()
{

}

// MONITOR
class Barberia : public HoareMonitor
{
    private:
        bool estaPelando;
        int genteEnSala,cliente;
        CondVar dormido,salaEspera,pelando;

    public:
        Barberia();
        void cortarPelo(int i);
        void siguienteCliente();
        void finCliente();
} ;

// PROCEDIMIENTOS MONITOR
Barberia::Barberia()
{
    salaEspera = newCondVar();
    dormido = newCondVar();
    pelando = newCondVar();
    estaPelando = false;
    genteEnSala = 0;
}

void Barberia::cortarPelo(int i)
{
    cout << "Cliente "<< i <<" entra en la barberia..."<< endl;
    genteEnSala++;
    if (estaPelando)
    {
        cout << "Cliente "<< i <<" se pone a esperar"<< endl;
        salaEspera.wait();
        genteEnSala--;
        cout << "Cliente "<< i <<" sale de esperar"<< endl;
    }
    
    cout << "Cliente "<< i <<" despierta al barbero."<< endl;
    dormido.signal();
    pelando.wait();
    cout << "Cliente "<< i <<" sale de la barberia..."<< endl;
}

void Barberia::siguienteCliente()
{
    cout << "Barbero: revisa si hay clientes..." <<endl;
    if(genteEnSala == 0)
    {
        cout << "Barbero: No hay clientes; se va a dormir."<<endl;
        dormido.wait();
        cout << "Barbero: Un cliente lo despierta."<<endl;
    }
    
    cout << "Barbero: Le dice al cliente que se siente..."<<endl;
    estaPelando = true;
    salaEspera.signal();
}

void Barberia::finCliente()
{
    cout << "Barbero: Notifica cliente para que libere silla..."<<endl;
    estaPelando = false;
    pelando.signal();
}

// BARBERO
void cortarPeloCliente()
{
    chrono::milliseconds pelar( aleatorio<0,0>() );
    cout << "Barbero comienza a pelar cliente..."<< endl;
    this_thread::sleep_for(pelar);
    cout << "...Barbero finaliza de pelar cliente"<<endl;

}

void funcion_hebra_barbero(MRef<Barberia> monitor, int barb)
{
    while (true)
    {
        monitor->siguienteCliente();
        cortarPeloCliente();
        monitor->finCliente();
    }
    
}

// CLIENTE
void esperarFueraBarberia(int i)
{
    cout << "Cliente "<< i <<" se va a dar un paseo..."<< endl;
    chrono::milliseconds paseo( aleatorio<0,0>() );
    this_thread::sleep_for(paseo);
    cout << "...Cliente "<< i <<" vuelve de dar un paseo."<< endl;
}

void funcion_hebra_cliente(MRef<Barberia> monitor, int cli)
{
    while (true)
    {
        monitor->cortarPelo(cli);
        esperarFueraBarberia(cli);
    }
    
}

int main()
{
    MRef<Barberia> monitor = Create<Barberia>();
    thread hebra_barbero[totalBarb], hebra_cliente[totalCli];

    for(int i=0;i<totalBarb;i++)
	{
		hebra_barbero[i] = thread(funcion_hebra_barbero, monitor, i);
	}

	for(int i=0;i<totalCli;i++)
	{
		hebra_cliente[i] = thread(funcion_hebra_cliente,monitor,i);
	}


	for(int i=0;i<totalBarb;i++)
	{
		hebra_barbero[i].join();
	}

	for(int i=0;i<totalCli;i++)
	{
		hebra_cliente[i].join();
	}
}