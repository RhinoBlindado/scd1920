#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

/*VARIABLES GLOBALES*/

// Numero de hebras por cada agente
int totalEst = 1,
	totalFum = 3;

/*FUNCION ALEATORIA*/
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

/*MONITOR*/
class Estanco : public HoareMonitor
{
 	private:
	int listaIngrediente[3]={0,0,0};
	bool mostradorLibre;

	CondVar mostrador, ingrediente[3];

	public:                    	// constructor y métodos públicos
	Estanco() ;           			// constructor
  	void ponerIngrediente(int ingr, int i);
	void esperarRecogidaIngrediente(int i);
	void obtenerIngrediente(int ingr,int i);
} ;

/*METODOS MONITOR*/
Estanco::Estanco()
{
	mostrador = newCondVar();
	for (int i = 0; i < totalFum; i++)
	{
		ingrediente[i] = newCondVar();
	}
	mostradorLibre = true;
}

void Estanco::ponerIngrediente(int ingr, int i)
{
    cout<<"Estanco "<<i<<" : Reviso si esta libre el mostrador. 1"<<endl;
	if (!mostradorLibre)
	{
        cout<<"Estanco "<<i<<" : Esta ocupado. Espero. 1"<<endl;
		mostrador.wait();
	}

    mostradorLibre = false;

    cout<<"Estanco "<<i<<" : Coloco el ingrediente "<<ingr<<" en mostrador."<<endl;
    
	listaIngrediente[ingr]++;

    cout<<"Estanco "<<i<<" : Aviso al fumador implicado."<<endl;

	ingrediente[ingr].signal();
}

void Estanco::esperarRecogidaIngrediente(int i)
{
    cout<<"Estanco "<<i<<" : Reviso si esta libre el mostrador."<<endl;

	if(!mostradorLibre)
	{	
        cout<<"Estanco "<<i<<" : Espero."<<endl;
		mostrador.wait();
	}
}

void Estanco::obtenerIngrediente(int ingr,int i)
{
    cout<<"Fumador "<<i<<" : Reviso si esta mi ingrediente "<<ingr<<"."<<endl;
	if(listaIngrediente[ingr] == 0)
	{
        cout<<"Fumador "<<i<<" : No esta. Espero."<<endl;
		ingrediente[ingr].wait();
	}

    cout<<"Fumador "<<i<<" : Dejo el mostrador libre."<<endl;
	listaIngrediente[ingr]--;

	mostradorLibre = true;

	mostrador.signal();
}

/*PRODUCIR*/
int ProducirIngrediente(int i)
{

	chrono::milliseconds duracion_producir( aleatorio<0,0>() );
	int producto = aleatorio<0,2>();
    cout<<"Productor "<<i<<" : Voy a generar un ingrediente... "<<producto<<" ."<<endl;
	this_thread::sleep_for( duracion_producir );
    cout<<"Productor "<<i<<" : ...Finalizo de generar un ingrediente."<<endl;
	
	return producto;	
}
/*FUNCION ESTANCO*/
void funcion_hebra_estanco(MRef<Estanco> monitor, int nroEstanco)
{
	int dato;
	while(true)
	{
		dato = ProducirIngrediente(nroEstanco);
		monitor->ponerIngrediente(dato,nroEstanco);
    	monitor->esperarRecogidaIngrediente(nroEstanco);
	}
}

/*CONSUMIR*/
void fumar(int i)
{
	chrono::milliseconds duracion_fumar( aleatorio<0,0>() );
    cout << "Fumador "<<i<<" : "<< " empieza a fumar (" << duracion_fumar.count()<<" milisegundos)"<<endl;
	this_thread::sleep_for( duracion_fumar );
	cout << "Fumador "<<i<<" : termina de fumar, comienza espera de ingrediente."<<endl;
}
/*FUNCION FUMADOR*/
void funcion_hebra_fumador(MRef<Estanco> monitor, int nroFumador)
{
	while(true)
	{
		monitor->obtenerIngrediente(nroFumador%3,nroFumador);
		fumar(nroFumador);
	}	
}

int main()
{
	MRef<Estanco> monitor = Create<Estanco>();
	thread hebra_estanco[totalEst], hebra_fumador[totalFum];

	for(int i=0;i<totalEst;i++)
	{
		hebra_estanco[i] = thread(funcion_hebra_estanco, monitor, i);
	}

	for(int i=0;i<totalFum;i++)
	{
		hebra_fumador[i] = thread(funcion_hebra_fumador,monitor,i);
	}


	for(int i=0;i<totalEst;i++)
	{
		hebra_estanco[i].join();
	}

	for(int i=0;i<totalFum;i++)
	{
		hebra_fumador[i].join();
	}
}
