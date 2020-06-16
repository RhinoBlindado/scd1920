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

int totalP = 5, totalC = 7;

//  FUNCIONES SECUNDARIAS
//      GENERAR NUM ALEATORIOS
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// MONITOR
class Mensaje : public HoareMonitor
{
    private:
        CondVar esperaSigM, esperaTrans, esperaFinT;
        int mensaje,solicitando;
        bool inicioTransmision,finTransmision;


    public:
        Mensaje();
        void broadcast(int m,int i);
        int fetch(int i);
} ;

// PROCEDIMIENTOS MONITOR
Mensaje::Mensaje()
{
    esperaSigM = newCondVar();
    esperaTrans = newCondVar();
    esperaFinT = newCondVar();
    mensaje = -1;
    solicitando = 0;
    inicioTransmision = false;
    finTransmision = true;
}

void Mensaje::broadcast(int m, int i)
{

    cout << "Productor "<< i <<" : Reviso si hay otro transmitiendo."<< endl;
    if(!finTransmision)
    {
        cout << "Productor "<< i <<": Hay transmision. Espero."<< endl;
        esperaSigM.wait();
    }

    cout << "Productor "<< i <<": Puedo transmitir. Empiezo."<< endl;

    inicioTransmision = true;
    finTransmision = false;

    cout << "Productor "<< i <<": Coloco el mensaje "<<m<<" en la red."<< endl;

    mensaje = m;


    if (solicitando > 0)
    {
        cout << "Productor "<< i <<": Aviso a hebras esperando."<< endl;
        esperaTrans.signal();
    }
    else
    {
        cout << "Productor "<< i <<": Espero que las hebras reciban el mensaje."<< endl;
        esperaFinT.wait();
    }


    cout << "Productor "<< i <<": Finalizo la transmision."<< endl;
    inicioTransmision = false;
    finTransmision = true;

    esperaSigM.signal();
    cout << "Productor "<< i <<": Salgo del monitor."<< endl;
    
}

int Mensaje::fetch(int i)
{
    int mR;

    cout << "Consumidor "<< i <<": Solicito un mensaje."<< endl;
    solicitando++;

    if (!inicioTransmision)
    {
        cout << "Consumidor "<< i <<": Productor no ha iniciado transmision. Espero."<< endl;
        esperaTrans.wait();
    }

    cout << "Consumidor "<< i <<": Hay transmision. Aviso a otras hebras y recibo el mensaje."<< endl;
    esperaTrans.signal();

    mR = mensaje;
    cout << "Consumidor "<< i <<": Recibido el mensaje "<< mR <<"."<< endl;
    solicitando--;
    if (solicitando == 0)
    {
        cout << "Consumidor "<< i <<": No hay mas solicitudes, aviso al productor."<< endl;
        esperaFinT.signal();
    }
    

    cout << "Consumidor "<< i <<": Salgo del monitor."<< endl;
    return mR;
}

// HEBRA

void utilizarM(int i, int cons)
{
    chrono::milliseconds Bar ( aleatorio<400,500>() );
    cout << "Consumidor "<< cons <<" consume el mensaje"<<i<<""<< endl;
    this_thread::sleep_for(Bar);
    cout << "Consumidor "<< cons <<"... ha consumido el mensaje."<< endl;
}

void funcion_consumidora(MRef<Mensaje> monitor, int cons)
{
    int i;
    while (true)
    {
        i = monitor->fetch(cons);
        utilizarM(i,cons);
    }
}

int mensaje(int i)
{
    chrono::milliseconds Bar ( aleatorio<400,500>() );
    cout << "Productor "<< i <<" empieza a producir un mensaje."<< endl;
    this_thread::sleep_for(Bar);
    cout << "Productor "<< i <<"... ha producido un mensaje."<< endl;
    return aleatorio<0,1000>();
}

void funcion_productora(MRef<Mensaje> monitor, int prod)
{
    int i;
    while (true)
    {
        i = mensaje(prod);
        monitor->broadcast(i,prod);
    }
    
}

// MAIN
int main(int argc, char const *argv[])
{
    MRef<Mensaje> monitor = Create<Mensaje>();
    thread hebra_prod[totalP], hebra_cons[totalC];

    for(int i=0;i<totalP;i++)
	{
		hebra_prod[i] = thread(funcion_productora, monitor, i);
	}

	for(int i=0;i<totalC;i++)
	{
		hebra_cons[i] = thread(funcion_consumidora,monitor,i);
	}


	for(int i=0;i<totalP;i++)
	{
		hebra_prod[i].join();
	}

	for(int i=0;i<totalC;i++)
	{
		hebra_cons[i].join();
	}


    return 0;
}
