#include "packets.h"
#include "packet_reader.h"
#include "player.h"
#include <stdint.h>
#include <stdio.h>

uint16_t build_packet(uint8_t packet_id, uint8_t *server_payload, uint16_t server_payload_size, uint8_t* out_buffer) {
    uint16_t total_size = server_payload_size + 3;

    out_buffer[0] = (uint8_t)(total_size);
    out_buffer[1] = (uint8_t)(total_size >> 8);
    out_buffer[2] = packet_id;

    if (server_payload_size > 0 && server_payload != NULL) {
        memcpy(out_buffer+3, server_payload, server_payload_size);
    }
    return total_size;
}

void handle_client_packet(Client* client, uint8_t client_slot, uint8_t packet_id, uint8_t *client_payload, uint16_t client_payload_len) {
    uint8_t response_buffer[1024];
    uint16_t response_size;

    switch (packet_id) {
        ////////////////////////////////
        //  INVENTORY SECTION
        ///////////////////////////////

        // connection request
        // returns client_slot
        case 1: {
            printf("pacote de id 1 recebido\n");
            uint8_t server_payload[1] = { client_slot };
            response_size = build_packet(3, server_payload, 1, response_buffer);
            send(client->fd, response_buffer, response_size, 0);
            printf("pacote de id 3 respondido!\n");
            break;
        }
        // sync player
        // returns uma caralhada de coisa
        case 4: {
            PacketReader r;
            reader_init(&r, client_payload, client_payload_len);

            skip_bytes(&r, 8);

            read_string(&r, client->player.name, sizeof(client->player.name));
            client->player.difficulty = client_payload[client_payload_len - 3];
            client->player.slot_id = client_slot;
            client->player.is_active = true;
            client->player.raw_sync_size = build_packet(
                4,
                client_payload,
                client_payload_len,
                client->player.raw_sync_packet
            );

            printf("jogador %s salvo! Cache gerado com %d bytes.\n",
                client->player.name, client->player.raw_sync_size);
            break;
        }

        case 5: {
            PacketReader r;
            reader_init(&r, client_payload, client_payload_len);

            skip_bytes(&r, 1);
            Item *item_slot = &client->player.slots[read_2bytes(&r)];

            item_slot->stack = read_2bytes(&r);
            item_slot->prefix = read_byte(&r);
            item_slot->id = read_2bytes(&r);
            break;
        }

        case 16: {
            PacketReader r;
            reader_init(&r, client_payload, client_payload_len);

            skip_bytes(&r, 1);
            client->player.life = read_2bytes(&r);
            client->player.max_life = read_2bytes(&r);
            break;
        }

        case 42: {
            PacketReader r;
            reader_init(&r, client_payload, client_payload_len);

            skip_bytes(&r, 1);
            client->player.mana = read_2bytes(&r);
            client->player.max_mana = read_2bytes(&r);
            break;
        }

        case 50: {
            PacketReader r;
            reader_init(&r, client_payload, client_payload_len);

            copy_bytes(&r, client->player.buffs, 1, 44);
            break;
        }

        case 68: {
            printf("pulado\n");
            break;
        }

        ////////////////////////////////
        //  WORLD SECTION
        ///////////////////////////////

        // request for world data
        case 9: {
            printf("pulado\n");
            break;
        }

        default: {
            printf("packet of id %d not implemented yet\n", packet_id);
            break;
        }

    }
}
