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
Semaphore ingrGenerado = 0;

int igr;

int num_prod[3] = {0,0,0};
int num_cosm[3] = {0,0,0};


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
int producir()
{
	// Generar el retraso en producir aleatoriamente, así como que ingrediente se va a producir.
	chrono::milliseconds duracion_producir( aleatorio<20,200>() );
	int producto = aleatorio<0,2>();
	
	// Espera bloqueada del tiempo que tarda en producir.
	this_thread::sleep_for( duracion_producir );
	
	// Informa que ya ha producido un ingrediente.
	cout << "Proveedor producido ingrediente: " << producto << endl;
	num_prod[producto] += 1;
	return producto;	
}

void verificacion()
{
	cout << "Producido: " << num_prod[0] <<", " << num_prod[1] <<", " << num_prod[2] <<endl;
	cout << "Consumido: "<< num_cosm[0] <<", " << num_cosm[1] <<", " << num_cosm[2] <<endl;
	if(num_cosm[0] > num_prod[0] || num_cosm[1] > num_prod[1] || num_cosm[2] > num_prod[2])
		exit(0);
}



void funcion_hebra_proveedor( )
{
	int i;
	while (true)
	{
		sem_wait(mostradorVacio);
		igr = producir();
		cout << "Proveedor envia ingrediente a Estanquero"<<endl;
		sem_signal(ingrGenerado);
	}
}


// función que ejecuta la hebra del estanquero
void funcion_hebra_estanquero(  )
{
	int i;
	while ( true )
	{
		// Genera el producto.
		sem_wait(ingrGenerado);
		i = igr;
		cout << "Estanquero recibe ingrediente." <<endl;
		// Notificar que se ha puesto el ingrediente en mostrador.
		cout << "Estanquero: ingrediente " << i <<" en mostrador."<<endl;

		// Actualizar semáforo.
		sem_signal(listaIngredientes[i]);
		verificacion();
	}
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

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
	cout << "Fumador " << num_fumador << " : ha tomado el ingrediente que necesitaba" <<endl;

	// Dejar el mostrador vacío
	sem_signal(mostradorVacio);
	
	fumar(num_fumador);
	num_cosm[num_fumador] += 1;
	verificacion();
   }
}

//----------------------------------------------------------------------

int main()
{
	// Definiendo variables y lanzando las hebras
	int numFumadores = 3;
	thread hebraProveedor (funcion_hebra_proveedor);
	thread hebraEstanquero (funcion_hebra_estanquero);
	thread hebraFumador[numFumadores];
	
	for (int i = 0; i < numFumadores; i++)
	{
		hebraFumador[i] = thread(funcion_hebra_fumador,i);
	}

	// Esperar que terminen hebras (que no lo harán ya que tienen un while(TRUE), pero por buena práctica y por si las moscas)
	hebraEstanquero.join();
	for (int i = 0; i < numFumadores; i++)
	{
		hebraFumador[i].join();
	}	
}











