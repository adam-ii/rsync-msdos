/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "rsync.h"
#include "lib/ringbuffer.h"

#define MAX_PIPE_FIFOS 2

/* Use the upper end of the FD_SET range for pipe "file descriptors" */
#define MIN_PIPE_FD (FD_SETSIZE - (2 * MAX_PIPE_FIFOS))
#define MAX_PIPE_FD (FD_SETSIZE - 1)

static struct ring_buffer_t ring_buffer_fds[MAX_PIPE_FIFOS];

static struct ring_buffer_t *dos_pipe_get_ring_buffer(int fd)
{
	int offset = fd - MIN_PIPE_FD;
	int index = offset / 2;

	if (index < 0 || index >= MAX_PIPE_FIFOS) {
		rprintf(FERROR, "dos_pipe_get_ring_buffer(%d) file descriptor out of range\n", fd);
		return NULL;
	}

	return ring_buffer_fds + index;
}

static int dos_pipe_is_read_end(int fd)
{
	assert(dos_pipe_get_ring_buffer(fd));
	
	/* Read pipe handles are allocated in even increments from MIN_PIPE_FD */
	return ((fd - MIN_PIPE_FD) & 1) == 0;
}

int dos_pipe_open(int pipefd[2], int size)
{
	int i;

	for (i = 0; i < MAX_PIPE_FIFOS; ++i) {
		/* Find the first available buffer */
		struct ring_buffer_t *ptr = ring_buffer_fds + i;
		if (ptr->buf == NULL) {
			if (ring_buffer_init(ptr, size) == ENOMEM) {
				out_of_memory("dos_pipe_open");
			}

			pipefd[0] = MIN_PIPE_FD + (i * 2);
			pipefd[1] = pipefd[0] + 1;
			return 0;
		}
	}

	return EMFILE;
}

int dos_close_fd(int fd)
{
	if (fd >= MIN_PIPE_FD) {
		struct ring_buffer_t *rb = dos_pipe_get_ring_buffer(fd);
        if (rb) {
            ring_buffer_bytes_free(rb);
            return 0;
        }
        return EBADF;
    } else {
        return close(fd);
    }
}

int dos_read_fd(int fd, void *buf, size_t len)
{
	if (fd >= MIN_PIPE_FD) {
		struct ring_buffer_t *rb = dos_pipe_get_ring_buffer(fd);

		if (verbose > 3)
			rprintf(FINFO, "dos_read_fd(%d,%u) fifo used=%d\n", fd, len, ring_buffer_bytes_used(rb));

		assert(dos_pipe_is_read_end(fd));
		return ring_buffer_read(rb, buf, len);
	} else {
#ifdef MSDOS
		return recv(fd, buf, len, 0);
#else
		return read(fd, buf, len);
#endif
	}
}

int dos_write_fd(int fd, void *buf, size_t len)
{
	if (fd >= MIN_PIPE_FD) {
		struct ring_buffer_t *rb = dos_pipe_get_ring_buffer(fd);

		if (verbose > 3)
			rprintf(FINFO, "dos_write(%d,%u) fifo free=%d\n", fd, len, ring_buffer_bytes_free(rb));

		assert(!dos_pipe_is_read_end(fd));
		return ring_buffer_write(rb, buf, len);
	} else {
#ifdef MSDOS
		return send(fd, buf, len, 0);
#else
		return write(fd, buf, len);
#endif
	}
}

int dos_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int n = 0;
	int select_rc = 0;

	if (nfds >= MIN_PIPE_FD) {
		int fd;

		if (verbose > 3)
			rprintf(FINFO, "dos_select(%d,%p,%p,%p)\n", nfds, readfds, writefds, exceptfds);

		for (fd = MIN_PIPE_FD; fd < nfds; ++fd) {
			if (readfds && FD_ISSET(fd, readfds)) {
				struct ring_buffer_t *rb = dos_pipe_get_ring_buffer(fd);

				if (ring_buffer_bytes_used(rb) == 0) {
					/* No data available to read */
					if (verbose > 3)
						rprintf(FINFO, "dos_select readfd FD_CLR(%d)\n", fd);
					FD_CLR(fd, readfds);
				} else {
					n++;
				}
			} else if (writefds && FD_ISSET(fd, writefds)) {
				struct ring_buffer_t *rb = dos_pipe_get_ring_buffer(fd);

				if (ring_buffer_bytes_free(rb) == 0) {
					/* No space available to write */
					if (verbose > 3)
						rprintf(FINFO, "dos_select writefd FD_CLR(%d)\n", fd);
					FD_CLR(fd, writefds);
				} else {
					n++;
				}
			}
		}

		/* Allow select() to iterate the remaining fds */
		nfds = 0;

		for (fd = MIN_PIPE_FD - 1; fd >= 0; --fd) {
			if (readfds && FD_ISSET(fd, readfds)) {
				if (verbose > 4)
					rprintf(FINFO, "dos_select read FD_ISSET(%d)\n", fd);
				nfds = fd + 1;
				break;
			}

			if (writefds && FD_ISSET(fd, writefds)) {
				if (verbose > 4)
					rprintf(FINFO, "dos_select write FD_ISSET(%d)\n", fd);
				nfds = fd + 1;
				break;
			}

			if (exceptfds && FD_ISSET(fd, exceptfds)) {
				rprintf(FINFO, "dos_select except FD_ISSET(%d)\n", fd);
				nfds = fd + 1;
				break;
			}
		}

		if (nfds == 0) {
			/* No more file handles in the fd_sets */
			return n;
		}
	}

	select_rc = select(nfds, readfds, writefds, exceptfds, timeout);
	if (select_rc < 0)
		return select_rc;

	return n + select_rc;
}
