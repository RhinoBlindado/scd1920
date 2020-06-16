#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>
#include <string>
#include <list>
#include <cassert>


using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
int numClientes = 4,
    numTendero = 1,
    num_procesos = numClientes + numTendero,
    idTendero = numClientes,
    capacidadLicor = 30,
    etiqGenerica = 0;


//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//      FUNCIONES
// CLIENTE
void cliente(int id)
{
    int botella;

    MPI_Status estadoSinUso;

    while (true)
    {
        cout << "Cliente "<<id<<": Envío petición para comprar una botella."<<endl;
        MPI_Ssend(&botella, 1, MPI_INT, idTendero, etiqGenerica, MPI_COMM_WORLD);
        cout << "Cliente "<<id<<": Petición aceptada, espero la botella."<<endl;
        MPI_Recv(&botella, 1, MPI_INT, idTendero, etiqGenerica, MPI_COMM_WORLD, &estadoSinUso);
        cout << "Cliente "<<id<<": He recibido la botella "<<botella<<" y me la tomo."<<endl;

        sleep_for( milliseconds( aleatorio<200,500>() ) );
        cout << "Cliente "<<id<<": Se terminó la botella. Voy por más."<<endl;
    }
}

//  TENDERO
void tendero()
{
    int botellasEnAlmacen = capacidadLicor,
        clienteAnt = -1,
        msjSinUso,
        flag;

    MPI_Status estado;

    while (true)
    {
        if (botellasEnAlmacen > 0)
        {
            if (botellasEnAlmacen > 5)
            {
                for (int i = 0; i < numClientes; i++)
                {
                    MPI_Iprobe(i, etiqGenerica, MPI_COMM_WORLD, &flag, &estado);

                    if (i != clienteAnt && flag && botellasEnAlmacen)
                    {
                        cout <<"Tendero: Le doy una botella al cliente "<<i<<"."<<endl;
                        MPI_Recv(&msjSinUso, 1, MPI_INT, i, etiqGenerica, MPI_COMM_WORLD, &estado);
                        MPI_Ssend(&botellasEnAlmacen, 1, MPI_INT, i, etiqGenerica, MPI_COMM_WORLD);
                        clienteAnt = i;
                        botellasEnAlmacen--;
                    }
                }
            }
            else
            {
                MPI_Recv(&msjSinUso, 1, MPI_INT, MPI_ANY_SOURCE, etiqGenerica, MPI_COMM_WORLD, &estado);
                MPI_Ssend(&botellasEnAlmacen, 1, MPI_INT, estado.MPI_SOURCE, etiqGenerica, MPI_COMM_WORLD);
                botellasEnAlmacen--;
            }
        }
        else
        {
            cout <<"Tendero: No hay más botellas, relleno mi almacén."<<endl;
            botellasEnAlmacen = capacidadLicor;
            sleep_for( milliseconds( aleatorio<200,400>() ) );
            cout <<"Tendero: Almacén rellenado."<<endl;
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

	if ( num_procesos == num_procesos_actual )
	{
      if (id_propio == idTendero)
         tendero();
      else
         cliente(id_propio);
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