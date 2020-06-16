#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream> 


using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
int	numCli = 20,
    numCon = 1,
    numProc = numCli + numCon,
    idCon = numCli,
    etiqUsar = 0,
    etiqLibr = 1;

//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
//      FUNCIONES
// CONTROLADOR
void controlador()
{
    int cont=0,
        etiqAcept,
        msjSinUso,
        flag,
        cli1,cli2,
        tag1,tag2;

    MPI_Status estado;

    while (true)
    {
        if (cont < 12 || cont > 14)
        {
            cout<<"Controlador: Recibo mensajes normal. Total: "<<cont<<"."<<endl;
            MPI_Recv(&msjSinUso, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);

            if (estado.MPI_TAG == etiqUsar)
            {
                cont++;
                cout<<"Controlador. Solicitud de uso aceptada. Cliente "<<estado.MPI_SOURCE<<"."<<endl;
                MPI_Send(&msjSinUso, 1, MPI_INT, estado.MPI_SOURCE, etiqUsar, MPI_COMM_WORLD);
            }
            else
            {
                cont--;
                cout<<"Controlador. Solicitud de liberación aceptada. Cliente "<<estado.MPI_SOURCE<<"."<<endl;
                MPI_Send(&msjSinUso, 1, MPI_INT, estado.MPI_SOURCE, etiqLibr, MPI_COMM_WORLD);
            }
        }
        else
        {
            cout<<"Controlador: Peligro. Cambio de modo. Total: "<<cont<<"."<<endl;
            int encontrados = 0;
            while (encontrados < 2)
            {
                for(int i = 0; i < numCli; i++)
                {
                    MPI_Iprobe(i, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &estado);

                    if (flag)
                    {
                        if (encontrados == 0)
                        {
                            cli1 = estado.MPI_SOURCE;
                            tag1 = estado.MPI_TAG;
                            cout<<"Controlador: Encontrado primer cliente: "<<cli1<<", etiqueta:"<<tag1<<"."<<endl;
                        }

                        if (encontrados == 1)
                        {
                            cli2 = estado.MPI_SOURCE;
                            tag2 = estado.MPI_TAG;
                            cout<<"Controlador: Encontrado segundo cliente: "<<cli2<<", etiqueta:"<<tag2<<"."<<endl;
                            encontrados++;
                            break;
                        }
                        encontrados++;
                    }
                }
            }
            cout<<"Controlador: Acepto ambas solicitudes y respondo."<<endl;
            MPI_Recv(&msjSinUso, 1, MPI_INT, cli1, tag1, MPI_COMM_WORLD, &estado);
            MPI_Recv(&msjSinUso, 1, MPI_INT, cli2, tag2, MPI_COMM_WORLD, &estado);

            MPI_Send(&msjSinUso, 1, MPI_INT, cli1, tag1, MPI_COMM_WORLD);
            MPI_Send(&msjSinUso, 1, MPI_INT, cli2, tag2, MPI_COMM_WORLD);


            if (tag1 == tag2)
            {
                if(tag1 == etiqLibr)
                    cont -= 2;
                else
                    cont += 2;
            }
            cout<<"Controlador: Salgo de modo peligro."<<endl;
        }
    }
}

// CLIENTE  
void cliente(int id)
{
    int permiso;

    MPI_Status estado;

    while (true)
    {
        cout<<"Cliente "<<id<<": Envío solicitud de uso y espero respuesta."<<endl;
        MPI_Send(&permiso, 1, MPI_INT, idCon, etiqUsar, MPI_COMM_WORLD);
        MPI_Recv(&permiso, 1, MPI_INT, idCon, etiqUsar, MPI_COMM_WORLD, &estado);

        cout<<"Cliente "<<id<<": Solicitud aceptada, uso el recurso."<<endl;
        sleep_for( milliseconds( aleatorio<1, 10000>() ));
        cout<<"Cliente "<<id<<": Finalizo de usar recurso. Envío solicitud de liberación."<<endl;

        MPI_Send(&permiso, 1, MPI_INT, idCon, etiqLibr, MPI_COMM_WORLD);
        MPI_Recv(&permiso, 1, MPI_INT, idCon, etiqLibr, MPI_COMM_WORLD, &estado);
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
		if (id_propio == idCon)
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
		{ cout << "el número de procesos esperados es:    " << numProc << endl
		       << "el número de procesos en ejecución es: " << num_procesos_actual << endl
		       << "(programa abortado)" << endl ;
		}
	}

	MPI_Finalize( );
	return 0;
} 
