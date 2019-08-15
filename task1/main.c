#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main()
{
	ssize_t ret;

	char buf[10];
	char* ptr = buf;
	off_t len = (off_t) 10;

	int fd[2];
	pid_t pid;

	int status;

	pipe(fd);

	pid = fork();
	if (pid == 0) {
		close(fd[0]);

		ret = write(fd[1], "Hello\n", 6);
		if (ret == -1) {
			perror("write");
			close(fd[1]);
			exit(EXIT_FAILURE);
		}

		close(fd[1]);
	} else {
		close(fd[1]);

		while ((len != 0) && (ret = read(fd[0], ptr, len)) != 0) {
			if (ret == -1) {
				if (errno == EINTR)
					continue;
				perror("read");
				break;
			}

			len -= ret;
			ptr += ret;
		}

		printf("Perent: %s", buf);

		wait(&status);

		close(fd[0]);
	}

	return 0;
}
