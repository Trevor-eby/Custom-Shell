/* $begin shellmain */
#include "csapp.h"
#include <time.h>
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void help();
void quit();
void print_pid();
void print_ppid();
int change_dir(char *pathname);
void catch_sig();

sigjmp_buf buf;

int main(int argc, char *argv[])
{
    if(!sigsetjmp(buf, 1)){
            signal(SIGINT, catch_sig);
    }
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
	/* Read */
        if(argv[1] == NULL)
        {
            printf("sh257> ");
        } //strcmp(argv[1], "-p") == 0
        else
        {
            printf("%s> ", argv[2]);  
        }           
	    Fgets(cmdline, MAXLINE, stdin);
	    if (feof(stdin))
	        exit(0);

	/* Evaluate */
	    eval(cmdline);
    }
}
/* $end shellmain */
  
void catch_sig(int sig){
    printf("\n");
    siglongjmp(buf, 1);
}

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
{
    char *argv[MAXARGS]; /* Argument list execvp() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
	return;   /* Ignore empty lines */

    if (!builtin_command(argv)) {
        if ((pid = Fork()) == 0) {   /* Child runs user job */
            if (execvp(argv[0], argv) < 0) {
                printf("Execution failed (in fork)\n");
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

	/* Parent waits for foreground job to terminate */
	if (!bg) {
	    int status;
	    if (waitpid(pid, &status, 0) < 0){
		    unix_error("waitfg: waitpid error");
        }
        if(WIFEXITED(status)){
            printf("Process exited with status code %d\n", WEXITSTATUS(status));
        }

	}
	else
	    printf("%d %s", pid, cmdline);
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "exit")) /* quit command */
    {
        //exit(0);
        quit_shell();
    }
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
    {
	    return 1;
    }
    if (!strcmp(argv[0], "help"))
    {
        help();
        return 1;
    }
    if (!strcmp(argv[0], "pid"))
    {
        print_pid();
        return 1;
    }
    if (!strcmp(argv[0], "ppid"))
    {
        print_ppid();
        return 1;
    }
    if (!strcmp(argv[0], "cd"))
    {
        char cwd[256];
        if(argv[1] == NULL){
            if(getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("%s\n", cwd);
            }
            
        }
        else{
            change_dir(argv[1]);
        }
        return 1;
    }
    /* Not a builtin command */
    return 0;                    
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0) /* Ignore blank line */
    {
        return 1;
    } 
    
    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

void quit_shell()
{
    raise(SIGTERM);
}

void print_pid()
{
    printf("%d\n", getpid());
    fflush(stdout);
}

void print_ppid()
{
    printf("%d\n", getppid());
    fflush(stdout);
}

int change_dir(char *argv)
{
    if((chdir(argv)) == 0){
        return 0;
    }
    else{
        return -1;
    }
}

void help()
{
    printf("**********************************************************************\n");
    printf("A Custom Shell for CMSC 257\n");
    printf("   - Trevor Eby\n");
    printf("Usage:\n");
    printf("Help: This command gives information on all the built-in commands.\n");
    printf("cd: Lists the current working directory.\n");
    printf("cd <arg>: Changes the current directory to the directory of the argument.\n");
    printf("pid: Prints the process ID.\n");
    printf("ppid: Prints the parent process ID.\n");
    printf("exit: Exits the shell.\n");
    printf("- Use the command man to get information on non-built-in commands.\n");
    printf("**********************************************************************\n");
    fflush(stdout);
}