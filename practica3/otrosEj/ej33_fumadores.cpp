#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream> 


using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
int	// Cantidad de procesos.
	numFumadores = 3,
	numEstanco = 1,
	numProc = numFumadores + numEstanco,
	// IDs importantes.
	// Fumadores = [0, 1, 2 , ... numFumadores-1]
	idEstanco = numFumadores,
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
// ESTANQUERO - GENERAR INGREDIENTE
int generaIngrediente()
{
	int ingr1,
		ingr2,
		seleccionado;
	do
	{
		ingr1 = aleatorio<0,2>();
		ingr2 = aleatorio<0,2>();
	} while (ingr1 == ingr2);	
	
	for (int i = 0; i < numFumadores; i++)
	{
		if (i != ingr1 && i != ingr2)
		{
			seleccionado = i;
		}
	}

	sleep_for( milliseconds( aleatorio<200,500>() ) );
	cout<<"Estanco: Generado dos ingredientes ["<<ingr1<<", "<<ingr2<<"]. Aviso a fumador "<<seleccionado<<endl;

	return seleccionado;
}

// ESTANQUERO
void estanquero()
{
	int fumador,
		msjSinUso;

	MPI_Status estado;

	while (true)
	{
		cout<<"Estanco: Voy a generar ingredientes."<<endl;
		fumador = generaIngrediente();
		MPI_Send(&msjSinUso, 1, MPI_INT, fumador, etiqGen, MPI_COMM_WORLD);
		cout<<"Estanco: Mostrador en uso. Espero respuesta de fumador "<<fumador<<endl;
		MPI_Recv(&msjSinUso, 1, MPI_INT, fumador, etiqGen, MPI_COMM_WORLD, &estado);
		cout<<"Estanco: Mostrador libre. Respuesta recibida de fumador "<<fumador<<endl;
	}
}

// FUMADOR
void fumador(int id)
{
	int msjSinUso;

	MPI_Status estado;

	while (true)
	{
		cout<<"Fumador "<<id<<": Espero que me avise el estanco."<<endl;
		MPI_Recv(&msjSinUso, 1, MPI_INT, idEstanco, etiqGen, MPI_COMM_WORLD, &estado);
		cout<<"Fumador "<<id<<": Recibo los ingredientes. Le aviso al estanco que es mostrador está libre"<<endl;
		MPI_Send(&msjSinUso, 1, MPI_INT, idEstanco, etiqGen, MPI_COMM_WORLD);

		cout<<"Fumador "<<id<<": Lío el cigarro y fumo"<<endl;
		sleep_for( milliseconds( aleatorio<200,500>() ) );
		cout<<"Fumador "<<id<<": Terminé mi cigarro, quiero más."<<endl;
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

	if ( numProc == num_procesos_actual )
	{
		if (id_propio == idEstanco)
		{
			estanquero();
		}
		else
		{
			fumador(id_propio);
		}
		
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