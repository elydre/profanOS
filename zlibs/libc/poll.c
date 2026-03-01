#include <poll.h>
#include <stdint.h>
#include <sys/time.h>
#include <modules/filesys.h>

static uint32_t get_time() {
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
	int res = 0;

	uint32_t time_end = get_time() + timeout;
	while (timeout < 1 && get_time() < time_end && res == 0) {
		for (int i = 0; i < nfds; i++) {
			if (fds[i].fd == -1)
				continue;
			int rw = fm_get_fd_rw(fds[i].fd);
			if (rw == 0)
				continue;
			res++;
			if (rw < 0)
				fds[i].revents |= POLLNVAL;
			else {
				if ((rw & FM_READ) && (fds[i].events & POLLIN))
					fds[i].revents |= POLLIN;
				if ((rw & FM_WRITE) && (fds[i].events & POLLOUT))
					fds[i].revents |= POLLOUT;
			}
		}
		if (timeout == 0)
			break;
	}
	return res;
}
