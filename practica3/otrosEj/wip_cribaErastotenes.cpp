#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>
#include <string>
#include <cmath>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

//      VARIABLES GLOBALES
const int 
 nLimite = 30,
 numProcesos = nLimite + 1;

//      FUNCIONES
void inicio()
{

}

void final()
{

}

void filtro(int id)
{

}

void impresor()
{
    bool finalizado=false;
    int valor;
    MPI_status estado;
    while (!finalizado)
    {
        MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado);
        if (valor == -999)
        {
            finalizado=true;
            cout<<"Impresor: Fin de valores."<<endl;
        }
        else
            cout <<"Impresor: Recibido valor "<<valor<<" del proceso "<<estado.MPI_SOURCE<<"."<<endl;
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
        if(id_propio == 0)
        {
            inicio();
        }
        else if (id_propio = nLimite-1)
        {
            final();
        }
        else if (id_propio = nLimite)
        {
            impresor();
        }
        else
        {
            filtro(d_propio);
        }   
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