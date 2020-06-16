#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream> 


using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
int N = 4,
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
// PROCESO
void proceso(int id)
{
    int miValor = aleatorio<1,999>(),
        suma = 0,
        temp,
        sigProc = (id + 1) %N,
        antProc = (id + N-1) %N;

    MPI_Status estado;

//    cout<<"Proceso "<<id<<": Mi valor es "<<miValor<<", yo envío al "<<sigProc<<" y recibo de "<<antProc<<endl;
    
    for (int i = 0; i < N; i++)
    {
        if (id % 2 == 0)
        {
            cout<<"Proceso "<<id<<":    Envio valor "<<miValor<<", "<<i<<endl;
            MPI_Ssend(&miValor, 1, MPI_INT, sigProc, etiqGen, MPI_COMM_WORLD);
            MPI_Recv(&miValor, 1, MPI_INT, antProc, etiqGen, MPI_COMM_WORLD, &estado);
         //   cout<<"Proceso "<<id<<":    Recib valor "<<miValor<<", "<<i<<endl;
            suma += miValor;
        }
        else
        {
            temp = miValor;
            MPI_Recv(&miValor, 1, MPI_INT, antProc, etiqGen, MPI_COMM_WORLD, &estado);
    //        cout<<"Proceso "<<id<<":    Recib valor "<<miValor<<", "<<i<<endl;
            suma += miValor;

            cout<<"Proceso "<<id<<":    Envio valor "<<temp<<", "<<i<<endl;
            MPI_Ssend(&temp, 1, MPI_INT, sigProc, etiqGen, MPI_COMM_WORLD);
        }
    }   

    cout<<"Proceso "<<id<<": Suma final es "<<suma<<endl;
}

// MAIN
int main( int argc, char** argv )
{
	int id_propio, num_procesos_actual;

	// Inicializacion MPI
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
	MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

	if ( N == num_procesos_actual )
	{
        proceso(id_propio);
	}
	else
	{
		if ( id_propio == 0 ) 
		{ cout << "el número de procesos esperados es:    " << N << endl
		       << "el número de procesos en ejecución es: " << num_procesos_actual << endl
		       << "(programa abortado)" << endl ;
		}
	}

	MPI_Finalize( );
	return 0;
}