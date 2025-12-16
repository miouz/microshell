#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

char **env;

int ft_putstr_fd2(char *msg)
{
	int i = 0;
	while (msg[i])
		i++;
	write(2, msg, i);
	return 1;
}

void msg_exit(void)
{
	ft_putstr_fd2("error: fatal\n");
	exit(EXIT_FAILURE);
}
  
int ft_cd(char **argv)
{
	int i = 0;
	while(argv[i])
		i++;
	if ( i != 2)
		return (ft_putstr_fd2("error: cd: bad arguments\n"));
	if (chdir(argv[1]) < 0)
	{
		ft_putstr_fd2("error: cd: cannot change directory to ");
		ft_putstr_fd2(argv[1]);
		ft_putstr_fd2("\n");
		return (1);
	}
	return 0;	
}

int exec_cmd(char **argv, int fd_in, int fd_out, int fd_to_close)
{
	if (!argv || !argv[0])
		return EXIT_FAILURE;
	if (argv && strcmp(argv[0], "cd") == 0)
		return (ft_cd(argv));

	pid_t pid = fork();
	if (pid < 0)
		msg_exit();
	if (pid == 0)
	{
		if (fd_in != -1)
		{
			if (dup2(fd_in, STDIN_FILENO) < 0)
				msg_exit();
			close(fd_in);
		}
		if (fd_out != -1)
		{
			if (dup2(fd_out, STDOUT_FILENO) < 0)
				msg_exit();
			close(fd_out);		
		}
		if (fd_to_close != -1)
			close(fd_to_close);
		if (execve(argv[0], argv, env) != 0)
		{
			ft_putstr_fd2("error: cannot execute ");
			ft_putstr_fd2(argv[0]);
			ft_putstr_fd2("\n");
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}

int main(int argc, char **argv, char **envp)
{
	if (argc < 2)
		msg_exit();

	env = envp;
	int i = 0;
	int fds[2] = {-1, -1};
	int pipefd[2];

	argv++;
	while (argv[i])
	{
		//pipe or first cmd;
		if (strcmp(argv[i], "|") == 0)
		{
			argv[i] = NULL;
			if (pipe(pipefd) < 0)
				msg_exit();
			fds[1] = pipefd[1];
			exec_cmd(argv, fds[0], fds[1], pipefd[0]);
			//close last read;
			if (fds[0] != -1)
				close(fds[0]);
			//give current read to next cmd;
			fds[0] = pipefd[0];
			//close current write;
			if (fds[1]!= -1)
				close(fds[1]);
			argv = &argv[i + 1];
			i = -1;
		}

		else if (argv[i] && (strcmp(argv[i], ";") == 0 
				|| argv[i + 1] == NULL))
		{
			if (strcmp(argv[i], ";") == 0)
				argv[i] = NULL;	
			exec_cmd(argv, fds[0], -1, -1);
			argv = &argv[i + 1];
			i = -1;
			while(waitpid(-1, NULL, 0) > 0)
				continue;
			if (fds[0] != -1)
				close(fds[0]);
			fds[0] = -1;
			fds[1] = -1;
		}
		i++;
	}
	return (EXIT_SUCCESS);
}






