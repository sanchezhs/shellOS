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
#define GREEN "\e[0;32m"

job* processList;

void manejador(int senal){

  int stat, info, size;
  enum status status;
  pid_t pid;
  job* aux;
  size = list_size(processList);

  
  for(int i = size; i >= 1; i--){
    
    aux = get_item_bypos(processList,i);
    pid = waitpid(aux->pgid,&stat,WUNTRACED|WNOHANG|WCONTINUED);
    
    
    if(aux->pgid == pid){
      status = analyze_status(stat,&info);
      block_SIGCHLD();
      
      
      if(status == FINALIZADO && !WIFCONTINUED(stat)){//&& !WIFCONTINUED(stat)
        printf("\nComando %s ejecutado en segundo plano con PID %d ha concluido su ejecucion. Info %d\n",aux->command,aux->pgid,info);
        delete_job(processList,aux);
      }else if(status == REANUDADO){
        printf("Comando %s ejecutado en segundo plano con PID %d ha reanudado su ejecucion\n", aux->command,aux->pgid);
        aux->ground = PRIMERPLANO;
        //delete_job(processList,aux);
      }else if(status == SUSPENDIDO){
        printf("Comando %s ejecutado en segundo plano con PID %d ha suspendido su ejecucion. Info %d\n", aux->command,aux->pgid,info);
        aux->ground = DETENIDO;
      }else if(WIFCONTINUED(stat)){
        printf("Comando %s ejecutado en segundo plano con PID %d ha reanudado su ejecucion\n", aux->command, aux->pgid);
        aux->ground = SEGUNDOPLANO;
       // delete_job(processList,aux);
        }
        
    unblock_SIGCHLD();
    

  }
}
}

int get_internal_command(char* args[], int* status, enum status status_res, int* info, char pwd[]){
   
    int used = 0;
    
    //Comandos internos
    if(strcmp(args[0],"cd")==0){
        used++;
        if(args[1] == NULL)
          chdir(getenv("HOME"));
        else if(chdir(args[1])==-1){
          fprintf(stderr,"Error, %s\n", strerror(errno));
        }

        if(getcwd(pwd,MAX_LINE) == NULL)
          fprintf(stderr, "Error, %s\n", strerror(errno));
        
        
    }else if(strcmp(args[0],"logout")==0){
        exit(0);
    }else if(strcmp(args[0],"jobs")==0){
      used++;
      if(processList->next == NULL)
        printf("No hay tareas\n");
      else
        print_job_list(processList);
      //continue;
    }else if(strcmp(args[0],"fg")==0){
      used++;
      if(empty_list(processList)){

        printf("No hay tareas\n");

      }else{
        
        job* aux;
        
        
        //se coge el primero de la lista
         
        if(args[1] == 0){
          aux = get_item_bypos(processList,1);
          
        } else if(atoi(args[1]) > list_size(processList)){
          printf("Error, no exista esa tarea\n");
          return -1;
        } else
          aux = get_item_bypos(processList, atoi(args[1]));



        aux->ground = PRIMERPLANO;
        set_terminal(aux->pgid);
        if(killpg(aux->pgid,SIGCONT)==-1)
          fprintf(stderr,"Error, %s",strerror(errno));
        waitpid(aux->pgid,status,WUNTRACED);
        set_terminal(getpid());

        status_res = analyze_status(*status, info);

        if(status_res == SUSPENDIDO){
          printf("Comando %s con PID %d ha sido suspendido\n",aux->command,aux->pgid);
          aux->ground = DETENIDO;
        }else{
          printf("Comando %s con PID %d finalizado con info %d\n",aux->command,aux->pgid,*info);
          delete_job(processList,aux);
        }




      }

    }else if(strcmp(args[0],"bg")==0){
      used++;
      if(empty_list(processList)){
        printf("No hay tareas\n");
      }else{
        
        job* aux;
        
        if(args[1] == 0){
          aux = get_item_bypos(processList,1);
        } else if(atoi(args[1]) > list_size(processList)){
          printf("Error, no exista esa tarea\n");
          return -1;
        } else
          aux = get_item_bypos(processList, atoi(args[1]));

        if(aux->ground == SEGUNDOPLANO)
          printf("Comando %s con PID %d ya se encuentra en segundo plano\n",aux->command,aux->pgid);
        else{
          aux->ground = SEGUNDOPLANO;
          killpg(aux->pgid,SIGCONT);
          //set_terminal(getpid());

        }

        





      }




    
    }
    return used;
}


void get_external_command(char* args[], int* status, enum status status_res ,int *info, int* background, int* pid_fork, int* pid_wait){

   *pid_fork = fork();

    //Hijo
    if(*pid_fork == 0){
      new_process_group(getpid());//creo su grupo de procesos

      if(!(*background)) {//primer plano, toma el terminal
        set_terminal(getpid());
      }

      restore_terminal_signals();//restablece las señales
      if(execvp(args[0],args)==-1) {
        printf("Error. Comando %s no encontrado\n",args[0]);
        exit(-1);
      }

    //Padre
    }else{

      // new_process_group(pid_fork);

      if(*background){
        printf("Comando %s ejecutando en segundo plano con pid %d\n\n",args[0],*pid_fork);
        block_SIGCHLD();
        job* cmd = new_job(*pid_fork,args[0],SEGUNDOPLANO);
        add_job(processList,cmd);
        unblock_SIGCHLD();
        //continue;
      }else{//foreground
        
        // set_terminal(pid_fork);
        printf("\n");
        *pid_wait = waitpid(*pid_fork,status, WUNTRACED|WCONTINUED);
        set_terminal(getpid());
        enum status estado = analyze_status(*status,info);

        if(estado == FINALIZADO ){
          printf("Comando %s ejecutado en primer plano con PID %d. Estado %s. Info: %d\n", args[0],*pid_fork,status_strings[estado],*info);
        }else if(estado == SUSPENDIDO){
          printf("Comando %s ejecutado en primer plano con PID %d suspendido\n",args[0],*pid_fork);
          block_SIGCHLD();
          job* cmd = new_job(*pid_fork,args[0],DETENIDO);
          add_job(processList,cmd);
          unblock_SIGCHLD();
        }else if(estado == REANUDADO) {//estado == REANUDADO
          set_terminal(*pid_fork);
          printf("Comando %s ejecutado en primer plano con PID %d ha sido reanudado\n",args[0],*pid_fork);
        }


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
    int info;		            // Informaci�n procesada por analyze_status()

    ignore_terminal_signals();//Ignoro señales
    signal(SIGCHLD, manejador);//handler 
    processList = new_list("Lista de comandos");//creo la lista de comandos

    char pwd[MAX_LINE];       //Aqui guardo el directorio actual
    if(getcwd(pwd,MAX_LINE) == NULL)
        fprintf(stderr, "Error, %s\n", strerror(errno));

    while (1) { // El programa termina cuando se pulsa Control+D dentro de get_command()

      printf(GREEN "COMANDO:" RESET BLUE "%s-> " RESET, pwd);
      fflush(stdout);
      get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el pr�ximo comando
      if (args[0]==NULL) continue; // Si se introduce un comando vac�o, no hacemos nada
      
      
      if(get_internal_command(args, &status, status_res, &info, pwd) == 0)
        get_external_command(args, &status, status_res, &info, &background, &pid_fork, &pid_wait);
     

    } 
}


