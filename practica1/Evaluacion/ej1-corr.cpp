#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

// Ejercicio de productor-consumidor implementado de manera que existen multiples consumidores y productores, distribución equitativa.

//**********************************************************************
// variables compartidas

const int num_items = 100 ,      // número de items
	       tam_vec   = 10 ;     // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos

int   numProd = 4;      //numero de productores
int   numCons = 1;      //numero de consumidores

int buffer[tam_vec];    // Vector de bufer que contiene los datos producidos

int primera_libre = 0;  // Definiendo variable que indica dónde está el inicio de la cola (x.push_front())
int primera_ocupada = 0; // Definiendo variable que indica dónde está el final de la cola (x.pop_back())

// Definiendo semáforos
Semaphore   libres = tam_vec; //Semaforo que indica cuantos espacios libres hay en el bufer
Semaphore   ocupadas = 0;     //Semaforo que indica cuantos espacios ocupados hay en el bufer
std::mutex cerrojo;



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

void  funcion_hebra_productora( int prod  )
{
   int iter = num_items/numProd;
   if (prod == 0 && num_items%numCons != 0 ) 
      iter += num_items%numProd;

   for( unsigned i = 0 ; i < iter ; i++ )
   {
        int dato = producir_dato() ;
      
        //Productor espera que haya espacio en el búfer para insertar producto.
        sem_wait(libres);

        //Insertar dato en búfer
        cerrojo.lock();
		cout<<"Prod "<< prod << " : dato "<< dato <<" en bufer"<<endl;
        buffer[primera_libre] = dato;

        //Se actualiza el "puntero", si sobrepasa el tamaño de la cola, se reposiciona al principio.
        primera_libre++;
        primera_libre = primera_libre % tam_vec;
         cerrojo.unlock();
        cout<<"1era posicion libre: "<< primera_libre <<endl;

        //Productor indica que hay un espacio ocupado en el búfer
        sem_signal(ocupadas);
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora( int cons )
{
   int iter = num_items/numCons;

   //Si el número de productos no es divisible entre el numero de consumidores, alguno debe sumar el resto para que todos puedan finalizar.
   if (cons == 0 && num_items%numCons != 0 ) 
      iter += num_items%numCons;

   for( unsigned i = 0 ; i < iter ; i++ )
   {
        int dato ;
        //Consumidor espera que el vector esté ocupado para obtener un producto.
        sem_wait(ocupadas);

        //Extraer dato del búfer

			cout<<"                  Cons "<< cons <<" dato fuera de bufer"<<endl;

        dato = buffer[primera_ocupada];

        //Se actualiza la posicion del "puntero", si sobrepasa el tamaño del vector, se reposiciona al principio.
        primera_ocupada++;
        primera_ocupada = primera_ocupada % tam_vec;


      cout<<"                  1ra posicion ocupada: "<< primera_ocupada <<endl;

      //Consumidor indica que ha liberado un espacio en el búfer y procede a consumir el dato.
      sem_signal(libres);
      consumir_dato( dato ) ;
    }
}
//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO, multiples productores y consumidores)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

    //Declaración del vector de hebras, las variables globales permiten que se puedan modificar los numeros de iteración
    thread hebra_productora[numProd],
          hebra_consumidora[numCons];

    //Se lanzan las hebras
    for (int i = 0; i < numProd; i++)
    {
        hebra_productora[i] = thread (funcion_hebra_productora, i);
    }

    for (int i = 0; i < numCons; i++)
    {
        hebra_consumidora[i] = thread (funcion_hebra_consumidora, i);
    }
    
    for (int i = 0; i < numProd; i++)
    {
        hebra_productora[i].join();
    }
    for (int i = 0; i< numCons; i++)
    {
        hebra_consumidora[i].join();
    }

   cout <<"fin"<<endl;
   test_contadores();
}
