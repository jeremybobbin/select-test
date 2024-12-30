#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/wait.h>

#define MESSAGE "abcdefghijklmnopqrstuvwxyz\n"
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main() {
	int i, n, pid, out[2], err[2];
	fd_set rs;

	if (pipe(&out[0]) == -1 || pipe(&err[0]) == -1) {
		fprintf(stderr, "parent - pipe: %s\n", strerror(errno));
		return 1;
	}

	switch ((pid = fork())) {
	case -1:
		fprintf(stderr, "parent - fork: %s\n", strerror(errno));
		return 1;
	case 0:
		if (
			close(out[0])   == -1 ||
			close(err[0])   == -1 ||
			dup2(out[1], 1) == -1 ||
			dup2(err[1], 2) == -1
		) {
			fprintf(stderr, "child - dup/close: %s\n", strerror(errno));
			return 1;
		}
		for (i = 0; i < sizeof(MESSAGE)-1; i += n) {
			if ((n = write(2, &MESSAGE[i], sizeof(MESSAGE)-i-1)) == -1) {
				return 1;
			} else if (n == 0) {
				return 1;
			}
		}
		if (close(1) == -1) {
			return 1;
		}
		return 0;
	default:
		if (
			close(out[1])   == -1 ||
			close(err[1])   == -1
		) {
			fprintf(stderr, "parent - dup/close: %s\n", strerror(errno));
			return 1;
		}
	}

	FD_ZERO(&rs);
	FD_SET(out[0], &rs);
	FD_SET(err[0], &rs);

	if ((n = select(MAX(out[0], err[0])+1, &rs, NULL, NULL, NULL)) == -1) {
		fprintf(stderr, "parent - select: %s\n", strerror(errno));
		return 1;
	}

	if (wait(&n) == -1) {
		fprintf(stderr, "parent - wait: %s\n", strerror(errno));
		return 1;
	}

	if (n) {
		fprintf(stderr, "parent - child exited with code: %d\n", n);
		return n;
	}

	switch ((FD_ISSET(out[0], &rs) ? 1 : 0) | (FD_ISSET(err[0], &rs) ? 2 : 0)) {
	case 0:
		fprintf(stdout, "\n");
		break;
	case 1:
		fprintf(stdout, "out\n");
		break;
	case 2:
		fprintf(stdout, "err\n");
		break;
	case 1|2:
		fprintf(stdout, "out, err\n");
		break;
	}

	return 0;
}
