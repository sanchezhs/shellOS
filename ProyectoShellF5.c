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


    char pwd[MAX_LINE];
    if(getcwd(pwd,MAX_LINE) == NULL)
        fprintf(stderr, "error=%s\n", strerror(errno));

    while (1) { // El programa termina cuando se pulsa Control+D dentro de get_command()

      ignore_terminal_signals();
      printf(MAG "COMANDO:" RESET BLUE "%s->" RESET, pwd);
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
        if(pid_fork == 0){
          new_process_group(getpid());

          if(!background) {//primer plano
        	  set_terminal(getpid());
          }

          restore_terminal_signals();
          if(execvp(args[0],args)==-1) {
        	status_res = analyze_status(status,&info);
            printf("Error. Comando %s no encontrado\n",args[0]);
            exit(-1);
          }

        }else{

          if(background){
            printf("Comando %s ejecutando en segundo plano con pid %d\n\n",args[0],pid_fork);
            continue;
          }if(!background){
          
            waitpid(pid_fork,&status,0);printf("\n");
            set_terminal(getpid());
            enum status estado = analyze_status(status,&info);
            if(estado == FINALIZADO)
              printf("Comando %s ejecutado en primer plano con pid %d.Estado %s. Info: %d\n", args[0],pid_fork,status_strings[estado],info);
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


