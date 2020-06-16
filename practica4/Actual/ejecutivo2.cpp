// -----------------------------------------------------------------------------
//   ------------
//   Ta.  T    C
//   ------------
//   A  250  100
//   B  250   80
//   C  500   50
//   D  500   40
//   E 1000   20
//  -------------
//
//  Planificación (con Ts == 250 ms)
//  *---------*----------*---------*--------*
//  | A B C   | A B D E  | A B C   | A B D  |
//  *---------*----------*---------*--------*
//
//
// -----------------------------------------------------------------------------

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", milliseconds(200) );  }
void TareaB() { Tarea( "B", milliseconds( 80) );  }
void TareaC() { Tarea( "C", milliseconds( 50) );  }
void TareaD() { Tarea( "D", milliseconds( 40) );  }
void TareaE() { Tarea( "E", milliseconds( 20) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario
   const milliseconds Ts( 250 );

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now(), fin_espr, fin_sec;

   steady_clock::duration duracionEsperada, duracionReal;


   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;


         switch( i )
         {
            case 1 : fin_espr = ini_sec + milliseconds (230);
                     TareaA(); TareaB(); TareaC();           break ;
            case 2 : fin_espr = ini_sec + milliseconds (240);
                     TareaA(); TareaB(); TareaD(); TareaE(); break ;
            case 3 : fin_espr = ini_sec + milliseconds (230);
                     TareaA(); TareaB(); TareaC();           break ;
            case 4 : fin_espr = ini_sec + milliseconds (220);
                     TareaA(); TareaB(); TareaD();           break ;
         }

         fin_sec = steady_clock::now();
         duracionEsperada = fin_espr - ini_sec ;
         duracionReal = fin_sec - ini_sec ;

         cout << "   Tiempo de duración esperado: " << milliseconds_f(duracionEsperada).count()<<"ms \n"
              << "   Tiempo de duración real:     "<< milliseconds_f(duracionReal).count()<<"ms"<<endl;
         

         if (duracionReal > duracionEsperada + milliseconds(20))
         {
            cout <<"!! Error: Tiempo real ha excedido 20ms de retraso con el tiempo esperado. Abortando."<<endl;
            exit(1);
         }

         ini_sec += Ts ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );
      }
   }
}
