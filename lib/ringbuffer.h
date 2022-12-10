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

#include <stddef.h>

struct ring_buffer_t {
	int read_pos;
	int write_pos;
	int buf_size;
	unsigned char *buf;
};

int ring_buffer_init(struct ring_buffer_t *rb, size_t size);
int ring_buffer_free(struct ring_buffer_t *rb);

int ring_buffer_read(struct ring_buffer_t *rb, void *buf, size_t len);
int ring_buffer_write(struct ring_buffer_t *rb, void *buf, size_t len);

int ring_buffer_bytes_used(struct ring_buffer_t *rb);
int ring_buffer_bytes_free(struct ring_buffer_t *rb);
