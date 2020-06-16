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

//  idProductores = 0, 1, 2
//  idBuffer = 4
//  idConsumidor = 3

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
    int dato;

    while (true)
    {
        dato = aleatorio<1,1000>();
        cout << "Productor "<<id<<": voy a generar el dato "<<dato<<endl;

        sleep_for(milliseconds(aleatorio<100,600>()));

        cout << "Productor "<<id<<": dato "<<dato<<" generado, lo envio al bufer."<<endl;
        MPI_Ssend(&dato, 1, MPI_INT, idBuffer, etiqProd, MPI_COMM_WORLD);

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
        primeraLibre = 0,
        flag;

    MPI_Status estado;

    while (true)
    {
        if (cont > 1)
        {
            primeraLibre--;
            cont--;

            cout <<"Búfer: Hay mas de 2 datos en búfer. Envio el dato "<<buff[primeraLibre]<<endl;
            MPI_Ssend(&buff[primeraLibre], 1, MPI_INT, idCon, etiqCons, MPI_COMM_WORLD);
        }
        else
        {
            cout <<"Búfer: Menos de 2 datos en búfer."<<endl;
        }
        
        cout<<"Búfer: Chequeo de productores."<<endl;
        for (int i = 0; i < numProds; i++)
        {
            MPI_Iprobe(i, etiqProd, MPI_COMM_WORLD, &flag, &estado);
            cout<<"i="<<i<<" flag="<<flag<<" cont="<<cont<<endl;
            
            if(ultimoProd == i && flag)
            {
                cout<<"Búfer: Petición denegada de productor "<<i<<"."<<endl;
            }

            if (ultimoProd != i && flag && cont < tam)
            {
                cout<<"Búfer: Acepto la petición del productor "<<i<<"."<<endl;
                MPI_Recv(&buff[primeraLibre], 1, MPI_INT, i, etiqProd, MPI_COMM_WORLD, &estado);
                ultimoProd = i;
                primeraLibre++;
                cont++;

            }
            
        }

        sleep_for(milliseconds(aleatorio<100,600>()));

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