#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;


//	Variables Compartidas
// Definiendo semáforos:
// mostradorVacio = { 1 si el mostrador está vacio, 0 si tiene un ingrediente }
// listaIngredientes = Por cada espacio en el vector indica si ese ingrediente i e
Semaphore mostradorVacio = 1;
Semaphore listaIngredientes[3] = {0,0,0};
Semaphore turno[2] = {0,0};

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------

//	Función que simula la acción de producir, como un retardo aletorio de la hebra.
int producir( int i )
{
	// Generar el retraso en producir aleatoriamente, así como que ingrediente se va a producir.
	chrono::milliseconds duracion_producir( aleatorio<20,200>() );
	int producto = aleatorio<0,2>();
	
	// Espera bloqueada del tiempo que tarda en producir.
	this_thread::sleep_for( duracion_producir );
	
	// Informa que ya ha producido un ingrediente.
	cout << "Estanquero "<< i <<" producido ingrediente: " << producto << endl;
	return producto;	
}

// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero( int num_estanquero  )
{
	// i_prev se encarga de tener en cuenta si se han repetido ingredientes seguidamente
	int i_prev = -1;
	int i;
	while ( true )
	{
		// El semaforo da el orden al que deben alternarse los estanqueros.
		sem_wait(turno[num_estanquero]);

		// Genera el producto.
		i = producir( num_estanquero );

		if(i_prev == i)
		{
			cout <<"El estanquero " << num_estanquero <<" ha producido dos veces seguidas el mismo ingrediente: "<< i <<endl;
		}
		// Esperar que el mostrador esté vacío para colocar ingrediente.
		sem_wait(mostradorVacio);

		// Notificar que se ha puesto el ingrediente en mostrador.
		cout << "Estanquero "<< num_estanquero <<" ha puesto el ingrediente " << i <<" en mostrador."<<endl;

		// Actualizar semáforo.
		sem_signal(listaIngredientes[i]);

		sem_signal(turno[(num_estanquero+1)%2]);
		i_prev = i;
	}
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

 //   cout << "Fumador " << num_fumador << "  :"
 //         << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

 //   cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
	// Esperar que el ingrediente faltante esté disponible.
	sem_wait(listaIngredientes[num_fumador]);
	
	// Notificar que se ha tomado el ingrediente.
//	cout << "Fumador " << num_fumador << " : ha tomado el ingrediente que necesitaba" <<endl;

	// Dejar el mostrador vacío
	sem_signal(mostradorVacio);
	
	fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
	// Definiendo variables y lanzando las hebras
	int numFumadores = 3, numEstanqueros = 2;

	thread hebraFumador[numFumadores];
	thread hebraEstanquero[numEstanqueros];

	// Se asigna aleatoriamente quien de los estanqueros va a comenzar primero.
	if(aleatorio<0,1>())
	{
		sem_signal(turno[0]);
	}
	else
		sem_signal(turno[1]);

	for (int i = 0; i < numEstanqueros; i++)
	{
		hebraEstanquero[i] = thread(funcion_hebra_estanquero,i);
	}

	for (int i = 0; i < numFumadores; i++)
	{
		hebraFumador[i] = thread(funcion_hebra_fumador,i);
	}

	// Esperar que terminen hebras (que no lo harán ya que tienen un while(TRUE), pero por buena práctica y por si las moscas)
	for (int i = 0; i < 2; i++)
	{
		hebraEstanquero[i].join();
	}

	for (int i = 0; i < numFumadores; i++)
	{
		hebraFumador[i].join();
	}	

	
}











