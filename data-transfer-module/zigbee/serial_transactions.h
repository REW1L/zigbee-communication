#ifndef SERIAL_TRANSACTION_H
#define SERIAL_TRANSACTION_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

int configure_device(const char *device);

int send_frame(int fd, char *array, int size);

int read_from_device(int fd, char *buffer, size_t size);

int get_network_info(int fd);

int join_network(int fd);

int exit_network(int fd);

int create_network(int fd);

#if defined (__cplusplus)
}
#endif
#endif /* SERIAL_TRANSACTION_H */