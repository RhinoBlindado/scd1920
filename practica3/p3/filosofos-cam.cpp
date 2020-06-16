#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;


// VARIABLES GLOBALES
const int
   num_filosofos = 5,  
   idCamarero = 10,
   etiqSentar = 0,
   etiqLevant = 1,
   num_procesos  = (2*num_filosofos)+1 ;


//    FUNCIONES AUXILIARES
// FUNCION RNG

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//    FUNCIONES DE PROCESOS
// FILOSOFO
void funcion_filosofos( int id )
{
			// Se reduce en -1 todo para evitar tomar en cuenta el proceso del camarero.
  int id_ten_izq = (id+1)              % (num_procesos-1),
			//  													V-- Originalmente -1, ahora -2.
      id_ten_der = (id+num_procesos-2) % (num_procesos-1),
      peticion;
         cout <<"id="<<id<<"numproc="<<num_procesos<<endl;

  while ( true )
  {
      cout <<"Filósofo "<<id<< " solicita al camarero sentarse"<<endl;
      MPI_Ssend(&peticion, 1, MPI_INT, idCamarero, etiqSentar, MPI_COMM_WORLD);

      cout <<"Filósofo " <<id << " solicita ten. izq. " <<id_ten_izq <<endl; 
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);
    
      cout <<"Filósofo " <<id <<" solicita ten. der. " <<id_ten_der <<endl;
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

      cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
      sleep_for( milliseconds( aleatorio<10,100>() ) );

      cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;    
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

      cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
      MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

      cout << "Filosofo " << id << " se levanta de la mesa"<<endl;
      MPI_Ssend(&peticion, 1, MPI_INT, idCamarero, etiqLevant, MPI_COMM_WORLD);

      cout << "Filosofo " << id << " comienza a pensar" << endl;
      sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}

// TENEDOR
void funcion_tenedores( int id )
{
  int valor, id_filosofo ;
  MPI_Status estado ;

  while ( true )
  {
			// Esperar primera solicitud; esto es que he sido tomado por un filosofo.
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado );

			// Obtener el ID del filosofo que me ha tomado.
      id_filosofo = estado.MPI_SOURCE;
      cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

			// Esperar segunda solicitud; esto es que he sido liberado por el filosofo que me tomó.
      MPI_Recv( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado );
      cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}

// CAMARERO
void funcion_camarero()
{
	int
    numFiloMesa = 0,
    etiqAceptable,
    peticion;

   MPI_Status filosofo;

   while(true)
   {
			// Si la mesa no esta llena, aceptar cualquier etiqueta; sino, solo aceptar solicitudes de levantarse.
      if (numFiloMesa < 4)
         etiqAceptable = MPI_ANY_TAG;
      else
         etiqAceptable = etiqLevant;
   
      MPI_Recv(&peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiqAceptable, MPI_COMM_WORLD, &filosofo);

			// Determinar que hacer dependiendo de la solicitud recibida.
      switch (filosofo.MPI_TAG)
      {
         case etiqSentar:
            cout << "Camarero sienta al filosofo " << filosofo.MPI_SOURCE << " en la mesa" <<endl;
            numFiloMesa++;
         break;
      
         case etiqLevant:
            cout << "Camarero levanta al filosofo " << filosofo.MPI_SOURCE << " de la mesa" <<endl;
            numFiloMesa--;
         break;
      }

   }
}

// MAIN
int main( int argc, char** argv )
{
	int id_propio, num_procesos_actual ;

	// Inicializacion MPI
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
	MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


	if ( num_procesos == num_procesos_actual )
	{
		if(id_propio == idCamarero)
			funcion_camarero();
		else
		{  
		   if ( id_propio % 2 == 0 )    
		      funcion_filosofos( id_propio );
		   else
		      funcion_tenedores( id_propio );
		}
	}
	else
	{
		if ( id_propio == 0 ) 
		{ cout << "el número de procesos esperados es:    " << num_procesos << endl
		       << "el número de procesos en ejecución es: " << num_procesos_actual << endl
		       << "(programa abortado)" << endl ;
		}
	}

	MPI_Finalize( );
	return 0;
}
