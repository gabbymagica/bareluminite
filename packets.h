#ifndef PACKETS_H
#define PACKETS_H

#include "player.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

uint16_t build_packet(uint8_t packet_id, uint8_t *payload, uint16_t payload_size, uint8_t* out_buffer);
void handle_client_packet(Client* Client, uint8_t client_slot, uint8_t packet_id, uint8_t *payload, uint16_t payload_len);

#endif
