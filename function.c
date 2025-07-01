#include "minishell.h"

char *external_command[200]; 
int status;
int pid;
extern char prompt[25];
char input_string[50];
Slist *head;
int job_counter = 1;

void external_external(char **external_command) 
{
	int fd = open("external_commands.txt", O_RDONLY);
	if (fd == -1) 
	{
		perror("File open error");
		return;
	}

	char buffer[1];
	char temp[200];
	int i = 0, j = 0;
	int ret;

	while ((ret = read(fd, buffer, 1)) > 0) 
	{
		if (buffer[0] == '\n') 
		{
			temp[j] = '\0';
			external_command[i] = malloc(strlen(temp) + 1);
			strcpy(external_command[i], temp);
			i++;
			j = 0;
		} 
		else if (j < 199) 
		{
			temp[j++] = buffer[0];
		}
	}

	if (j > 0) 
	{
		temp[j] = '\0';
		external_command[i] = malloc(strlen(temp) + 1);
		strcpy(external_command[i], temp);
		i++;
	}

	external_command[i] = NULL;
	close(fd);
}

char *get_command(char *input_string) 
{
	static char command[100];
	int i = 0;
	while (input_string[i] != ' ' && input_string[i] != '\0') 
	{
		command[i] = input_string[i];
		i++;
	}
	command[i] = '\0';
	return command;
}

int check_command_type(char *comm) 
{
	char *builtins[] = {
		"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
		"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
		"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "jobs", "fg", "bg", NULL
	};

	for (int i = 0; builtins[i] != NULL; i++) 
	{
		if (strcmp(builtins[i], comm) == 0)
			return BUILTIN;
	}

	for (int i = 0; external_command[i] != NULL; i++) 
	{
		if (strcmp(external_command[i], comm) == 0)
			return EXTERNAL;
	}

	return NO_COMMAND;
}

void scan_input(char *prompt, char *input_string) 
{
	external_external(external_command);
	signal(SIGINT, signal_handler);
	signal(SIGTSTP, signal_handler); 
	while (1) 
	{
		printf(ANSI_COLOR_GREEN "%s : " ANSI_COLOR_RESET, prompt);
		scanf("%[^\n]", input_string);
		getchar();

		fflush(stdout);
		if (strncmp("PS1", input_string, 3) == 0) 
		{
			char *new = strchr(input_string, '=');
			if (new && *(new + 1) != '\0') 
			{
				strcpy(prompt, new + 1);
			} 
			else 
			{
				printf("Command not found\n");
			}
		} 
		else 
		{
			char *comm = get_command(input_string);
			int res = check_command_type(comm);
			if (res == BUILTIN) 
			{
				execute_internal_commands(input_string);
			} 
			else if (res == EXTERNAL) 
			{
				execute_external_commands(input_string);
			} 
			else 
			{
				printf("Command not found: %s\n", comm);
			}
		}
		memset(input_string,0,strlen(input_string));
	}
}

void execute_internal_commands(char *input_string) 
{
	if (strcmp(input_string, "exit") == 0) 
	{
		exit(0);
	}

	else if (strcmp(input_string, "pwd") == 0) 
	{
		char path[1024];
		if (getcwd(path, sizeof(path)) != NULL) 
		{
			printf("%s\n", path);
		} 
		else 
		{
			perror("getcwd");
		}
	}

	else if (strncmp(input_string, "cd ", 3) == 0) 
	{
		char *dir = input_string + 3;
		if (chdir(dir) != 0) 
		{
			perror("cd");
		}
	}

	else if (strcmp(input_string, "echo $SHELL") == 0) 
	{
		char *shell = getenv("SHELL");
		if (shell)
			printf("%s\n", shell);
		else
			printf("SHELL variable not set\n");
	}

	else if (strcmp(input_string, "echo $$") == 0) 
	{
		printf("%d\n", getpid());
	}

	else if (strcmp(input_string, "echo $?") == 0) 
	{
		printf("%d\n", WEXITSTATUS(status));
	}

	else if(strcmp(input_string,"jobs") == 0)
	{
		print_list(head);
	}

	else if (strcmp(input_string, "fg") == 0)
{
	char cmd[100];
	int p = find_pid(&head, cmd);

	if (p == -1)
	{
		printf("fg: current: no such job\n");
		return;
	}

	int fg_job_id = head->job_id;
	char fg_command[100];
	strcpy(fg_command, head->command);

	printf("%s\n", cmd);  
	kill(p, SIGCONT);
	waitpid(p, &status, 0);  
	delete_first(&head);

	printf("[%d]   Done\t\t\t%s\n", fg_job_id, fg_command);
}


	else if(strcmp(input_string,"bg") == 0)
	{
		char cmd[30];
		int p = find_pid(&head,cmd);
		if (p == -1)
		{
			printf("bg: current: no such job\n");
			return;
		}
		int job_id = head->job_id;
		printf("[%d]+ %s &\n", job_id, cmd);
		kill(p,SIGCONT);
		signal(SIGCHLD,sigchild_handler);
	}
	else if (strncmp(input_string, "wc", 2) == 0)
	{
		char *filename = input_string + 2;

		while (*filename == ' ') filename++;

		FILE *fp;

		if (*filename == '\0') 
		{
			printf("(minishell: reading from stdin — press Ctrl+D to stop)\n");
			fp = stdin;
		}
		else 
		{
			fp = fopen(filename, "r");
			if (!fp) 
			{
				perror("wc");
				return;
			}
		}

		int lines = 0, words = 0, chars = 0;
		char ch;
		int in_word = 0;

		while ((ch = fgetc(fp)) != EOF) 
		{
			chars++;

			if (ch == '\n')
				lines++;

			if (ch == ' ' || ch == '\n' || ch == '\t')
				in_word = 0;
			else if (!in_word) 
			{
				in_word = 1;
				words++;
			}
		}

		printf("%d %d %d", lines, words, chars);
		if (*filename != '\0')
			printf(" %s", filename);
		printf("\n");

		if (fp != stdin)
			fclose(fp);
	}

	else 
	{
		printf("The given command is not supported by minishell\n");
	}
}

void execute_external_commands(char *input_string) 
{
	char *buffer[25];
	int pipe_flag=0;
	char str[100];
	int i=0;
	int j=0;
	int pipe_count=0;
	while(*input_string)
	{
		if(*input_string == '|')
		{
			pipe_flag++;
			pipe_count++;
		}
		if(*input_string != ' ' && *input_string != '\0')
		{
			str[i++]=*input_string;
		}
		else
		{
			str[i]='\0';
			i=0;
			buffer[j]=malloc(strlen(str)+1);
			strcpy(buffer[j],str);
			j++;
		}
		input_string++;
	}
	str  [i]='\0';
	buffer[j]=malloc(strlen(str)+1);
	strcpy(buffer[j],str);
	j++;
	buffer[j]=NULL; 

	pid = fork();
	if (pid == 0) 
	{
		signal(SIGTSTP, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		if (pipe_flag) 
		{
			n_pipe(pipe_count, buffer);

		} 
		else 
		{
			execvp(buffer[0], buffer);
			perror("execvp failed");

		}
	} 
	else if (pid > 0) 
	{
		waitpid(pid, &status, WUNTRACED);
	} 
	else 
	{
		perror("fork failed");
	}
}

void signal_handler(int signum) 
{
	if (signum == SIGINT) 
	{
		
		if (input_string[0] == '\0')
		{
			printf("\n");
			printf(ANSI_COLOR_GREEN"%s : " ANSI_COLOR_RESET, prompt);
		}
		else
			printf("\n");
		fflush(stdout);
	} 
	else if (signum == SIGTSTP) 
	{
		if (input_string[0] == '\0') 
		{
			printf("\n");
			printf(ANSI_COLOR_GREEN "%s : " ANSI_COLOR_RESET, prompt);
		}
		else 
		{
			printf("\n");
			if (pid > 0) 
			{
				insert_at_first(&head, pid, input_string);
			}
		}
		fflush(stdout);
	}
}

void sigchild_handler(int signum) 
{
	if (signum == SIGCHLD)
	{
		int status;
		pid_t p = waitpid(-1, &status, WNOHANG);  
		if (p > 0)
		{
			delete_first(&head); 
		}
	}
}



int insert_at_first(Slist **head, int pid, char *input_string) 
{
	Slist *new = (Slist *)malloc(sizeof(Slist));
	if (new == NULL) 
	{
		perror("malloc");
		return 1;
	}
	new->pid = pid;
	new->job_id = job_counter++; 
	strcpy(new->command, input_string);
	new->link = *head;
	*head = new;
	printf("[%d]+  Stopped\t\t%s\n", new->job_id, new->command);
	return 0;
}

int delete_first(Slist **head) 
{
	if (*head == NULL) 
	{
		return 1;
	}
	Slist *temp = *head;
	*head = (*head)->link;
	free(temp);

	if (*head == NULL)
		job_counter = 1;

	return 0;
}


void print_list(Slist *head)
{
	if(head == NULL)
	{
		printf("world\n");
		printf("List is empty\n");
		return;
	}
	else
	{
		while(head)
		{
			printf("PID NUM is %d\n",head->pid);
			printf("Command is %s\n",head->command);
			head = head->link;
			printf("\n");
		}
	}
}

int find_pid(Slist **head, char *command)
{
	if(*head == NULL)
	{
		return -1;
	}
	else
	{
		strcpy(command,(*head)->command);

	}
	return (*head)->pid;
} 



void n_pipe(int pipe_count, char **arg) 
{
	int index[pipe_count + 1];
	index[0] = 0;
	int j = 1;

	for (int i = 0; arg[i] != NULL; i++) 
	{
		if (strcmp(arg[i], "|") == 0) 
		{
			arg[i] = NULL;
			index[j++] = i + 1;
		}
	}

	int pipefd[pipe_count][2];

	for (int i = 0; i < pipe_count; i++) 
	{
		if (pipe(pipefd[i]) == -1) {
			perror("pipe");
			exit(1);
		}
	}

	for (int i = 0; i <= pipe_count; i++) 
	{
		pid_t pid = fork();

		if (pid == 0) 
		{
			if (i != 0) 
			{
				dup2(pipefd[i - 1][0], 0);
			}

			if (i != pipe_count) 
			{
				dup2(pipefd[i][1], 1);
			}

			for (int k = 0; k < pipe_count; k++) 
			{
				close(pipefd[k][0]);
				close(pipefd[k][1]);
			}

			execvp(arg[index[i]], &arg[index[i]]);
			perror("execvp failed");
			exit(EXIT_FAILURE);
		} 
		else if (pid < 0) 
		{
			perror("fork failed");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < pipe_count; i++) 
	{
		close(pipefd[i][0]);
		close(pipefd[i][1]);
	}

	for (int i = 0; i <= pipe_count; i++) 
	{
		wait(NULL);
	}
}

