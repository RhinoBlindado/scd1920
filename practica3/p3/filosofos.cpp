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
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos ;


// 		FUNCIONES AUXILIARES
// FUNCION RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//		FUNCIONES DE PROCESOS
// FILOSOFO
void funcion_filosofos( int id )
{
   int id_ten_izq = (id+1)              % num_procesos,
       id_ten_der = (id+num_procesos-1) % num_procesos,
       peticion;

	while ( true )
	{
		// Si el filosofo posee un numero de orden impar, invertir el orden que toma los tenedores.
		if(id/2 % 2)
		{
			 cout <<"Filósofo " <<id << " solicita ten. izq. " <<id_ten_izq <<endl;
			 MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

			 cout <<"Filósofo " <<id <<" solicita ten. der. " <<id_ten_der <<endl;
			 MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);
		}
		else
		{
			 cout <<"Filósofo " <<id <<" solicita ten. der. " <<id_ten_der <<endl;
			 MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

			 cout <<"Filósofo " <<id << " solicita ten. izq. " <<id_ten_izq <<endl;
			 MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);
		}

		cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
		sleep_for( milliseconds( aleatorio<10,100>() ) );

		cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);

		cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
		MPI_Ssend(&peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

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
		MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado );

		id_filosofo = estado.MPI_SOURCE;
		cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

		MPI_Recv( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado );
		cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
	}
}


// MAIN
int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      if ( id_propio % 2 == 0 )         
         funcion_filosofos( id_propio );
      else                               
         funcion_tenedores( id_propio );
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
