#ifndef PACKET_READER_H
#define PACKET_READER_H

#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t *data;
    uint16_t length;
    uint16_t offset;
} PacketReader;

void reader_init(PacketReader *reader, uint8_t *payload, uint16_t length);
uint8_t read_byte(PacketReader *reader);
uint16_t read_2bytes(PacketReader *reader);
float read_float(PacketReader *reader);
void read_string(PacketReader *reader, char *out_string, int max_size);
void skip_bytes(PacketReader *reader, uint16_t count);
void copy_bytes(PacketReader *reader, uint8_t *out_buff, int start, int max_size);

#endif
