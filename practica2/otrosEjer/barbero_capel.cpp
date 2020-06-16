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
int totalBarb = 3, totalCli = 5;

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
class Barberia : public HoareMonitor
{
    private:
        static const int numCli = 5;
        CondVar sillaA,sillaB[numCli],esperaCobro[numCli],barberoEspera;
        int barberos,espera;
        bool esperando[numCli] = {false};
        bool terminado[numCli] = {false};
        bool cobrado[numCli] = {false};


    public:
        Barberia();
        void corte_de_pelo(int i);
        int siguente_cliente(int i);
        void termina_corte_de_pelo(int i, int j);
} ;

// PROCEDIMIENTOS MONITOR
Barberia::Barberia()
{
    for (int i = 0; i < totalCli; i++)
    {
        sillaB[i] = newCondVar();
        esperaCobro[i] = newCondVar();
    }
    sillaA = newCondVar();
    barberoEspera = newCondVar();
    barberos = 0;
    espera = 0;
       
}

void Barberia::corte_de_pelo(int i)
{
    cout<<"Cliente "<<i<<": Entra en barberia."<<endl;
    if (barberos == 0)
    {
        cout<<"Cliente "<<i<<": No hay barbero, espero en silla A."<<endl;
        sillaA.wait();
    }

    cout<<"Cliente "<<i<<": Hay barbero, espero en silla B."<<endl;
    espera++;
    esperando[i] = true;
    barberoEspera.signal();

    cout<<"Cliente "<<i<<": Espero que me corte el cabello."<<endl;
    if (!terminado[i])
    {
        sillaB[i].wait();
    }

    cout<<"Cliente "<<i<<": Le cobro al barbero."<<endl;
    cobrado[i] = true;
    esperaCobro[i].signal();

    cout<<"Cliente "<<i<<": Salgo de la barberia."<<endl;
    
}

int Barberia::siguente_cliente(int i)
{
    cout<<"Barbero "<<i<<": Entra en barberia."<<endl;
    barberos++;
    cout<<"Barbero "<<i<<": Reviso si hay clientes."<<endl;
    if (espera == 0)
    {
        cout<<"Barbero "<<i<<": No hay gente esperando en B, reviso A."<<endl;
        if (sillaA.empty())
        {
            cout<<"Barbero "<<i<<": No hay nadie. Espero."<<endl;
            barberoEspera.wait();
        }
        else
        {
            cout<<"Barbero "<<i<<": Hay gente en A. Les aviso que vayan a B."<<endl;
            sillaA.signal();
        }
    }

    cout<<"Barbero "<<i<<": Busco un cliente que este esperando y no haya terminado su corte."<<endl;

    bool encontrado=false;
    int j=0;
    int cliente;
    while (!encontrado)
    {
        if(esperando[j] && !terminado[j])
        {
            cliente = j;
            encontrado = true;
        }
        else
        {
            j++;
        }
    }
    cout<<"Barbero "<<i<<": Empiezo a cortarle el pelo al cliente "<<cliente<<"."<<endl;
    espera--;
    esperando[j] = false;
    return cliente;
}

void Barberia::termina_corte_de_pelo(int i,int j)
{
    cout<<"Barbero "<<j<<": Finaliza el corte de pelo del cliente "<<i<<"."<<endl;
    terminado[i] = true;
    cout<<"Barbero "<<j<<": Aviso cliente "<<i<<" para que se salga de la silla."<<endl;
    sillaB[i].signal();

    if (!cobrado[i])
    {
        cout<<"Barbero "<<j<<": Espero el cobro."<<endl;
        esperaCobro[i].wait();
    }
    cout<<"Barbero "<<j<<": Salgo de la barberia."<<endl;
    terminado[i] = false;
    barberos--;
}

// BARBERO
void cortarPeloCliente()
{
    chrono::milliseconds pelar( aleatorio<200,300>() );
    cout << "Barbero comienza a pelar cliente..."<< endl;
    this_thread::sleep_for(pelar);
    cout << "...Barbero finaliza de pelar cliente"<<endl;

}

void funcion_hebra_barbero(MRef<Barberia> monitor, int barb)
{
    int i;
    while (true)
    {
        i = monitor->siguente_cliente(barb);
        cortarPeloCliente();
        monitor->termina_corte_de_pelo(i,barb);
    }

}

// CLIENTE
void esperarFueraBarberia(int i)
{
    cout << "Cliente "<< i <<" se va a dar un paseo..."<< endl;
    chrono::milliseconds paseo( aleatorio<400,500>() );
    this_thread::sleep_for(paseo);
    cout << "...Cliente "<< i <<" vuelve de dar un paseo."<< endl;
}

void funcion_hebra_cliente(MRef<Barberia> monitor, int cli)
{
    while (true)
    {
        monitor->corte_de_pelo(cli);
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
