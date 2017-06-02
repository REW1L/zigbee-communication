#ifndef DEVICE_ACTIONS_H
#define DEVICE_ACTIONS_H

#include <stdlib.h>

int configure_device(int);

int send_frame(int fd, char *array, int size);

size_t read_from_device(int fd, char *buffer, size_t size);

int get_network_info(int fd);

int join_network(int fd);

int exit_network(int fd);

int create_network(int fd);

#endif /* DEVICE_ACTIONS_H */
