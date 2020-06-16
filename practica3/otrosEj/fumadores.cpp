#include <iostream>
#include <thread>
#include <random> 
#include <chrono> 
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

//      VARIABLES GLOBALES
const int
 numIngr = 3,
 numProcesos = 1+1+3,
 etiqEstan = 3;

//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
//      FUNCIONES
// FUMADOR
void funcionFumador(int id)
{
    const int etiqFumad = id;
    int peticion;
    MPI_Status estatus;
    while (true)
    {
        cout << "Fumador " << id <<": Envío solicitud del ingrediente al mostrador."<<endl;
        MPI_Ssend(&peticion, 1, MPI_INT, numIngr+1, etiqFumad, MPI_COMM_WORLD);

        cout << "Fumador "<< id <<": Espero recibir el ingrediente del mostrador."<<endl;
        MPI_Recv(&peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiqFumad, MPI_COMM_WORLD, &estatus);

        cout << "Fumador "<<id<<": He recibido el ingrediente "<< peticion <<" del mostrador y procedo a fumar."<<endl;
        sleep_for( milliseconds( aleatorio<10,100>() ) );
        cout << "Fumador "<<id<<": Finalizado de fumar."<<endl;
    }
    
}

// ESTANQUERO - ESPERA
int producir()
{
    int producto = aleatorio<0,2>();
    sleep_for( milliseconds( aleatorio<10,100>() ) );
    return producto;
}

// ESTANQUERO - PRINCIPAL
void funcionEstanquero()
{
    int ingr;
    while (true)
    {
        cout << "Estanquero: Voy a producir un ingrediente."<<endl;
        ingr = producir();
        cout << "Estanquero: He producido el ingrediente "<<ingr<<", lo coloco en el mostrador."<<endl;
        MPI_Ssend(&ingr, 1, MPI_INT, numIngr+1, etiqEstan, MPI_COMM_WORLD);
        cout << "He terminado de enviar el ingrediente."<<endl;
    }
}

// MOSTRADOR
void funcionMostrador()
{
    int etiquetaAceptable,peticion,ingr;
    bool enUso=false;
    MPI_Status estatus;

    while (true)
    {
        if (enUso)
        {
            etiquetaAceptable = ingr;
        }
        else
        {
            etiquetaAceptable = etiqEstan;
        }

        MPI_Recv(&peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiquetaAceptable, MPI_COMM_WORLD, &estatus);

        if (estatus.MPI_TAG  == etiqEstan)
        {
            cout <<"Mostador: Recibido ingrediente "<<peticion<<" de estanquero."<<endl;
            ingr = peticion;
            enUso = true;
        }
        else
        {
            cout <<"Mostrador: Envio ingrediente "<<ingr<<" al fumador correspondiente "<<estatus.MPI_SOURCE<<"."<<endl;
            MPI_Ssend(&ingr, 1, MPI_INT, estatus.MPI_SOURCE, estatus.MPI_TAG, MPI_COMM_WORLD);
            enUso = false;
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
        if (id_propio < numIngr)
            funcionFumador(id_propio);
        else if (id_propio == numIngr)
            funcionEstanquero();
        else
            funcionMostrador();
                
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