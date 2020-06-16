#include <iostream>
#include <thread>
#include <random> 
#include <chrono> 
#include <mpi.h>
#include <cmath>
#include <vector>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

//      VARIABLES GLOBALES
int
 m = 4,
 etiqGenerica = 0,
 idColector = 0,
 numProcesos = m + 1;

const int
 rangoIter = 4,
 rangoNum = 666,
 T = 100;

//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//      FUNCIONES
void colector()
{
    int msj, flag = 0, N = 99999;
    vector<int> listaNum;
    MPI_Status estado,estSinUso;
    
    for(int i = 0; i<m; i++)
    {
        MPI_Recv(&msj, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estSinUso);

        if (msj < N)
        {
            N = msj;
        }
    }

    cout << "Colector: Numero de min iteraciones: "<<N<<endl;
    N = (int)pow(N,m);

    cout << "Colector: Numero de iteraciones: "<<N<<endl;

    for (int i = 0; i < N; i++)
    {
        sleep_for(milliseconds(T));
        for (int j = 1; j <= m; j++)
        {
            MPI_Iprobe(j, etiqGenerica, MPI_COMM_WORLD, &flag, &estado);
            if (flag > 0)
            {
                MPI_Recv(&msj, 1, MPI_INT, estado.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estSinUso);
                listaNum.push_back(msj);
           }
        }

        int count = listaNum.size();
        if (count)
        {
            cout <<"Colector: Numeros recibidos ("<<count<<") son: ";
            for (int i = 0; i < count ; i++)
            {
                cout<<listaNum[i]<<" ";
            }
            cout << endl;
        }
        listaNum.clear();
    }
    bool vacio = false;

    cout<<"Colector: Bucle finalizado. Espera de los procesos que aun están en cola."<<endl;

    while(!vacio)
    {
        MPI_Iprobe(MPI_ANY_SOURCE, etiqGenerica, MPI_COMM_WORLD, &flag, &estado);
        if (flag > 0)
        {
            MPI_Recv(&msj, 1, MPI_INT, estado.MPI_SOURCE, etiqGenerica, MPI_COMM_WORLD, &estSinUso);
            cout<<"Colector: Mensaje " << msj <<" recibido de "<<estado.MPI_SOURCE<<endl;
        }
        else
        {
            vacio = true;   
        }
    }
    cout<<"Colector: Finalzado "<<endl;
}

void generadores(int idGen)
{
    int 
     count,
     numero;

    //      Generación de iteraciones y envío a colector().
    // Generar número de iteraciones a realizar.
    count = aleatorio<1,rangoIter>();

    cout <<"Generador "<<idGen<<": Mi numero de iteraciones es "<<count<<endl;
    // Enviarlo a colector().
    MPI_Ssend(&count, 1, MPI_INT, idColector, etiqGenerica, MPI_COMM_WORLD);
    
    //      Iteraciones.
    for (int i = 0; i < count; i++)
    {
        // Espera arbitraria.
        sleep_for(milliseconds(aleatorio<1,100>()));

        numero = aleatorio<1,rangoNum>();

        MPI_Send(&numero,1,MPI_INT, idColector, etiqGenerica, MPI_COMM_WORLD);

        cout << "Generador "<<idGen<<" : Numero "<<numero<<" generado."<<endl;
    }

    cout <<"Generador "<<idGen<<": Finalizado."<<endl;
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
        if (id_propio == 0)
        {
            colector();
        }
        else
        {
            generadores(id_propio);
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