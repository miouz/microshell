#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

char **env_g;

int ft_strlen(char *str)
{
	int i = 0;

	while(str[i])
		i++;
	return (i);
}

void	printcmd(char **arg)
{
	printf("cmd begin\n");
	for (int i = 0; arg[i]; i++)
		printf("cmd is : %s\n", arg[i]);
	printf("cmd ends\n");
}

void ft_putstr_fd2(char* msg)
{
	int size = ft_strlen(msg);
	write(2, msg, size);
}

int exec_cmd(int fd_in, int fd_out, int to_close, char **argv)
{
	pid_t pid;

	pid = fork();
	if (pid < 0)
		ft_putstr_fd2("cant fork\n");
   	if (pid == 0)
	{
		// (void)to_close;
		if (to_close >0)
			close(to_close);
		dup2(fd_in, STDIN_FILENO);
		dup2(fd_out, STDOUT_FILENO);
		if (execve(argv[0], argv, env_g) < 0)
			exit(1);
		close(fd_in);
		close(fd_out);
	}
	return (0);
}

int main(int argc, char** argv, char** envp)
{
	//set default fds for cmd;
	int fds[2]={STDIN_FILENO, STDOUT_FILENO};
	int pipe_fd[2];
	int i = 0;

	if (argc < 2)
		return (1);

	env_g = envp;
	argv++;
	while (argv[i])
	{
		if (strcmp(argv[i], "|") == 0)
		{
			argv[i] = NULL;
			if (pipe(pipe_fd) < 0)
				exit(1);
			//give write side to current cmd;
			fds[1] = pipe_fd[1];
			exec_cmd(fds[0], fds[1], pipe_fd[0], argv);
			argv = &argv[i + 1];
			//close read side of pipe before assign new fd for next cmd;
			if (fds[0] != 0)
				close(fds[0]);
			fds[0] = pipe_fd[0];
			//close write side in parent;
			if (fds[1] != 1)
				close(fds[1]);
			i = -1;
		}

		//last cmd of pipe or a cmd alone;
		//no need to creat pipe for next cmd;
		
		else if (strcmp(argv[i], ";") == 0
			|| argv[i + 1] == NULL)
		{
			if (strcmp(argv[i], ";") == 0)
				argv[i] = NULL;
			exec_cmd(fds[0], fds[1], -1, argv);
			//close read side in parent;
			if (fds[0] != 0)
				close(fds[0]);
			fds[0] = 0;
			fds[1] = 1;
			argv = &argv[i + 1];
			i = -1;
			while(waitpid(-1, NULL, 0) > 0)
			{}
		}
		i++;
	}
	return (0);
}
