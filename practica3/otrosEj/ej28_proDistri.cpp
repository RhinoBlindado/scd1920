#include <iostream>
#include <thread>
#include <random> 
#include <chrono> 
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

//      VARIABLES GLOBALES
int numClientes = 6, 
    numProcesos = numClientes + 1,
    idControlador = numClientes,
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
void cliente(int id)
{
    int msjSinUso;
    MPI_Status estadSinUso;
    while (true)
    {
        cout <<"Cliente: "<<id<<" Envio solicitud al controlador."<<endl;
        MPI_Send(&msjSinUso, 1, MPI_INT, idControlador, etiqGenerica, MPI_COMM_WORLD);
        cout <<"Cliente: "<<id<<" Espero respuesta del controlador."<<endl;
        MPI_Recv(&msjSinUso, 1, MPI_INT, idControlador, etiqGenerica, MPI_COMM_WORLD, &estadSinUso);

        cout <<"Cliente: "<<id<<" Respuesta recibida, me pongo a trabajar."<<endl;

        // Realizar un trabajo 
        sleep_for(milliseconds(aleatorio<100,600>()));

        cout <<"Cliente: "<<id<<" Fin del trabajo."<<endl;
    }
}

void controlador()
{
    int contador = 0,
        preparados[numClientes] = {0},
        msjSinUso;
    MPI_Status estado;

    while(true)
    {

        if (contador < 3)
        {
            cout <<"Controlador: No hay 3 procesos, espero recepcion de mensaje."<<endl;
            MPI_Recv(&msjSinUso, 1, MPI_INT, MPI_ANY_SOURCE, etiqGenerica, MPI_COMM_WORLD, &estado);
            contador++;
            preparados[estado.MPI_SOURCE] = 1;

            cout <<"Controlador: Contador = "<<contador<<". Cliente "<<estado.MPI_SOURCE<<" preparado."<<endl;
        }
        else
        {
            cout <<"Controlador: 3 procesos listos; los reactivo."<<endl;
            for (int i = 0; i < numClientes; i++)
            {
                if (preparados[i])
                {
                    cout <<"Controlador: Enviando mensaje de reactivacion a "<<i<<"."<<endl;
                    MPI_Ssend(&msjSinUso, 1, MPI_INT, i, etiqGenerica, MPI_COMM_WORLD);
                    preparados[i] = 0;
                }
            }
            contador = 0;
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
        if (id_propio == idControlador)
        {
            controlador();
        }
        else
        {
            cliente(id_propio);
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
}