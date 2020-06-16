#include <iostream>
#include <thread>
#include <random> 
#include <chrono> 
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

//      VARIABLES GLOBALES
int numProds = 3,
    numCons = 1,
    numBuffer = 1,
    numProcesos = numProds + numCons + numBuffer,
    idBuffer = numProds + 1,
    idCon = numProds,
    etiqProd = 0,
    etiqCons = 1;

//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//      FUNCIONES
void productor(int id)
{
    int dato,msj;

    while (true)
    {
        dato = aleatorio<1,1000>;
        cout << "Productor "<<id<<": voy a generar el dato "<<dato<<endl;

        sleep_for(milliseconds(aleatorio<100,600>()));

        cout << "Productor "<<id<<": dato"<<dato<<"generado, lo envio al bufer."<<endl;
        MPI_Ssend(&dato, 1, MPI_INT, idBuffer, etiqProd, MPI_COMM_WORLD);
        MPI_Recv(&msj, 1, MPIT)
        cout << "Productor "<<id<<": final de envío."<<endl;
    }
}


void consumidor()
{
    int dato;
    MPI_Status staSinUso;

    while(true)
    {
        cout <<"Consumidor: espero dato del búfer."<<endl;
        MPI_Recv(&dato, 1, MPI_INT, idBuffer, etiqCons, MPI_COMM_WORLD, &staSinUso);
        cout<<"Consumidor: dato "<<dato<<" recibido del búfer. Lo consumo."<<endl;

        sleep_for(milliseconds(aleatorio<100,600>()));
        
        cout<<"Consumidor: He finalizado de consumidor."<<endl;
    }
}

void buffer()
{
    int tam = 4,
        cont = 0,
        buff[tam] = {0},
        ultimoProd = -1,
        msjSinUso,
        actProd,
        espera,
        primeraLibre = 0,
        etiqValida;

    MPI_Status estado;

    while (true)
    {
        if(cont > 1)
        {
            etiqValida = MPI_ANY_TAG;
        }
        else
        {
            etiqValida = etiqProd;
        }

        MPI_Recv(&msjSinUso, 1, MPI_INT, MPI_ANY_SOURCE, etiqValida, MPI_COMM_WORLD, &estado);
        

        if (estado.MPI_TAG == etiqCons)
        {
            primeraLibre--;
            cont--;

            cout <<"Búfer: Envio el dato "<<buff[primeraLibre]<<" al consumidor."<<endl;
            MPI_Ssend(&buff[primeraLibre], 1, MPI_INT, idCon, etiqCons, MPI_COMM_WORLD);
        }
        else
        {
            actProd = estado.MPI_SOURCE;   
            if(ultimoProd != actProd)
            {
                MPI_Recv(&buff[primeraLibre], 1, MPI_INT, actProd, etiqProd, MPI_COMM_WORLD, &estado);
                primeraLibre++;
                cont++;

                while (espera)
                {

                }

                ultimoProd = actProd;
            }
            else
            {
                espera = actProd;
            }
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
        if (id_propio == idBuffer)
        {
            buffer();
        }
        else if (id_propio == idCon)
        {
            consumidor();
        }
        else
        {
            productor(id_propio);
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