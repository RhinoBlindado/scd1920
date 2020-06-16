#include <iostream>
#include <thread>
#include <random> 
#include <chrono> 
#include <mpi.h>
#include <list>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

//      VARIABLES GLOBALES
int manos = 10, 
    jugadores = 4,
    idCroupier = 0,
    etiqCarta = 0, 
    etiqApuesta = 1,
    etiqPermiso = 2,
    numProcesos = jugadores + 1;


//      FUNCIONES AUXILIARES
// RNG
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//      PROCESOS
void jugador(int id)
{
    int carta=-1,apuesta=-1,permiso=-1;

    for (int i = 0; i < manos; i++)
    {
        while (carta == apuesta)
        {
            carta = aleatorio<1,5>();
            apuesta = aleatorio<1,5>();
        }

        cout<<"Jugador "<<id<<": Mi carta es "<<carta<<" y mi apuesta es "<<apuesta<<endl;
        
        cout<<"Jugador "<<id<<": Le enseño la carta "<<carta<<" al croupier"<<endl;
        MPI_Ssend(&carta,1,MPI_INT,idCroupier,etiqCarta,MPI_COMM_WORLD);
        
        cout<<"Jugador "<<id<<": Le digo la apuesta "<<apuesta<<" al croupier"<<endl;
        MPI_Ssend(&apuesta,1,MPI_INT,idCroupier,etiqApuesta,MPI_COMM_WORLD);

        cout<<"Jugador "<<id<<": Le pido al croupier que me diga si he ganado"<<endl;
        MPI_Ssend(&permiso,1,MPI_INT,idCroupier,etiqPermiso,MPI_COMM_WORLD);

        cout<<"Jugador "<<id<<": Mano "<<i<<" finalizada"<<endl;
    }
}

void croupier()
{
    int flagCarta,
        flagApuesta,
        flagPermiso,
        msjSinUso,
        msj,
        sumaCartas,
        apuestas[jugadores],
        puntuacion[jugadores] = {0},
        cartas[jugadores];

    MPI_Status estado,estSinUso;

    for (int i = 0; i < manos; i++)
    {
        for (int j = 0; j < jugadores; j++)
        {
            cartas[j] = 0;
            apuestas[j] = 0;
        }
        
        sumaCartas = 0;

        cout<<"Croupier: Compruebo si hay mensajes sobre las cartas."<<endl;
        for (int j = 1; j <= jugadores; j++)
        {
            MPI_Iprobe(j, etiqCarta, MPI_COMM_WORLD, &flagCarta, &estado);
            if (flagCarta)
            {
                MPI_Recv(&msj, 1, MPI_INT, estado.MPI_SOURCE, etiqCarta, MPI_COMM_WORLD, &estSinUso);

                cout<<"Croupier: Carta "<<msj<<" recibida del jugador "<<estado.MPI_SOURCE<<endl;

                cartas[j-1] = msj;
                sumaCartas += msj;
            }
        }


        cout<<"Croupier: Paso a recibir las apuestas."<<endl;
        for (int j = 1; j <= jugadores; j++)
        {
            MPI_Iprobe(j, etiqApuesta, MPI_COMM_WORLD, &flagApuesta, &estado);
            if(cartas[j-1] && flagApuesta)
            {
                MPI_Recv(&msj, 1, MPI_INT, estado.MPI_SOURCE, etiqApuesta, MPI_COMM_WORLD, &estSinUso);

                cout<<"Croupier: Apuesta "<<msj<<" recibida de jugador "<<estado.MPI_SOURCE<<endl;
                apuestas[j-1] = msj;
            }
        }

        
        cout<<"Croupier: Paso a recibir las solicitudes para revisar quien gana esta mano."<<endl;
        for (int j = 1; j <= jugadores; j++)
        {
            MPI_Iprobe(j, etiqPermiso, MPI_COMM_WORLD, &flagPermiso, &estado);
            if (cartas[j-1] && apuestas[j-1] && flagPermiso)
            {
                MPI_Recv(&msjSinUso, 1, MPI_INT, estado.MPI_SOURCE, etiqPermiso, MPI_COMM_WORLD, &estSinUso);

                cout<<"Croupier: Solicitud recibida de "<<estado.MPI_SOURCE<<endl;

                if (apuestas[j-1] == sumaCartas)
                {
                    cout<<"Croupier: Jugador "<<j<< "ha acertado en la predicción, la suma de "<<sumaCartas<<" y su apuesta "<<apuestas[j]<<endl;
                    puntuacion[j-1]++;
                }
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
        if (id_propio == idCroupier)
        {
            croupier();
        }
        else
        {
            jugador(id_propio);
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