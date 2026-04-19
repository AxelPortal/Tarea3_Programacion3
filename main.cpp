#include <iostream>
#include <functional>
#include <list>
// implementamos la libreria map para registrar comandos por nombre
#include <map>
#include <sstream>

using namespace std;

/* Creamos esta funcion auxiliar para validacion numerica */
bool esNumero(const string& s) {
    if (s.empty()) return false;

    if (s == "-") return false;

    int i = 0;
    if (s[0] == '-') i = 1; // permitir negativos

    for (; i < s.size(); i++) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}


class Entity {
    int x, y, vida, nivel, recursos;
    string nombre;

public:
    Entity(): x(0), y(0), vida(100), nivel(1), recursos(0), nombre("Nombre0") {}

    void move (int x_, int y_) {
        x += x_;
        y += y_;
    }

    void heal (int pt_vida) {
        vida += pt_vida;
    }

    void damage (int danio) {
        vida -= danio;
        if (vida < 0)
            {vida = 0;}

    }

    void agregar_recursos(int pt_recursitos) {
        recursos += pt_recursitos;
    }

    void reset() {
        x = 0; y = 0; vida = 100; nivel = 1; recursos = 0; nombre="Nombre0";
    }

    void status () const {
        cout << getStateEntity() << endl;
    }
    void setNombre (const string& n) {
        nombre = n;
    }
    // .....hasta aqui hemos creado los atributos y metodos obligatorio de class Entity....

    string getStateEntity() const {
        stringstream ss;
        ss << "Posicion = ("<<x<<","<<y<<")\nVida = "<<vida<<"\nNivel = "<<nivel<<"\nRecursos = "<<recursos<<"\nNombre = "<<nombre;
        return ss.str();
    }

};


/*Tipo de comando obligatorio (p.4 de la tarea)*/
using Command = function<void(const list<string>&)>;
// creamos un alias de tipo 'Command' el cual pertenece a un function (clase plantilla). Este almacena objetos llamables por referencia constante a una lista tipo string
// y no retorna un valor sino un void, tal cual a como nos lo exige el problema.


/*2. Funcion libre _commando*/
// nota: cada comando debe manejar validacion de argumentos
void curar_Function(Entity& entity, const list<string>& args) {
    // segun el problema, heal espera solo un valor [heal n], por ende si args.size != 1 es error
    if (args.size() != 1) {
        cout << "Error en heal: solo acepta 1 argumento\n";
        return;
    }
    string ct_sum_vida = args.front();
    //validacion de que el argumento sea un numero y no una palabra o letras
    if (!esNumero(ct_sum_vida)) {
        cout << "Error: lo ingresado como argumento no es un numero\n";
        return;
    }
    int value = stoi(ct_sum_vida); // convertimos el string a int
    entity.heal(value); // modificamos el estado de Entity aumentando vida
}


/* 4. Comando como Functor
Requisitos: Ser una clase con operator() - Contener una referencia a Entity - Mantener estado interno propio
*/
class Danio_Functor_Command {
    Entity& entity; //referencia a Entity
    int count = 0; // manejo de estado interno
    list<int>regInterno_Damage;

public:
    Danio_Functor_Command(Entity& e): entity(e) {}
    //clase con operator()
    void operator()(const list<string>& args) {
        // damage espera solo un valor [damage n]
        if (args.size() != 1) {
            cout << "Error: ingresar solo un valor como argumento para damage\n";
            return;
        }
        // manejo del limite de uso
        if (count>=3) {
            cout<<"Limite maximo de ejecuciones damage alcanzado"<<endl;
            return;
        }

        string ct_to_damage = args.front();
        // validacion numerica
        if (!esNumero(ct_to_damage)) {
            cout << "Error: no es numero\n";
            return;
        }

        int value = stoi(ct_to_damage);
        entity.damage(value);

        count++;
        cout<<"Damage Functor ejecutado: "<<count<<endl;
        regInterno_Damage.push_back(value);

    }
};

/* COMMAND CENTER */
class CommandCenter {
    Entity& entity;
    map<string, Command> commands;

    // lista tipo string para HISTORIAL
    list<string> history;

    map<string, list<pair<string, list<string>>>> macros;


public:
    CommandCenter(Entity& e) : entity(e) {}

    // registro comando, recibe nombre (move, damage,...) y el alias de la linea 78, como parametro
    void registerCommand(const string& name, Command cmd) {
        // usamos nuestra variable de tipo map y al nombre del comando le asignamos como valor el objeto Command (funcion invocable de ese comando)
        commands[name] = cmd;   //uso center.registerCommand("move",lambda)
        // es decir, nuestro command tiene como clave el nombre del comando (move, heal,...) el cual estara asociado a un function especifico
    }

    void ejecutar(const string& name, const list<string>& args) {
        // creamos un iterador para que apunte a nuestro commands (q es un map)
        map<string, Command>::iterator it = commands.find(name);

        if (it == commands.end()) {
            cout << "Error: este comando no existe\n";
            return;
        }

        string before = entity.getStateEntity();
        // el iterador al hacer el find(name) ya sabe en que map esta y tmb la clave (comando) pero no a la funcion que esta asociada
        // entonces con 'second' hacemos que asocie esa clave con nuestra funcion asociada [lambda, functor, funcion libre...]
        it->second(args);

        string after = entity.getStateEntity();

        /* Manejo de historial */
        history.push_back("___"+name + "___\n# Historial Antes: \n" + before + "\n# Historial Despues: " + after + "\n---------------");
    }

    void imprimirHistory() {
        list<string>::iterator it;
        cout<<"---Historial---"<<endl;
        for (it = history.begin(); it != history.end(); ++it) {
            cout << *it << endl;
        }
    }
    void eliminarComando(const string& name) {
        map<string, Command>::iterator it = commands.find(name);
        cout << "\n--- Eliminando comando '" << name << "' ---\n";
        if (it == commands.end()) {
            cout << "Error: el comando '" << name << "' no existe\n";
            return;
        }

        commands.erase(it);
        cout << "Comando '" << name << "' eliminado correctamente\n";
    }

    void registerMacro(
    const std::string& name,
    const std::list<std::pair<
        std::string,
        std::list<std::string>
    >>& steps
    ) {
        macros[name] = steps;
    }


    void executeMacro(const std::string& name) {
        map<string, list<pair<string, list<string>>>>::iterator it = macros.find(name);
        if (it == macros.end()) {
            cout << "Error: macro no existe\n";
            return;
        }
        list<pair<string, list<string>>>::iterator step;

        for (step = it->second.begin(); step != it->second.end(); ++step) {

            const string& cmdName = step->first;
            const list<string>& args = step->second;

            // verificar si el comando existe
            map<string, Command>::iterator cmdIt = commands.find(cmdName);

            if (cmdIt == commands.end()) {
                cout << "Error: comando '" << cmdName << "' no existe. Macro detenido.\n";
                return;
            }
            // ejecutar
            ejecutar(cmdName, args);
        }
    }





};



int main() {
    /* Requisitos para main:
    -Creacion de la entidad
    -Creacion del CommandCenter
    -Registro de comandos
    -Ejecucion de comandos validos e invalidos
    -Visualizacion del estado final del sistema
    -Implementar al menos 3 ejemplos para cada implementaci´on que realice.
    */
    Entity entity;
    CommandCenter center_command(entity);

    /* Usamos la Funcion libre */
    center_command.registerCommand("heal",
        [&](const list<string>& args){curar_Function(entity, args);}
    );

    center_command.registerCommand("reset",
    [&](const list<string>& args) {
        if (!args.empty()) {
    cout << "Error: reset no recibe argumentos\n";
    return;
    }
    entity.reset();
    });

    /* 3. Comando como expresion Lambda */
    //asociamos al comando 'move' nuestra funcion lambda
    center_command.registerCommand("move",
        [&](const list<string>& args){

            if (args.size() != 2) {
                cout << "Error move\n";
                return;
            }

            auto it = args.begin();

            if (!esNumero(*it)) {
                cout << "Error: posicion x invalido\n";
                return;
            }
            int x = stoi(*it++);

            if (!esNumero(*it)) {
                cout << "Error: posicion y invalido\n";
                return;
            }
            int y = stoi(*it);

            entity.move(x, y);
        }
    );

    /* 4 Functor */
    Danio_Functor_Command damage(entity);
    center_command.registerCommand("damage", damage);

    /* Comando extra (usamos recursos) */
    center_command.registerCommand("armamento",
        [&](const list<string>& args){

            if (args.size() != 1) {
                cout << "Error, solo admite 1\n";
                return;
            }

            string val = args.front();

            if (!esNumero(val)) {
                cout << "Error: no es numero\n";
                return;
            }

            int r = stoi(val);
            entity.agregar_recursos(r);
        }
    );

    /* Status */
    center_command.registerCommand("status",
        [&](const list<string>&){
            entity.status();
        }
    );

    /* Modificamos nombre */
    center_command.registerCommand("nuevo_nombre",
    [&](const list<string>& args){

        if (args.size() != 1) {
            cout << "Error setname\n";
            return;
        }

        entity.setNombre(args.front());
        }
    );

    /* Ejecucion */

    // ### Comando invalido
    center_command.ejecutar("desconocido", {"10"});

    // ### Cambio nombre
    center_command.ejecutar("nuevo_nombre", {"Dayron"});


    // ### Funcion libre (heal): 3 ejemplos
    center_command.ejecutar("heal", {"10"});
    center_command.ejecutar("heal", {"20"});
    center_command.ejecutar("heal", {"abc"}); // invalido, porque abs no es un numero

    // ### LAMBDA (move): 3 ejemplos
    center_command.ejecutar("move", {"5","5"});
    center_command.ejecutar("move", {"10","20"});
    center_command.ejecutar("move", {"x","10"}); // invalido, porque x no es numero
    center_command.ejecutar("reset", {});
    // ### FUNCTOR (damage): 3 ejemplos
    center_command.ejecutar("damage", {"5"});
    center_command.ejecutar("damage", {"2"});
    center_command.ejecutar("damage", {"1"});
    // ### Ejecuciones para ver contador y límite en damage (Danio_Functor_Command)
    center_command.ejecutar("damage", {"4"}); // se activa el límite

    // ### EXTRA (recursos: en este caso, llamemosle armamento [tematica videojuego: free fire)
    center_command.ejecutar("armamento", {"50"});
    center_command.ejecutar("armamento", {"-10"});
    center_command.ejecutar("armamento", {"abc"}); // invalido, no es numero


    // ### Estado final
    center_command.ejecutar("status", {});

    //Registramos un Macro Command y los ejecutamos
    center_command.registerMacro("combo1",{{"move",{"3","2"}},{"heal",{"10"}}, {"status",{}}});
    center_command.executeMacro("combo1");

    center_command.registerMacro("combo2",{{"move",{"4","4"}},{"heal",{"2"}}, {"damage",{"5"}}});
    center_command.executeMacro("combo2");

    center_command.registerMacro("combo3",{{"move",{"3","6"}},{"heal",{"20"}}, {"nuevo_nombre",{"Saiyuk"}}});
    center_command.executeMacro("combo3");

    //Muestra de macro inexistente
    center_command.executeMacro("Jame Jame Ja");



    /* Historial */
    center_command.imprimirHistory();


    //Eliminar comandos 'heal', 'move' y 'damage'

    center_command.eliminarComando("heal");
    center_command.eliminarComando("move");
    center_command.eliminarComando("damage");

    // Al intentar ejecutar después de eliminar, nos saldrá un error porque los comandos ya no existen
    center_command.ejecutar("heal", {"10"});
    center_command.ejecutar("move", {"20"});
    center_command.ejecutar("damage", {"30"});




    return 0;
}
// main: La salida actualmente : errores - comandos functor - estado - historial
// es decir, cuando hacemos un command.ejecutar, nuestro command center busca la llave del map, se asocia a la funcion asociada (it-second..)
// ingresamos al functor, y como cada command valida argumenos, y luego aplica logica, entonces finaliza, sale y guarda en el historial