/* 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "ringbuffer.h"
#include <errno.h>

int ring_buffer_init(struct ring_buffer_t *rb, size_t size)
{
    rb->read_pos = 0;
    rb->write_pos = 0;
    rb->buf_size = size;
    rb->buf = (unsigned char*)malloc(size);
    
    if (rb->buf == NULL)
        return ENOMEM;

    return 0;
}

int ring_buffer_free(struct ring_buffer_t *rb)
{
    if (rb->buf) {
        free(rb->buf);
        rb->buf = NULL;
    }
    return 0;
}

int ring_buffer_bytes_used(struct ring_buffer_t *rb)
{
	int n = rb->write_pos - rb->read_pos;
	if (n < 0)
		n += rb->buf_size;
	return n;
}

int ring_buffer_bytes_free(struct ring_buffer_t *rb)
{
	return rb->buf_size - ring_buffer_bytes_used(rb);
}

int ring_buffer_read(struct ring_buffer_t *rb, void *buf, size_t len)
{
	unsigned char *p = (unsigned char *)buf;
	int i = 0;

	for (; i < len; ++i) {
		if (rb->read_pos == rb->write_pos) {
			return i;
		} else {
			*p++ = rb->buf[rb->read_pos++];
			if (rb->read_pos == rb->buf_size) {
				rb->read_pos = 0;
			}
		}
	}

	return i;
}

int ring_buffer_write(struct ring_buffer_t *rb, void *buf, size_t len)
{
	unsigned char *p = (unsigned char *)buf;
	int max_bytes = ring_buffer_bytes_free(rb);
	int i = 0;

	/* Clamp write length to number of bytes */
	if (len > max_bytes) {
		len = max_bytes;
	}

	for (; i < len; ++i) {
		rb->buf[rb->write_pos++] = *p++;
		if (rb->write_pos == rb->buf_size) {
			rb->write_pos = 0;
		}
	}

	return i;
}
