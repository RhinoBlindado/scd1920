
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

//	Funciones Auxiliares
//	Funcion RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//	Monitor
class MonFilo : public HoareMonitor
{
    private:
    int tenedor[5] = {1,1,1,1,1};
    int primer_ten;
    CondVar filo,espera_tenedor[5];

    void tenedores();
    public:
	MonFilo();
    void coger_tenedor(int num_ten, int num_proc);
    void libera_tenedor(int num_ten, int num_proc);
} ;

//	Funciones Monitor
MonFilo::MonFilo()
{
    primer_ten = 0;
    filo = newCondVar();

    for (int i = 0; i < 5; i++)
    {
        espera_tenedor[i] = newCondVar();
    }    
}

void MonFilo::tenedores()
{
    cout <<"Tenedores: ";
    for (int i = 0; i < 5; i++)
    {
        cout<<tenedor[i]<<" ";
    }
    cout<<endl;   
}


void MonFilo::coger_tenedor(int num_ten, int num_proc)
{
    cout << "Filosofo "<<num_proc<<" : Desea tomar tenedor "<<num_ten<<"."<<endl;
    this->tenedores();
    if (num_ten == num_proc)
    {
        cout << "Filosofo "<<num_proc<<" : Primer tenedor."<<endl;    
        primer_ten++;
        if (primer_ten > 3)
        {
            cout << "Filosofo "<<num_proc<<" : Hay 4 filosofos ya intentando. Espero."<<endl;    
            filo.wait();
        }
        cout << "Filosofo "<<num_proc<<" : Reviso si esta el tenedor."<<endl;    
        if(tenedor[num_ten] == 0)
        {
            cout << "Filosofo "<<num_proc<<" : No esta. Espero."<<endl;    
            espera_tenedor[num_ten].wait();
        }
    }
    else
    {
        cout << "Filosofo "<<num_proc<<" : Segundo tenedor."<<endl;    
        cout << "Filosofo "<<num_proc<<" : Reviso si esta el tenedor."<<endl;    
        if (tenedor[num_ten] == 0)
        {
            cout << "Filosofo "<<num_proc<<" : No esta. Espero."<<endl;    
            espera_tenedor[num_ten].wait();
        }
        cout << "Filosofo "<<num_proc<<" : Aviso que ya hay menos filosofos intentando."<<endl;
        primer_ten--;
        filo.signal();
    }

    cout << "Filosofo "<<num_proc<<" : Cojo el tenedor "<<num_ten<<"."<<endl;    

    tenedor[num_ten] = 0;
    this->tenedores();

}

void MonFilo::libera_tenedor(int num_ten, int num_proc)
{
    cout << "Filosofo "<<num_proc<<" : Libero el tenedor y aviso."<<num_ten<<"."<<endl;    
    tenedor[num_ten] = 1;
    this->tenedores();
    espera_tenedor[num_ten].signal();   
}

//	Funciones Hebras

void comer(int i)
{
    chrono::milliseconds comer( aleatorio<500,800>() );
	cout << "Filosofo "<<i<<" : Empieza a comer"<<endl;
	this_thread::sleep_for(comer);
}

void pensar(int i)
{
    chrono::milliseconds pensar( aleatorio<400,750>() );
	cout << "Filosofo "<<i<<" : Empieza a pensar"<<endl;
	this_thread::sleep_for(pensar);
}

void funcionFilosofo(MRef<MonFilo> monitor, int i)
{
    while (true)
    {
        monitor->coger_tenedor(i,i);
        monitor->coger_tenedor((i+1)%5,i);
        comer(i);
        monitor->libera_tenedor(i,i);
        monitor->libera_tenedor((i+1)%5,i);
        pensar(i);
    }
}

//	Funcion principal
int main()
{
    MRef<MonFilo> monitor = Create<MonFilo>();
    thread hebraFilosofo[5];

    for(int i=0;i<5;i++)
	{
		hebraFilosofo[i] = thread(funcionFilosofo, monitor, i);
	}

	for(int i=0;i<5;i++)
	{
		hebraFilosofo[i].join();
	}
}
