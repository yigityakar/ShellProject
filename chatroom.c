#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 
#include <time.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    /* argv[0] : chatroom program name (./chatroom) */
    /* argv[1] : room name (i.e. comp304) */
    /* argv[2] : user name (i.e. mehmet) */

    printf("arguments: %s, %s, %s\n", argv[0], argv[1], argv[2]);
	
    /* generate chatroom and pipe directory names */
    int room_dir_size = strlen("/tmp/chatroom-") + strlen(argv[1]);
    char room_dir_name[room_dir_size];
	room_dir_name[0] = '\0';
	strcat(room_dir_name,"/tmp/chatroom-");
	strcat(room_dir_name, argv[1]);
    printf("room directory: %s\n", room_dir_name);

    /* open the chatroom if it exists, create a new folder if it doesn't */
    DIR *dir;
    struct dirent *sd;
    int name_found = 0;
	if((dir = opendir(room_dir_name)) == NULL){ /* open a new folder */
        printf("no such folder\n");
        mkdir(room_dir_name, 0777);
        dir = opendir(room_dir_name);
    }
    

    int pipe_dir_size = strlen(room_dir_name) + 1 + strlen(argv[2]);
    char pipe_dir_name[pipe_dir_size];
	pipe_dir_name[0] = '\0';
    strcat(pipe_dir_name,room_dir_name);
	strcat(pipe_dir_name,"/");
	strcat(pipe_dir_name, argv[2]);
    printf("pipe directory: %s\n", pipe_dir_name);
    
    name_found = access(pipe_dir_name, F_OK);
 
    if(name_found < 0) {
        /* generate named pipe */
        printf("no user found, creating a new pipe: %s\n", argv[2]);
        mkfifo(pipe_dir_name, 0777);
        
    }

    int room_prompt_size = strlen(argv[1]) + 150;
    char room_prompt_name[room_prompt_size];
	room_prompt_name[0] = '\0';
    strcat(room_prompt_name,"[");
    strcat(room_prompt_name, argv[1]);
	strcat(room_prompt_name,"] ");
    strcat(room_prompt_name, argv[2]);
    strcat(room_prompt_name,": ");

    ;
    int fd;
    
    while(1){ 
        closedir(dir);
        dir = opendir(room_dir_name);
        printf("inside dir: %s\n", room_dir_name);

        pid_t pid = fork();
        if (pid == 0) { // child
            while((sd = readdir(dir)) != NULL){     /* read folder contents */
                if (sd->d_name[0] == '.') { continue; }

                if (strcmp(sd->d_name, argv[2]) == 0) { 
                    /* user's pipe */
                    printf("reading from own pipe: %s %s\n", sd->d_name, argv[2]);
                    char message[100];
                    message[0] = '\0';
                    //while(1) {
                    sleep(1);
                    fd = open(pipe_dir_name, O_RDONLY);
                    read(fd, message, 100);
                    printf("read: %s\n", message);
                    close(fd);

                } 
            }
            exit(0);
        
        }  
        else 
        {
            closedir(dir);
            dir = opendir(room_dir_name);
            char message_sent[80];
            fflush(stdin);
            fgets(message_sent, 50, stdin);
            printf("message: %s\n", message_sent);

            if(message_sent[0] == 'x'){ 
                wait(NULL); 
                kill(pid, SIGTERM);
                closedir(dir);
                return 0;
            }

            pid_t pid2 = fork();
            if (pid2 == 0) { // child
                while((sd = readdir(dir)) != NULL){     /* read folder contents */    
                    pid_t pid3 = fork();
                    if (pid3 == 0) { // child
                    
                    if (sd->d_name[0] == '.') { exit(0); }

                    if (strcmp(sd->d_name, argv[2]) != 0) { 
                        /* other pipes */
                        printf("another pipe: %s\n", sd->d_name);
                        //exit(0);

                        /* generate pipe directory name of the other users */
                        int user_dir_size = strlen(room_dir_name) + 1 + strlen(sd->d_name);
                        char user_dir_name[user_dir_size];
                        user_dir_name[0] = '\0';
                        strcat(user_dir_name,room_dir_name);
                        strcat(user_dir_name,"/");
                        strcat(user_dir_name, sd->d_name);
                        //while(1) {
                        

                        //if (message_sent[0] == 'x'){ exit(0); }

                        char message_to_write[strlen(message_sent)];
                        strcpy(message_to_write, message_sent);
                    
                        strcat(room_prompt_name,message_sent);
                        printf("writing message to %s: %s\n", user_dir_name, room_prompt_name);
                        sleep(1);
                        int fd_user = open(user_dir_name, O_WRONLY);
                        printf("opened %s:\n", user_dir_name);
                        
                        write(fd_user, room_prompt_name, 100);
                        printf("wrote it!\n");
                        close(fd_user);
                        
                        //}
                        exit(0);
                        
                    } else { // own pipe
                        //continue;
                        exit(0);

                    }
                    //exit(0);

                    } else { // parent
                        wait(NULL);
                        kill(pid3, SIGTERM);
                        printf("killing pid for other pipes while %d\n", pid3);
                        continue;

                    }

                }
                exit(0);
            } else { // parent
                wait(NULL);
                kill(pid2, SIGTERM);
                printf("killing pid inside while %d\n", pid2);
                continue;

            }
            wait(NULL);
            printf("killing pid %d\n", pid);
            kill(pid, SIGTERM);
        }
        printf("!!!!! this should be printed once !!!!!\n");
    }
    
    return 0;
    
    
}
