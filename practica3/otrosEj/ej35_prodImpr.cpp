#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream> 


using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
int numProd = 3,
    numImpr = 1,
    numProc = numProd + numImpr,
    idImpr = numProd,
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
// IMPRESOR
void impresor()
{
    int cerosAcept = 0,
        unosAcept = 0,
        dosAcept = 0,
        ultimoCeroUno = -1,
        procAct,
        flag,
        impr;

    MPI_Status estado;

    while (true)
    {
        MPI_Iprobe(aleatorio<0,2>(), MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &estado);
        procAct = estado.MPI_SOURCE;
        if (flag)
        {
            switch (procAct)
            {
                case 0:
                    if(ultimoCeroUno != 0 && cerosAcept < 2*dosAcept)
                    {
                        MPI_Recv(&impr, 1, MPI_INT, procAct, etiqGen, MPI_COMM_WORLD, &estado);
                        cerosAcept++;
                        ultimoCeroUno = 0;
                        cout<<"Impresor: Recibido = "<<impr<<", Contador["<<cerosAcept<<", "<<unosAcept<<", "<<dosAcept<<"]. 0/1 ="<<ultimoCeroUno<<" 2*dosAcept="<<2*dosAcept<<endl;
                    }
                break;
                
                case 1:
                    if(ultimoCeroUno != 1 && unosAcept < 2*dosAcept)
                    {
                        MPI_Recv(&impr, 1, MPI_INT, procAct, etiqGen, MPI_COMM_WORLD, &estado);
                        unosAcept++;
                        ultimoCeroUno = 1;
                        cout<<"Impresor: Recibido = "<<impr<<", Contador["<<cerosAcept<<", "<<unosAcept<<", "<<dosAcept<<"]. 0/1 ="<<ultimoCeroUno<<" 2*dosAcept="<<2*dosAcept<<endl;
                    }
                break;

                case 2:
                    MPI_Recv(&impr, 1, MPI_INT, procAct, etiqGen, MPI_COMM_WORLD, &estado);
                    dosAcept++;
                    cout<<"Impresor: Recibido = "<<impr<<", Contador["<<cerosAcept<<", "<<unosAcept<<", "<<dosAcept<<"]. 0/1 ="<<ultimoCeroUno<<" 2*dosAcept="<<2*dosAcept<<endl;
                break;
            }
        }
        sleep_for( milliseconds( 1000 ));

    }
}

// PRODUCTOR  
void productor(int id)
{
    while (true)
    {
        MPI_Ssend(&id, 1, MPI_INT, idImpr, etiqGen, MPI_COMM_WORLD);
        sleep_for( milliseconds( aleatorio<200,2000>() ));
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
        if (id_propio == idImpr)
        {
            impresor();
        }
        else
        {
            productor(id_propio);
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