#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h> // termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h> // IO redirection

const char * sysname = "shellax";

enum return_codes {
	SUCCESS = 0,
	EXIT = 1,
	UNKNOWN = 2,
};

struct command_t {
	char *name;
	bool background;
	bool auto_complete;
	int arg_count;
	char **args;
	char *redirects[3]; // in/out redirection
	struct command_t *next; // for piping
};

/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t * command)
{
	int i=0;
	printf("Command: <%s>\n", command->name);
	printf("\tIs Background: %s\n", command->background?"yes":"no");
	printf("\tNeeds Auto-complete: %s\n", command->auto_complete?"yes":"no");
	printf("\tRedirects:\n");
	for (i=0;i<3;i++)
		printf("\t\t%d: %s\n", i, command->redirects[i]?command->redirects[i]:"N/A");
	printf("\tArguments (%d):\n", command->arg_count);
	for (i=0;i<command->arg_count;++i)
		printf("\t\tArg %d: %s\n", i, command->args[i]);
	if (command->next)
	{
		printf("\tPiped to:\n");
		print_command(command->next);
	}


}
/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command)
{
	if (command->arg_count)
	{
		for (int i=0; i<command->arg_count; ++i)
			free(command->args[i]);
		free(command->args);
	}
	for (int i=0;i<3;++i)
		if (command->redirects[i])
			free(command->redirects[i]);
	if (command->next)
	{
		free_command(command->next);
		command->next=NULL;
	}
	free(command->name);
	free(command);
	return 0;
}
/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt()
{
	char cwd[1024], hostname[1024];
    gethostname(hostname, sizeof(hostname));
	getcwd(cwd, sizeof(cwd));
	printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
	return 0;
}
/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command)
{
	const char *splitters=" \t"; // split at whitespace
	int index, len;
	len=strlen(buf);
	while (len>0 && strchr(splitters, buf[0])!=NULL) // trim left whitespace
	{
		buf++;
		len--;
	}
	while (len>0 && strchr(splitters, buf[len-1])!=NULL)
		buf[--len]=0; // trim right whitespace

	if (len>0 && buf[len-1]=='?') // auto-complete
		command->auto_complete=true;
	if (len>0 && buf[len-1]=='&') // background
		command->background=true;

	char *pch = strtok(buf, splitters);
	command->name=(char *)malloc(strlen(pch)+1);
	if (pch==NULL)
		command->name[0]=0;
	else
		strcpy(command->name, pch);

	command->args=(char **)malloc(sizeof(char *));

	int redirect_index;
	int arg_index=0;
	char temp_buf[1024], *arg;
	while (1)
	{
		// tokenize input on splitters
		pch = strtok(NULL, splitters);
		if (!pch) break;
		arg=temp_buf;
		strcpy(arg, pch);
		len=strlen(arg);

		if (len==0) continue; // empty arg, go for next
		while (len>0 && strchr(splitters, arg[0])!=NULL) // trim left whitespace
		{
			arg++;
			len--;
		}
		while (len>0 && strchr(splitters, arg[len-1])!=NULL) arg[--len]=0; // trim right whitespace
		if (len==0) continue; // empty arg, go for next

		// piping to another command
		if (strcmp(arg, "|")==0)
		{
			struct command_t *c=malloc(sizeof(struct command_t));
			int l=strlen(pch);
			pch[l]=splitters[0]; // restore strtok termination
			index=1;
			while (pch[index]==' ' || pch[index]=='\t') index++; // skip whitespaces

			parse_command(pch+index, c);
			pch[l]=0; // put back strtok termination
			command->next=c;
			continue;
		}

		// background process
		if (strcmp(arg, "&")==0)
			continue; // handled before

		// handle input redirection
		redirect_index=-1;
		if (arg[0]=='<')
			redirect_index=0;
		if (arg[0]=='>')
		{
			if (len>1 && arg[1]=='>')
			{
				redirect_index=2;
				arg++;
				len--;
			}
			else redirect_index=1;
		}
		if (redirect_index != -1)
		{
			command->redirects[redirect_index]=malloc(len);
			strcpy(command->redirects[redirect_index], arg+1);
			continue;
		}

		// normal arguments
		if (len>2 && ((arg[0]=='"' && arg[len-1]=='"')
			|| (arg[0]=='\'' && arg[len-1]=='\''))) // quote wrapped arg
		{
			arg[--len]=0;
			arg++;
		}
		command->args=(char **)realloc(command->args, sizeof(char *)*(arg_index+1));
		command->args[arg_index]=(char *)malloc(len+1);
		strcpy(command->args[arg_index++], arg);
	}
	command->arg_count=arg_index;
	return 0;
}

void prompt_backspace()
{
	putchar(8); // go back 1
	putchar(' '); // write empty over
	putchar(8); // go back 1 again
}
/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command)
{
	int index=0;
	char c;
	char buf[4096];
	static char oldbuf[4096];

    // tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write the settings
    // of stdin to oldt
    static struct termios backup_termios, new_termios;
    tcgetattr(STDIN_FILENO, &backup_termios);
    new_termios = backup_termios;
    // ICANON normally takes care that one line at a time will be processed
    // that means it will return if it sees a "\n" or an EOF or an EOL
    new_termios.c_lflag &= ~(ICANON | ECHO); // Also disable automatic echo. We manually echo each char.
    // Those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately.
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);


    //FIXME: backspace is applied before printing chars
	show_prompt();
	int multicode_state=0;
	buf[0]=0;
  	while (1)
  	{
		c=getchar();
		//printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

		if (c==9) // handle tab
		{
			buf[index++]='?'; // autocomplete
			break;
		}

		if (c==127) // handle backspace
		{
			if (index>0)
			{
				prompt_backspace();
				index--;
			}
			continue;
		}
		if (c==27 && multicode_state==0) // handle multi-code keys
		{
			multicode_state=1;
			continue;
		}
		if (c==91 && multicode_state==1)
		{
			multicode_state=2;
			continue;
		}
		if (c==65 && multicode_state==2) // up arrow
		{
			int i;
			while (index>0)
			{
				prompt_backspace();
				index--;
			}
			for (i=0;oldbuf[i];++i)
			{
				putchar(oldbuf[i]);
				buf[i]=oldbuf[i];
			}
			index=i;
			continue;
		}
		else
			multicode_state=0;

		putchar(c); // echo the character
		buf[index++]=c;
		if (index>=sizeof(buf)-1) break;
		if (c=='\n') // enter key
			break;
		if (c==4) // Ctrl+D
			return EXIT;
  	}
  	if (index>0 && buf[index-1]=='\n') // trim newline from the end
  		index--;
  	buf[index++]=0; // null terminate string

  	strcpy(oldbuf, buf);

  	parse_command(buf, command);

  	//print_command(command); // DEBUG: uncomment for debugging

    // restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
  	return SUCCESS;
}
int process_command(struct command_t *command);
int main()
{
	while (1)
	{
		struct command_t *command=malloc(sizeof(struct command_t));
		memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

		int code;
		code = prompt(command);
		if (code==EXIT) break;

		code = process_command(command);
		if (code==EXIT) break;

		free_command(command);
	}

	printf("\n");
	return 0;
}

int process_command(struct command_t *command)
{
	int r;
	if (strcmp(command->name, "")==0) return SUCCESS;

	if (strcmp(command->name, "exit")==0)
		return EXIT;

	if (strcmp(command->name, "cd")==0)
	{
		if (command->arg_count > 0)
		{
			r=chdir(command->args[0]);
			if (r==-1)
				printf("-%s: %s: %s\n", sysname, command->name, strerror(errno));
			return SUCCESS;
		}
	}


	pid_t pid=fork();
	if (pid==0) // child
	{
		/// This shows how to do exec with environ (but is not available on MacOs)
	    // extern char** environ; // environment variables
		// execvpe(command->name, command->args, environ); // exec+args+path+environ

		/// This shows how to do exec with auto-path resolve
		// add a NULL argument to the end of args, and the name to the beginning
		// as required by exec

		// increase args size by 2
		command->args=(char **)realloc(
			command->args, sizeof(char *)*(command->arg_count+=2));

		// shift everything forward by 1
		for (int i=command->arg_count-2;i>0;--i)
			command->args[i]=command->args[i-1];

		// set args[0] as a copy of name
		command->args[0]=strdup(command->name);
		// set args[arg_count-1] (last) to NULL
		command->args[command->arg_count-1]=NULL;

		// I/O redirection
		if (command->redirects[0] != NULL){ // input
			int infile = open(command->redirects[0], O_RDONLY, 0); 
			dup2(infile, STDIN_FILENO);  // redirect STDIN file descriptor to the infile
			close(infile);
		}
		if (command->redirects[1]!= NULL){ // output
			int outfile = open(command->redirects[1], O_RDWR | O_CREAT, 0666); 
			dup2(outfile, STDOUT_FILENO); // redirect STDOUT file descriptor to the outfile
			close(outfile);		
		}
		if (command->redirects[2]!= NULL){ // append
			int appfile = open(command->redirects[2], O_RDWR | O_CREAT | O_APPEND, 0666); 
			dup2(appfile, STDOUT_FILENO); // redirect STDOUT file descriptor to the appfile
			close(appfile);			
		}

		// find the directory containing the executable of the command
		DIR *dir;
		struct dirent *sd;
		char *env_var;
		env_var =getenv("PATH");

		char paths[strlen(env_var)];
		strcpy(paths, env_var);

		char *path_list[20];
		char *token=strtok(paths,":");
		
		int n =0; //path count
		while(token!=NULL){
			path_list[n]=token;
			token=strtok(NULL,":");
			n++;
		}
		int found =0;
		char *dir1;
		for(int j = 0; j<n;j++){
			dir1 = path_list[j];
			dir = opendir(dir1);
			while((sd=readdir(dir))!=NULL){
				
				if((strcmp(sd->d_name,command->name))==0){ //found the file with matching name as the command
					found=1;
					break;
				}
			}
			
			closedir(dir);
			
			if (found==1){
				break;
			}
		}
		// wiseman command
		if(strcmp("wiseman",command->name)==0){

			// open the crontab text file
			FILE *f = fopen("cron.txt","w");

			// generate the necessary string containing time and command information
			char cp[] = "*/";
			char *cpat=" * * * *  /usr/games/fortune | /usr/bin/espeak \n";
			int s = 3 + strlen(cpat);
			char crn[s];
			crn[0]='\0';

			// format for the repeat time
			strcat(crn,cp);
			strcat(crn,command->args[1]);
			strcat(crn,cpat);
			fprintf(f,"%s\n" ,crn);
			fclose(f);
			
			// call to crontab command
			char *crargs[]={"crontab","cron.txt",NULL};
			execv("/usr/bin/crontab",crargs);


		}
		// chatroom command
		if(strcmp("chatroom",command->name)==0){
			execv("chatroom",command->args);
		}
		
		// psvis command
		if(strcmp("psvis",command->name)==0){
			execv("psvis",command->args);
		}

		// custom command
		if(strcmp("blackjack",command->name)==0){
			execv("blackjack",command->args);
		}

		// custom command
		if(strcmp("minezone",command->name)==0){
			execv("minezone",command->args);
		}
		
		if (found==1){
			// create the full path name: path + '/' + command->name
			int new_size = strlen(dir1) + 1 + strlen(command->name);
			char path_name[new_size];
			path_name[0] = '\0';
			strcat(path_name, dir1);
			strcat(path_name, "/");
			strcat(path_name, command->name);


           	if (command->next != NULL){ // piping

				// uniq command
				if(strcmp(command->next->name,"uniq") ==0){

					if(command->next->args[0] != NULL){
					char *args2[] = {"newuniq",command->args[1],command->next->args[0] ,NULL};
					execv("newuniq",args2);
					}
					else{
					char *args2[] = {"newuniq",command->args[1],NULL };
					execv("newuniq",args2);
					}
				}
				else{

                int fd1[2];
				if (pipe(fd1) == -1){
					fprintf(stderr, "Pipe Failed");
					return 1;
				}

                pid_t child_pid = fork();
                if (child_pid == 0){ //child
					close(fd1[0]);
					dup2(fd1[1],STDOUT_FILENO);      // redirect STDOUT file descriptor to the write end of the pipe
					execv(path_name, command->args); // call to the command (output will be written to the pipe)
                }
				else
				{
					waitpid(child_pid, NULL, 0);     // wait for child execution
					close(fd1[1]);
					dup2(fd1[0], STDIN_FILENO);      // redirect STDIN file descriptor to the read end of the pipe
					process_command(command->next);  // recursive call to process_command for the next command 
									    // (input will be read from the pipe)
					
				}
            }}
			else{
				execv(path_name,command->args); 
			}
		}
		exit(0);
	}
	else
	{
    // TODO: implement background processes here
	if (!command->background){
		waitpid(pid, NULL, 0); // wait for child process to finish
	} 
		return SUCCESS;
	}

	printf("-%s: %s: command not found\n", sysname, command->name);
	return UNKNOWN;
}
