#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>
#include <string>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ; 

// VARIABLES GLOBALES
int 
 num_procesos  = 10;

//    FUNCIONES DE PROCESOS

void procesoEtapa (int etapa)
{
   char actual,nuevo;
   bool endOfStream = false;
   int siguienteProc;
   MPI_Status estado;


   if(etapa != num_procesos-1)
      siguienteProc = etapa+1;
   else
      siguienteProc = 0;

  
   MPI_Recv(&actual,1,MPI_CHAR,etapa-1,0,MPI_COMM_WORLD,&estado);
//   cout << "Etapa "<<etapa<<": he recibido el primer caracter " <<actual<<endl;

   if(actual == '\n')
      endOfStream = true;

   while (!endOfStream)
   {
      MPI_Recv(&nuevo,1,MPI_CHAR,etapa-1,0,MPI_COMM_WORLD,&estado);
//      cout << "Etapa "<<etapa<<": he recibido el caracter " <<nuevo<<" lo comparo con "<<actual<<endl;

      if(nuevo != '\n')
      {
         if(actual>nuevo)
         {
//            cout << "Etapa "<<etapa<<": envio el caracter " <<nuevo<<endl;
            MPI_Ssend(&nuevo,1,MPI_CHAR,siguienteProc,0,MPI_COMM_WORLD);
         }
         else
         {
//            cout << "Etapa "<<etapa<<": envio el caracter " <<actual<<endl;
            MPI_Ssend(&actual,1,MPI_CHAR,siguienteProc,0,MPI_COMM_WORLD);
            actual = nuevo;
         }
      }
      else
      {
         MPI_Ssend(&actual,1,MPI_CHAR,siguienteProc,0,MPI_COMM_WORLD);
         endOfStream = true;
         actual = nuevo;
      }
   }
   MPI_Ssend(&actual,1,MPI_CHAR,siguienteProc,0,MPI_COMM_WORLD);
//   cout << "Etapa "<<etapa<<": Fin de cadena, termino."<<endl;
   
}

void inout ()
{
   string input;
   char valor;
   MPI_Status estado;
   cout << "Ingrese una cadena de carácteres: \n>";
   cin >> input;

   input.push_back('\n');
   int count = input.size();

   for (int i = 0; i < count; i++)
   {
//      cout << "inout: Envio el caracter " << input[i] << " al primer proceso" <<endl;
      MPI_Send(&input[i], 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
   }

   input.clear();

   for (int i = 0; i < count; i++)
   {
      MPI_Recv(&valor, 1, MPI_CHAR, num_procesos-1, 0, MPI_COMM_WORLD, &estado );
//      cout << "inout: He recibido el carácter: " <<valor<<endl;
      if(i != count)
         input+=valor;
   }

   cout << "La cadena ordenada es: " << input <<endl;
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
      if (id_propio == 0)
         inout();
      else
         procesoEtapa(id_propio);
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