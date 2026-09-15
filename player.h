#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t stack;
    uint8_t prefix;
    uint16_t id;
} Item;

typedef struct {
    bool is_active;
    uint8_t slot_id;

    char name[32];

    uint16_t life;
    uint16_t max_life;
    uint16_t mana;
    uint16_t max_mana;
    uint8_t buffs[44];

    Item slots[260];

    uint8_t difficulty;

    uint8_t raw_sync_packet[128];
    uint16_t raw_sync_size;
} Player;

typedef struct {
    int fd;
    uint8_t incoming_buffer[1024];
    int buffer_bytes;
    Player player;
} Client;

#endif
