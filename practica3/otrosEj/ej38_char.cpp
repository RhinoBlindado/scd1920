#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>
#include <list> 

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
int numProc = 4,
	etiqGen = 0;

//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//      FUNCIONES

// PRODUCIR CARACTER
char generarCaracter()
{
	return aleatorio<65,90>(); 
}

// PROCESO
void proceso( int id )
{
	char letra,letraMsj;

	list<char> letraRecib,listaEnv;

	MPI_Status estado;

	for (int i = 0; i < numProc; i++)
	{
		if(i != id)
		{
			letra = generarCaracter();
			listaEnv.push_back(letra);
			MPI_Send(&letra, 1, MPI_CHAR, i, etiqGen, MPI_COMM_WORLD);
		}
	}

	for (int i = 0; i < numProc; i++)
	{
		if (i != id)
		{
			MPI_Recv(&letraMsj, 1, MPI_CHAR, i, etiqGen, MPI_COMM_WORLD, &estado);
			letraRecib.push_back(letraMsj);
		}
	}
	

	int const t = id;
	sleep_for(milliseconds(t*100));

	cout <<"Proceso "<<id<<": \nEnviadas [ ";
	while (!listaEnv.empty())
	{
		cout<<listaEnv.front()<<" ";
		listaEnv.pop_front();
	}
	cout<<"]"<<endl;

	cout <<"Recibidas [ ";
	while (!letraRecib.empty())
	{
		cout<<letraRecib.front()<<" ";
		letraRecib.pop_front();
	}
	cout<<"]"<<endl;

}

// MAIN
int main( int argc, char** argv )
{
	int id_propio, num_procesos_actual;

	// Inicializacion MPI
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
	MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

	if ( numProc == num_procesos_actual )
	{
		proceso(id_propio);
	}
	else
	{
		if ( id_propio == 0 ) 
		{ cout << "el número de procesos esperados es:    " << numProc << endl
		       << "el número de procesos en ejecución es: " << num_procesos_actual << endl
		       << "(programa abortado)" << endl ;
		}
	}

	MPI_Finalize( );
	return 0;
} 