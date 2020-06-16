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


// Variables para chequeo
	mutex mtx;
	int totalProd[3]={0,0,0};
	int totalCons[3]={0,0,0};


/*FUNCION VERIFICADORA*/
void check()
{
	mtx.lock();
	cout << "PRODUCIDO: "<< totalProd[0] <<" "<< totalProd[1] <<" "<<totalProd[2]<<endl;
	cout << "CONSUMIDO: "<< totalCons[0] <<" "<< totalCons[1] <<" "<<totalCons[2]<<endl;
	mtx.unlock();
}

void say(string in)
{
	cout << in << endl;
}


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
  	void ponerIngrediente(int ingr);
	void esperarRecogidaIngrediente();
	void obtenerIngrediente(int ingr);
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

void Estanco::ponerIngrediente(int ingr)
{
	listaIngrediente[ingr]++;
	totalProd[ingr]++;
	assert( totalCons[ingr] < totalProd[ingr] );

//	check();
	mostradorLibre = false;
	ingrediente[ingr].signal();
//	cout << "ESTANCO: "<< listaIngrediente[0] << " "<< listaIngrediente[1] << " " << listaIngrediente[2] <<endl;
}

void Estanco::esperarRecogidaIngrediente()
{
	if(!mostradorLibre)
	{	
		say("Estanco: antes del wait");
		mostrador.wait();
	}
}

void Estanco::obtenerIngrediente(int ingr)
{
	if(listaIngrediente[ingr] == 0)
		ingrediente[ingr].wait();

	listaIngrediente[ingr]--;
	totalCons[ingr]++;
	check();
	mostradorLibre = true;
	//cout << "							FUMADOR "<< ingr <<" : "<< listaIngrediente[0] << " "<< listaIngrediente[1] << " " << listaIngrediente[2] <<endl;
	mostrador.signal();
}

/*PRODUCIR*/
int ProducirIngrediente()
{
	chrono::milliseconds duracion_producir( aleatorio<0,0>() );
	int producto = aleatorio<0,2>();
	
	this_thread::sleep_for( duracion_producir );
	
	cout << "Estanquero producido ingrediente: " << producto << endl;

	return producto;	
}
/*FUNCION ESTANCO*/
void funcion_hebra_estanco(MRef<Estanco> monitor, int nroEstanco)
{
	int dato;
	while(true)
	{
		dato = ProducirIngrediente();
		cout << "Estanco : " << nroEstanco <<" ha producido un ingrediente y lo coloca en el mostrador."<<endl;
		monitor->ponerIngrediente(dato);
		cout << "Estanco : " << nroEstanco <<" espera que sea recogido el ingrediente."<<endl;
		monitor->esperarRecogidaIngrediente();
	}
}

/*CONSUMIR*/
void fumar(int i)
{
	chrono::milliseconds duracion_fumar( aleatorio<0,0>() );
    cout << "Fumador " << i << "  : "<< " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
	this_thread::sleep_for( duracion_fumar );
	cout << "Fumador " << i << "  : termina de fumar, comienza espera de ingrediente." << endl;
}
/*FUNCION FUMADOR*/
void funcion_hebra_fumador(MRef<Estanco> monitor, int nroFumador)
{
	while(true)
	{
		cout <<"Fumador "<< nroFumador <<" : Espera a obtener el ingrediente"<<endl;
		monitor->obtenerIngrediente(nroFumador);
		cout <<"Fumador "<< nroFumador <<" : Obtiene el ingrediente.";
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
