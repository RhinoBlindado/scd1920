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
int totalBarb = 1, totalCli = 4, sala=2;

// VARIABLES AUXILIARES


//  FUNCIONES AUXILIARES
//      GENERAR NUM ALEATORIOS
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// MONITOR
class Barberia : public HoareMonitor
{
    private:
        CondVar dormido,salaEspera,silla,afuera;
        int clientes,cantSala;

    public:
        Barberia();
        void cortarPelo(int i);
        void siguienteCliente();
        void finCliente();
        void entrarBarberia(int i);
} ;

// PROCEDIMIENTOS MONITOR
Barberia::Barberia()
{
    salaEspera = newCondVar();
    dormido = newCondVar();
    silla = newCondVar();
    afuera = newCondVar();
    cantSala = sala;
    clientes = 0;
}

void Barberia::entrarBarberia(int i)
{
    cout << "Cliente "<<i<<" : Revisa si hay puesto dentro o no hay cola."<<endl;
    cout << "Cliente "<<i<<", Gente en sala: "<<clientes<<endl;
    clientes++;
    // Si esta llena la sala o hay gente en la cola de afuera.
    if(clientes > cantSala || !afuera.empty())
    {
        cout << "Cliente "<<i<<" : Hay gente esperando, se pone en la cola."<<endl;
        afuera.wait();
    }
    cout << "Cliente "<<i<<" : Entra en la barberia."<<endl;
}

void Barberia::cortarPelo(int i)
{
    // El cliente entra en la barberia.
    // Si se encuentra que la silla esta en uso, se va a esperar
    // a la sala de espera.


    cout << "Cliente "<<i<<" : chequea el barbero."<<endl;

    if(!silla.empty())
    {
        cout << "Cliente "<<i<<" : Silla en uso, va a sala de espera."<<endl;
        // Me siento en la sala hasta que el barbero me llame.
        salaEspera.wait();
        afuera.signal();
        cout << "Cliente "<<i<<" : Barbero lo llama, empieza a pelarse."<<endl;
        // Me ha llamado el barbero, voy a la silla.
        silla.wait();
    }
    else // La silla esta vacia, entonces despierto al barbero.
    {
        cout << "Cliente "<<i<<" : Barbero dormido, lo despierta."<<endl;
        // Despierto al barbero.
        afuera.signal();
        dormido.signal();
        // Me siento en la silla.
        cout << "Cliente "<<i<<" : Se sienta en la silla."<<endl;
        silla.wait();
    }
    clientes--;
}

void Barberia::siguienteCliente()
{
    cout << "Barbero: revisa si hay clientes."<<endl;

    // Si no hay clientes en la sala de espera o en la silla.

    if(silla.empty())
    {
        if(salaEspera.empty())
        {
            cout << "Barbero: No hay clientes, se va a dormir."<<endl;
            dormido.wait();
            cout << "Barbero: Ha llegado un cliente y lo ha despertado."<<endl;

        }
        else
        {
            cout << "Barbero: hay clientes, llama a uno."<<endl;
            salaEspera.signal();
        }
    }
    else
    {
        cout << "Barbero: ya hay un cliente en la silla." <<endl;
    }

    cout <<"Barbero: se prepara para pelar."<<endl;
}

void Barberia::finCliente()
{
    // He finalizado de pelar al cliente, se lo comunico.
    silla.signal();
}

// BARBERO
void cortarPeloCliente()
{
    chrono::milliseconds pelar( aleatorio<500,1000>() );
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
    chrono::milliseconds paseo( aleatorio<400,1200>() );
    this_thread::sleep_for(paseo);
    cout << "...Cliente "<< i <<" vuelve de dar un paseo."<< endl;
}

void funcion_hebra_cliente(MRef<Barberia> monitor, int cli)
{
    while (true)
    {
        monitor->entrarBarberia(cli);
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
