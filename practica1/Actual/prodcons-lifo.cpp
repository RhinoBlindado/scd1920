#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 100 ,   // número de items
	       tam_vec   = 40 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos

int buffer[tam_vec];    // Vector de bufer que contiene los datos producidos
int primera_libre = 0;  // Definiendo variable que indica dónde está el tope de la pila del búfer

// Definiendo semáforos
Semaphore   libres = tam_vec; //Semaforo que indica cuantos espacios libres hay en el bufer
Semaphore   ocupadas = 0;     //Semaforo que indica cuantos espacios ocupados hay en el bufer

// Definiendo cerrojo del acceso a bufer.
std::mutex cerrojo;						//Evita que se pueda leer y escribir al mismo tiempo en el búfer


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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,200>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,200>() ));

   cout << "                  consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;
      
      //Productor espera que haya espacio en el búfer para insertar producto.
      sem_wait(libres);

      //Insertar dato en búfer
			cerrojo.lock();
			
			cout<<"Prod: dato "<< dato <<" en bufer"<<endl;

      buffer[primera_libre] = dato;
      primera_libre++;

		  cerrojo.unlock();

      cout<<"libres: "<< tam_vec - primera_libre<<endl;

      //Productor indica que hay un espacio ocupado en el búfer
      sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato ;
      //Consumidor espera que el vector esté ocupado para obtener un producto.
      sem_wait(ocupadas);

      //Extraer dato del búfer
 	  	cerrojo.lock();

			cout<<"                  Cons: dato fuera de bufer"<<endl;

      dato = buffer[primera_libre-1];
      primera_libre--;

      cerrojo.unlock();

      cout<<"                  libres: "<< tam_vec - primera_libre<<endl;

      //Consumidor indica que ha liberado un espacio en el búfer
      sem_signal(libres);
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   cout <<"fin"<<endl;
   test_contadores();
}
