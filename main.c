#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <poll.h>
#include <time.h>
#include <sys/poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include "packets.h"
#include "player.h"

#define MAX_CLIENTS 10
#define TICK_RATE_MS 16 // 60 tps

uint64_t get_time_micro() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        printf("failed to read socket flags\n");
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        printf("failed to define socket flags\n");
        return -1;
    }

    return 0;
}

int main_loop(int server_fd) {
    struct pollfd fds[MAX_CLIENTS];
    Client clients[MAX_CLIENTS];
    int nfds = 1;

    set_nonblocking(server_fd);

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    clients[0].fd = server_fd;

    for (int i = 1; i < MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        clients[i].fd = -1;
    }

    while (1) {
        unsigned long last_timer_update = get_time_micro();
        int ready = poll(fds, nfds, -1);
        if (ready < 0) {
            printf("Error in poll\n");
            break;
        }

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (new_client_fd >= 0) {
                set_nonblocking(new_client_fd);

                for (int i = 1; i < MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = new_client_fd;
                        fds[i].events = POLLIN;
                        clients[i].fd = new_client_fd;
                        if (i >= nfds) nfds = i + 1;
                        printf("new client on slot %d\n", i);
                        break;
                    }
                }
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) continue;

            if (fds[i].revents & POLLIN) {
                Client *client = &clients[i];

                int space_left = sizeof(client->incoming_buffer) - client->buffer_bytes;

                int bytes_read = recv(client->fd, client->incoming_buffer + client->buffer_bytes, space_left, 0);

                // recv returns 0 for disconnection and -1 for error
                if (bytes_read <= 0) {
                    printf("client %d disconnected\n", i);
                    close(client->fd);
                    fds[i].fd = -1;
                    client->fd = -1;
                    client->buffer_bytes = 0;
                    memset(client->incoming_buffer, 0, sizeof(client->incoming_buffer));
                    continue;
                }

                client->buffer_bytes += bytes_read;
                while (client->buffer_bytes >= 2) {
                    uint16_t packet_length = (uint8_t)client->incoming_buffer[0] |
                        (((uint8_t)client->incoming_buffer[1]) << 8);

                    if (packet_length > sizeof(client->incoming_buffer)) {
                        printf("client %d sent packet_length bigger than buffer\n", i);
                        close(client->fd);
                        fds[i].fd = -1;
                        client->fd = -1;
                        client->buffer_bytes = 0;
                        memset(client->incoming_buffer, 0, sizeof(client->incoming_buffer));
                        break;
                    }

                    if (client->buffer_bytes < packet_length) {
                        break;
                    }

                    // packet resolved
                    uint8_t packet_id = (uint8_t)client->incoming_buffer[2];

                    printf("Recebido [ID: %d | Tamanho: %d bytes]: ", packet_id, packet_length);

                    for (int b = 0; b < packet_length; b++) {
                        printf("%02X ", (uint8_t)client->incoming_buffer[b]);
                    }
                    printf("\n");

                    uint8_t *payload = client->incoming_buffer + 3;
                    uint16_t payload_len = packet_length - 3;

                    handle_client_packet(client, i, packet_id, payload, payload_len);

                    // move the next message bytes buffer to the start and continue the loop
                    int remaining_bytes = client->buffer_bytes - packet_length;
                    if (remaining_bytes > 0) {
                        memmove(client->incoming_buffer, client->incoming_buffer + packet_length, remaining_bytes);
                    }
                    client->buffer_bytes = remaining_bytes;
                }
            }
        }
    }

    return 0;
}

int main() {
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){
        printf("creating the socket failed (%d)\n\n", server_fd);
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(7777);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        printf("binding the socket failed!\n\n");
        return -1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        printf("listen failed!\n\n");
        return -1;
    }

    main_loop(server_fd);

    return 0;
}
