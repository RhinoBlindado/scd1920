#include <iostream>
#include <thread>
#include <random> 
#include <chrono> 
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

// 	VARIABLES GLOBALES
const int
	// Numero de Productores.
	numP = 4,					      
	// Numero de Consumidores.
	numC = 5,
	// ID del buffer.
	idBuffer = numP,
	// Etiquetas para el buffer.
	etiq_P = 0,
	etiq_C = 1,
 	num_procesos_esperado = numP + numC + 1 , 	
	// El numero de items debe ser un multiplo de ambos numeros.
	multiplicador = 1,
  num_items = numP * numC * multiplicador,
  tam_vector = 10,
	intervalo = num_items/numP;

// 		FUNCIONES AUXILIARES
// FUNCION RNG
template< int min, int max > int aleatorio()
{
	static default_random_engine generador( (random_device())() );
	static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
	return distribucion_uniforme( generador );
}

//		FUNCIONES DE PROCESOS
// PRODUCTOR - ESPERA RNG
int producir(int inicioRango)
{
   int contador = aleatorio<0,9>();
   sleep_for( milliseconds( aleatorio<10,100>()) );
   cout << "Productor "<< inicioRango << " : ha producido valor " << contador << endl << flush;
   return contador ;
}

// PRODUCTOR
void funcion_productor( int numOrdenP )
{
   int solicitud = 1;
   bool finalizado = false;

   MPI_Status estado;

   while(!finalizado)
   {
      int valor_prod = producir(numOrdenP);
      cout << "Productor "<<numOrdenP<<" : pide permiso al buffer." << endl << flush;
      MPI_Ssend( &solicitud, 1, MPI_INT, idBuffer, etiq_P, MPI_COMM_WORLD );
      MPI_Recv( &solicitud, 1, MPI_INT, idBuffer, etiq_P, MPI_COMM_WORLD, &estado);

      cout << "Productor "<<numOrdenP<<" : Recibida la aceptación del buffer." << endl << flush;

      if(solicitud == -1)
      {
         cout<< "Productor "<<numOrdenP<<": Recibida solicitud de finalización"<<endl;
         finalizado = true;
      }
      else
      {
         cout << "Productor "<<numOrdenP<<" : va a enviar valor " << valor_prod << endl << flush;
         MPI_Ssend( &valor_prod, 1, MPI_INT, idBuffer, etiq_P, MPI_COMM_WORLD);  
      }
   }

   cout << "Productor " <<numOrdenP <<": Finalizado."<<endl;
}

// CONSUMIDOR - ESPERA RNG
void consumir( int valor_cons , int numOrdenC)
{
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor "<<numOrdenC<<" : ha consumido valor " << valor_cons << endl << flush ;
}

// CONSUMIDOR
void funcion_consumidor( int numOrdenC )
{
	int peticion,
       valor_rec = 1 ;

	MPI_Status  estado ;

   bool finalizado = false;

   while(!finalizado)
   {
      MPI_Ssend( &peticion,  1, MPI_INT, idBuffer, etiq_C, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, idBuffer, etiq_C, MPI_COMM_WORLD,&estado );
      cout << "Consumidor "<< numOrdenC <<" : ha recibido valor " << valor_rec << endl << flush ;

      if (valor_rec == -1)
      {
         finalizado = true;
      }
      else
      {
         consumir( valor_rec, numOrdenC );
      }
   }
   cout<<"Consumidor "<< numOrdenC <<": Finalizado."<<endl;
}

// BUFFER
void funcion_buffer()
{
	int buffer[tam_vector],
       solicitud, 
       valor,
       valorAnt=-1,                   
       primera_libre       = 0, 
       primera_ocupada     = 0, 
       num_celdas_ocupadas = 0,
       etiq_aceptable,
       finProd = 0,
       finCons = 0,
       msj = 1;

	MPI_Status estado ;             

   bool finalizado = false, valorRepetido = false;

      while(!finalizado)
      {
         if ( num_celdas_ocupadas == 0 && !valorRepetido)               
         {
            etiq_aceptable = etiq_P ;       
         }
         else if ( num_celdas_ocupadas == tam_vector && !valorRepetido) 
         {
            etiq_aceptable = etiq_C ;
         } 
         else                                         
         {
            etiq_aceptable = MPI_ANY_TAG ;     
         }

         if(finProd < numP || finCons < numC)
         {
            MPI_Recv(&solicitud, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado);

            switch( estado.MPI_TAG )
            {
               case etiq_P:
                  
                  MPI_Ssend(&msj, 1, MPI_INT, estado.MPI_SOURCE, etiq_P, MPI_COMM_WORLD);               
                  if (valorRepetido)
                  {
                     finProd++;
                  }
                  else
                  {
                     MPI_Recv(&valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_P, MPI_COMM_WORLD,&estado);
                  }

                  if (valor == valorAnt)
                  {
                     valorRepetido = true;
                     msj = -1;
                  }
                  else
                  {
                     valorAnt = valor;
                  }

                  buffer[primera_libre] = valor ;
                  primera_libre = (primera_libre+1) % tam_vector ;
                  num_celdas_ocupadas++ ;

                  cout << "Buffer ha recibido valor " << valor << endl ;
               break;

               case etiq_C:
                  valor = buffer[primera_ocupada] ;
                  primera_ocupada = (primera_ocupada+1) % tam_vector ;
                  num_celdas_ocupadas-- ;

                  if (valorRepetido)
                  {
                     valor = msj;
                     finCons++;
                  }

                  cout << "Buffer va a enviar valor " << valor << endl ;
                  MPI_Ssend(&valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_C, MPI_COMM_WORLD);
               break;
            }
         }
         else
         {
            finalizado = true;
         }
	 }
    cout<<"Buffer: Finalizando..."<<endl;

}

// 	FUNCION MAIN
int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;


   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos_esperado == num_procesos_actual )
   {
      if ( id_propio < numP )
         funcion_productor(id_propio);
      else if ( id_propio == idBuffer )
         funcion_buffer();
      else
         funcion_consumidor(id_propio-numP);
   }
   else
   {
      if ( id_propio == 0 ) 
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}
