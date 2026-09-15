#include "packet_reader.h"
#include <stdint.h>
#include <string.h>

void reader_init(PacketReader *reader, uint8_t *payload, uint16_t length) {
    reader->data = payload;
    reader->length = length;
    reader->offset = 0;
}

// read a byte
uint8_t read_byte(PacketReader *reader) {
    if (reader->offset >= reader->length) return 0;
    return reader->data[reader->offset++];
}

// read uint16_t
uint16_t read_2bytes(PacketReader *reader) {
    if (reader->offset >= reader->length) return 0;

    //little endian
    uint16_t uint16 = (uint8_t)reader->data[reader->offset] |
        (((uint8_t)reader->data[reader->offset + 1]) << 8);
    reader->offset += 2;
    return uint16;
}

// read a float
float read_float(PacketReader *reader) {
    if (reader->offset + 4 > reader->length) return 0.0f;

    float value;
    memcpy(&value, &reader->data[reader->offset], 4);
    reader->offset += 4;
    return value;
}

// read string
void read_string(PacketReader *reader, char *out_string, int max_size) {
    // first byte say the size of the string
    uint8_t str_len = read_byte(reader);

    if (reader->offset + str_len > reader->length) return;

    int bytes_to_copy = (str_len < max_size - 1) ? str_len : (max_size - 1);

    memcpy(out_string, &reader->data[reader->offset], bytes_to_copy);

    out_string[bytes_to_copy] = '\0';

    reader->offset += str_len;
}

// this DOES NOT alter offset, use only for some optimizations
void copy_bytes(PacketReader *reader, uint8_t *out_buff, int start, int max_size) {
    if (reader->offset + max_size > reader->length) return;
    memcpy(out_buff, &reader->data[start], max_size);
}

void skip_bytes(PacketReader *reader, uint16_t count) {
    if (reader->offset + count <= reader->length) {
        reader->offset += count;
    } else {
        reader->offset = reader->length;
    }
}
