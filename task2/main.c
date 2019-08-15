#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE_BUFF 50

int get_size_token(const char* token)
{
	int i;

	for (i = 0; token[i] && (token[i] != '\n'); ++i);

	return ++i;
}

void cpy(char* str1, const char* str2, int size)
{
	strcpy(str1, str2);

	str1[size - 1] = '\0';
}

void free_arglist(char** arglist, int n)
{
	int i;
	for (i = 0; i < n; ++i) {
		if (arglist[i] != NULL) {
			free(arglist[i]);
			arglist[i] = NULL;
		}
	}
}

int main()
{
	char input_symbol[SIZE_BUFF + 1];
	const char* delim = " ";
	char* savedptr;
	char* token;
	char** arg_list;

	char** redir_arglist;

	int count_arg = 7; // 0 - command, 6 - NULL 
	int size_arg;

	int i = 0;

	pid_t pid;
	int status;

	int fd[2];
	int f_redirect = 0;

	arg_list = malloc(sizeof(char*) * count_arg);
	redir_arglist = malloc(sizeof(char*) * count_arg);

	while (1) {
		printf(">>>");

		fgets(input_symbol, SIZE_BUFF, stdin);

		token = strtok_r(input_symbol, delim, &savedptr);
		if (token == NULL)
			break;

		size_arg = get_size_token(token);
		arg_list[0] = malloc(sizeof(char) * size_arg);
		cpy(arg_list[0], token, size_arg);

		if (!strcmp(arg_list[0], "q"))
			break;

		for (i = 1; (token = strtok_r(NULL, delim, &savedptr))
						&& (i < count_arg - 1); ++i) {
			if ((strcmp(token, "|") != 0) && (f_redirect == 0)) {
				size_arg = get_size_token(token);
				arg_list[i] = malloc(sizeof(char) * size_arg);
				cpy(arg_list[i], token, size_arg);
			} else {
				f_redirect = 1;
				break;
			}
		}

		arg_list[i] = NULL;

		if (f_redirect)
			pipe(fd);

		pid = fork();
		if (pid == 0) {
			if (f_redirect) {
				close(fd[0]);
				dup2(fd[1], 1);	
			}

			execvp(arg_list[0], arg_list);
			perror("Error exec");
			exit(EXIT_FAILURE);
		} else {
			if (f_redirect) {
				for (i = 0; (token = strtok_r(NULL, delim, &savedptr))
						&& (i < count_arg - 1); ++i) {
					size_arg = get_size_token(token);
					redir_arglist[0] = malloc(sizeof(char) * size_arg);
					cpy(redir_arglist[i], token, size_arg);
				}

				redir_arglist[i] = NULL;
			}

			wait(&status);

			if (f_redirect) {
				pid = fork();
				if (pid == 0) {
					close(fd[1]);
					dup2(fd[0], 0);

					execvp(redir_arglist[0], redir_arglist);
					perror("exec(redirect)");
					exit(EXIT_FAILURE);
				} else {
					close(fd[0]);
					close(fd[1]);
					wait(&status);

					free_arglist(redir_arglist, count_arg);

					f_redirect = 0;
				}
			}
		}

		// arg_list[i] = NULL;

		memset(input_symbol, 0, SIZE_BUFF + 1);
		free_arglist(arg_list, count_arg);
	}

	free_arglist(arg_list, count_arg);
	free(arg_list);

	return 0;
}
