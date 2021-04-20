/*------------------------------------------------------------------------------
Proyecto Shell de UNIX. Sistemas Operativos
Grados I. Inform�tica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Algunas secciones est�n inspiradas en ejercicios publicados en el libro
"Fundamentos de Sistemas Operativos", Silberschatz et al.

Para compilar este programa: gcc ProyectoShell.c ApoyoTareas.c -o MiShell
Para ejecutar este programa: ./MiShell
Para salir del programa en ejecuci�n, pulsar Control+D
------------------------------------------------------------------------------*/

#include "ApoyoTareas.h" // Cabecera del m�dulo de apoyo ApoyoTareas.c
 
#define MAX_LINE 256 // 256 caracteres por l�nea para cada comando es suficiente
#include <string.h>  // Para comparar cadenas de cars. (a partir de la tarea 2)


#define RED "\e[1;31m"
#define BLUE "\e[1;34m"
#define MAG "\e[1;35m"
#define RESET "\e[0m"

job* processList;

void manejador(int senal){

  int stat, info, size, ok;
  pid_t pid;
  job* aux;
  size = list_size(processList);

  ok = 1;
  for(aux = processList->next; aux != NULL && ok; aux = aux->next){
    if(aux->ground == SEGUNDOPLANO){
      pid = waitpid(aux->pgid,&stat, WNOHANG);
      //printf("\nComando %s ejecutado en segundo plano con PID %d ha concluido\n", aux->command,aux->pgid);
      delete_job(processList,aux);
      //ok = 0;
    }else if(aux->ground == DETENIDO){
      printf("comando suspendido");
    }
  }
}


// --------------------------------------------
//                     MAIN          
// --------------------------------------------

int main(void) {
    char inputBuffer[MAX_LINE]; // B�fer que alberga el comando introducido
    int background;         // Vale 1 si el comando introducido finaliza con '&'
    char *args[MAX_LINE/2]; // La l�nea de comandos (de 256 cars.) tiene 128 argumentos como m�x
                            // Variables de utilidad:
    int pid_fork, pid_wait; // pid para el proceso creado y esperado
    int status;             // Estado que devuelve la funci�n wait
    enum status status_res; // Estado procesado por analyze_status()
    int info;		      // Informaci�n procesada por analyze_status()

    ignore_terminal_signals();//Ignoro señales
    signal(SIGCHLD, manejador);//handler 
    processList = new_list("Lista de comandos");//creo la lista de comandos

    char pwd[MAX_LINE];       //Aqui guardo el directorio actual
    if(getcwd(pwd,MAX_LINE) == NULL)
        fprintf(stderr, "error=%s\n", strerror(errno));

    while (1) { // El programa termina cuando se pulsa Control+D dentro de get_command()
      printf(MAG "COMANDO:" RESET BLUE "%s-> " RESET, pwd);
      fflush(stdout);
      get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el pr�ximo comando
      if (args[0]==NULL) continue; // Si se introduce un comando vac�o, no hacemos nada
      
      if(strcmp(args[0],"cd")==0){

          if(chdir(args[1])==-1){
            fprintf(stderr,"error %s\n", strerror(errno));
          }
          if(getcwd(pwd,MAX_LINE) == NULL)
            fprintf(stderr, "error=%s\n", strerror(errno));
            
      }else if(strcmp(args[0],"logout")==0){
          exit(0);
      }else{
        pid_fork = fork();

        //Hijo
        if(pid_fork == 0){
          new_process_group(getpid());//creo su grupo de procesos

          if(!background) {//primer plano, toma el terminal
        	  set_terminal(getpid());
          }

          restore_terminal_signals();//restablece las señales
          if(execvp(args[0],args)==-1) {
            printf("Error. Comando %s no encontrado\n",args[0]);
            exit(-1);
          }

        //Padre
        }else{

          if(background){
            printf("Comando %s ejecutando en segundo plano con pid %d\n\n",args[0],pid_fork);
            job* cmd = new_job(pid_fork,args[0],SEGUNDOPLANO);
            add_job(processList,cmd);
            continue;
          }else{//foreground, toma el terminal
            pid_wait = waitpid(pid_fork,&status, WUNTRACED | WCONTINUED);
            
            printf("\n");
            set_terminal(getpid());
            enum status estado = analyze_status(status,&info);
            if(estado == FINALIZADO ){
              printf("Comando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d\n", args[0],pid_fork,status_strings[estado],info);
            }else if(estado == SUSPENDIDO){
              job* cmd = new_job(pid_fork,args[0],DETENIDO);
              add_job(processList,cmd);
            }else{//estado == REANUDADO
              
            }


          }
        }
      }

      

      

  /* Los pasos a seguir a partir de aqu�, son:
      (1) Genera un proceso hijo con fork()
      (2) El proceso hijo invocar� a execvp()
      (3) El proceso padre esperar� si background es 0; de lo contrario, "continue" 
      (4) El Shell muestra el mensaje de estado del comando procesado 
      (5) El bucle regresa a la funci�n get_command()
  */
  } // end while
}


