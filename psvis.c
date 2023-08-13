#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/module.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/types.h>



int main(int argc, char *argv[]){
    /* argv[0] : program name */
    /* argv[1] : root pid */
    /* argv[2] : output file name (.png) */
    
    /* install kernel module */
    pid_t pid_ins = fork();
    if (pid_ins == 0){
        
        char pidd[25];
        pidd[0] = '\0';

        strcat(pidd, "root_pid=");
        strcat(pidd, argv[1]);

        char *mod_args[]={"insmod","./newmodule.ko",pidd,NULL};
        execv("/usr/sbin/insmod", mod_args);

    } else {
        wait(NULL);
    }
    
    /* remove kernel module */
    pid_t pid_rm = fork();
    if (pid_rm == 0){
        char *mod_args[]={"rmmod","newmodule",NULL};
        execv("/usr/sbin/rmmod", mod_args);

    } else {
        wait(NULL);
    }

    /* display outputs of the kernel module */
    pid_t pid_dmesg = fork();
    if (pid_dmesg == 0){
        char *mod_args[]={"dmesg","-c",NULL};
        
        int outfile = open("out.txt",O_RDWR | O_CREAT,0666);
        dup2(outfile,1); // redirect STDOUT to out.txt file to store its contents
        execv("/usr/bin/dmesg", mod_args);
        

    } else {
        wait(NULL);
    }

    /* create a pipe to transfer kernel output to 'dot' command */
    int fd1[2];
	if (pipe(fd1) == -1){
		fprintf(stderr, "Pipe Failed");
		return 1;
	}

    pid_t pid = fork();
    if (pid == 0){ 
        /* child writes the digraph information from the kernel module output to the pipe */
        close(fd1[0]);
		dup2(fd1[1],1); // redirect STDOUT to point the write end of the pipe
        
        FILE *f = fopen("out.txt","r");

        char line[200];
        int digraph_begin = 0;
        while (fgets(line,200,f)){
            int i = 0;
            while(line[i]!=']') i++;
            i += 2; //skip ']' and the space character

            if(line[i]=='D') digraph_begin = 1; // beginning of digraph
            
            if(digraph_begin == 1) { // inside the digraph information, print the contents

                if(line[i]=='}'){ // end of digraph
                    printf("%c", '}');
                    break;
                }

                for (int k=i; k< strlen(line); k++){
                    printf("%c", line[k]);
                }
            }
        }

        fclose(f);

    } else {
        wait(NULL);
        remove("out.txt");

        close(fd1[1]);
		dup2(fd1[0], 0); //redirect STDIN to read end of the pipe
        
        int f_png = open(argv[2],O_RDWR | O_CREAT,0666);
        dup2(f_png, 1); // redirect STDOUT to the output .png file

        char *dot_args[]={"dot","-Tpng",NULL};
        execv("/usr/bin/dot", dot_args);
        
        return 0;
    }   
    
}