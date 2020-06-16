#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

// VARIABLES AUXILIARES
int aguaTotal = 500;
int procesos = 5;

//  FUNCIONES SECUNDARIAS
//      GENERAR NUM ALEATORIOS
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// MONITOR
class Riegos : public HoareMonitor
{
    private:
        int C;
        CondVar colaControl,esperoAgua;
    public:
        Riegos();
        void abrir_cerrar_valvula(int necesito, int i);
        void control_en_espera();
        void control_rellenado();
} ;

// PROCEDIMIENTOS MONITOR

Riegos::Riegos()
{
    C = aguaTotal;
    colaControl = newCondVar();
    esperoAgua = newCondVar();
}

void Riegos::abrir_cerrar_valvula(int necesito, int i)
{
    cout << "Usuario: "<<i<<" Reviso si hay agua en tanque."<< endl;
    while (C == 0)
    {
        cout << "Usuario: "<<i<<" No hay. Aviso controlador y me duermo. "<< endl;
        colaControl.signal();
        esperoAgua.wait();
        cout << "Usuario: "<<i<<" Vuelvo a revisar. "<< endl;
    }

    cout << "Usuario: "<<i<<" Hay "<<C <<" en tanque. Yo utilizo "<< necesito <<" ."<< endl;

    if (necesito < C)
    {
        C -= necesito;
    }
    else
    {
        C = 0;
    }

    cout << "Usuario: "<<i<<" Tengo el agua. Aviso y me salgo. "<< endl;
    esperoAgua.signal();
}

void Riegos::control_en_espera()
{
    cout << "Controlador: Reviso si hay agua. "<< endl;
    if (C>0)
    {
        cout << "Controlador: Hay agua, "<<C<<" espero. "<< endl;
        colaControl.wait();
    }
    cout << "Controlador: No hay agua.  "<< endl;
}

void Riegos::control_rellenado()
{
    cout << "Controlador: Relleno el tanque. "<< endl;
    C = aguaTotal;
    cout << "Controlador: Aviso. "<< endl;
    esperoAgua.signal();
}

// HEBRAS

void esperar72hrs()
{
    chrono::milliseconds Bar ( aleatorio<720,7200>() );
    cout << "Controlador: Espero por 72 horas... "<< endl;
    this_thread::sleep_for(Bar);
    cout << "...Controlador: Ya han pasado 72 horas "<< endl;
}


void funcion_controlador(MRef<Riegos> monitor, int i)
{
    while (true)
    {
        esperar72hrs();
        monitor->control_en_espera();
        monitor->control_rellenado();
    }
    
}

int generarAgua()
{
    return aleatorio<1,500>();
}

void regar(int i)
{
    chrono::milliseconds Bar ( aleatorio<240,2400>() );
    cout << "Usuario: "<<i<<" Empiezo a regar... "<< endl;
    this_thread::sleep_for(Bar);
    cout << "...Usuario: "<<i<<" Termino de regar. "<< endl;

}

void funcion_usuarios(MRef<Riegos> monitor, int i)
{
    int agua;
    while (true)
    {
        agua = generarAgua();
        monitor->abrir_cerrar_valvula(agua,i);
        regar(i);
    }
    
}

// MAIN

int main()
{
    MRef<Riegos> monitor = Create<Riegos>();
    thread controlador[1], usuarios[procesos];

    for (size_t i = 0; i < 1; i++)
    {
        controlador[i] = thread(funcion_controlador,monitor,i);
    }
    
    for (size_t i = 0; i < procesos; i++)
    {
        usuarios[i] = thread(funcion_usuarios,monitor,i);
    }
    

    for (size_t i = 0; i < 1; i++)
    {
        controlador[i].join();
    }

    for (size_t i = 0; i < procesos; i++)
    {
        usuarios[i].join();
    }
    
    
}