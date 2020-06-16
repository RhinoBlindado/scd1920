#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
	// Número de misioneros.
int M = 10,
	// Cantidad de procesos:
	numSalvajes = 4,
	numCocinero = 1,
	numOlla = 1,
	numProcesos = numSalvajes + numCocinero + numOlla,
	// ID de los procesos:
	// Salvajes: [0, 1 , 2 ... numSalvajes-1] 
	// Cocinero: numSalvajes
	// Olla: numSalvajes+1
	idCocinero = numSalvajes,
	idOlla = numSalvajes+1,
	// Etiquetas
	etiqSalvaje = 0,
	etiqCocinero = 1;

//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//      FUNCIONES
// COCINERO
void cocinero()
{
	int msjSinUso;

	MPI_Status estado;

	while (true)
	{
		cout<<"Cocinero: 	Espero solicitud para llenar."<<endl;
		MPI_Ssend(&msjSinUso, 1, MPI_INT, idOlla, etiqCocinero, MPI_COMM_WORLD);
		MPI_Recv(&msjSinUso, 1, MPI_INT, idOlla, etiqCocinero, MPI_COMM_WORLD, &estado);
		cout<<"Cocinero:	Solicitud recibida. Empiezo a llenar olla."<<endl;
        sleep_for( milliseconds( aleatorio<200,500>() ) );
		cout<<"Cocinero:	Olla rellenada. Envío confirmación."<<endl;
		MPI_Ssend(&msjSinUso, 1, MPI_INT, idOlla, etiqCocinero, MPI_COMM_WORLD);
	}
}

// SALVAJE
void salvaje(int id)
{
	int msj;

	MPI_Status estado;

	while (true)
	{
		cout<<"Salvaje "<<id<<":	Espero a servirme un misionero."<<endl;
		MPI_Ssend(&msj, 1, MPI_INT, idOlla, etiqSalvaje, MPI_COMM_WORLD);
		MPI_Recv(&msj, 1, MPI_INT, idOlla, etiqSalvaje, MPI_COMM_WORLD, &estado);

		cout<<"Salvaje "<<id<<":	Misionero servido. Me lo como."<<endl;
		sleep_for( milliseconds( aleatorio<200,500>() ) );
		cout<<"Salvaje "<<id<<":	Terminé de comerlo. Voy por más."<<endl;
	}
}

// OLLA
void olla()
{
	int misioneros = M,
		msj,
		etiqAcept;

	MPI_Status estado;

	while (true)
	{
		if (misioneros > 0)
		{
			etiqAcept = etiqSalvaje;
		}
		else
		{
			etiqAcept = etiqCocinero;
		}

		MPI_Recv(&msj, 1, MPI_INT, MPI_ANY_SOURCE, etiqAcept, MPI_COMM_WORLD, &estado);

		if (estado.MPI_TAG == etiqSalvaje)
		{
			misioneros--;
			cout<<"Olla: 		Salvaje "<<estado.MPI_SOURCE<<" toma un misionero, restan "<<misioneros<<endl;
			MPI_Ssend(&msj, 1, MPI_INT, estado.MPI_SOURCE, etiqSalvaje, MPI_COMM_WORLD);
		}
		else
		{
			cout<<"Olla: 		Vacía, se despierta cocinero."<<endl;
			MPI_Ssend(&msj, 1, MPI_INT, idCocinero, etiqCocinero, MPI_COMM_WORLD);
			MPI_Recv(&msj, 1, MPI_INT, idCocinero, etiqCocinero, MPI_COMM_WORLD, &estado);
			misioneros = M;
			cout<<"Olla: 		Rellenada. Se sirven salvajes."<<endl;
		}
	}
}

// MAIN
int main( int argc, char** argv )
{
	int id_propio, num_procesos_actual;

	// Inicializacion MPI
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
	MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

	if ( numProcesos == num_procesos_actual )
	{
		if (id_propio == idCocinero)
		{
			cocinero();
		}
		else if (id_propio == idOlla)
		{
			olla();
		}
		else
		{
			salvaje(id_propio);
		}
	}
	else
	{
		if ( id_propio == 0 ) 
		{ cout << "el número de procesos esperados es:    " << numProcesos << endl
		       << "el número de procesos en ejecución es: " << num_procesos_actual << endl
		       << "(programa abortado)" << endl ;
		}
	}

	MPI_Finalize( );
	return 0;
}