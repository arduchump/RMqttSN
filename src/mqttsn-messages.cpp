/*
mqttsn-messages.cpp
Copyright (C) 2013 Housahedron

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>

#include "mqttsn-messages.h"
#include "mqttsn.h"

MQTTSN::MQTTSN() :
waiting_for_response(false),
reg_msg_id(0),
topic_count(0)
{
    memset(topic_table, 0, sizeof(topic) * MAX_TOPICS);
    memset(message_buffer, 0, MAX_BUFFER_SIZE);
    memset(response_buffer, 0, MAX_BUFFER_SIZE);
}

MQTTSN::~MQTTSN() {
}

bool MQTTSN::wait_for_response() {
    return waiting_for_response;
}

uint16_t MQTTSN::bswap(const uint16_t val) {
    return (val << 8) | (val >> 8);
}

uint16_t MQTTSN::hash_P(const char* str) {
    uint16_t hash = 5381;
    char c = pgm_read_byte(str);

    while (c) {
        hash = ((hash << 5) + hash) + c;
        c = pgm_read_byte(++str);
    }

    return hash;
}

uint16_t MQTTSN::hash(const char* str) {
    uint16_t hash = 5381;
    char c = str[0];

    while (c) {
        hash = ((hash << 5) + hash) + c;
        c = *++str;
    }

    return hash;
}

bool MQTTSN::connack_handler(msg_connack* msg) {
    return msg->return_code == 0;
}

void MQTTSN::disconnect_handler(msg_disconnect* msg) {
}

bool MQTTSN::regack_handler(msg_regack* msg) {
    uint16_t msg_id = bswap(msg->message_id);

    if (msg_id == reg_msg_id && msg->return_code == 0) {
        for (uint8_t i = 0; i < topic_count; ++i) {
            if (topic_table[i].hash == msg_id) {
                topic_table[i].id = bswap(msg->topic_id);
                break;
            }
        }
        return true;
    }
    return false;
}

void MQTTSN::pingresp_handler() {
}

void MQTTSN::puback_handler(msg_puback* msg) {
}

void MQTTSN::parse_stream() {
    if (Serial.available() > 0) {
        uint8_t* response = response_buffer;
        uint8_t packet_length = (uint8_t)Serial.read();
        *response++ = packet_length--;

        while (packet_length > 0) {
            while (Serial.available() > 0) {
                *response++ = (uint8_t)Serial.read();
                --packet_length;
            }
        }

        dispatch();
        waiting_for_response = false;
    }
}

void MQTTSN::dispatch() {
    message_header* response_message = (message_header*)response_buffer;

    switch (response_message->type) {
    case CONNACK:
        connack_handler((msg_connack*)response_buffer);
        break;

    case REGACK:
        regack_handler((msg_regack*)response_buffer);
        break;

    case PUBACK:
        puback_handler((msg_puback*)response_buffer);
        break;

    case PINGRESP:
        pingresp_handler();
        break;

    case DISCONNECT:
        disconnect_handler((msg_disconnect*)response_buffer);
        break;

    default:
        return;
    }
}

void MQTTSN::send_message() {
    message_header* hdr = (message_header*)message_buffer;
    Serial.write(message_buffer, hdr->length);
    Serial.flush();
}

void MQTTSN::connect(const uint8_t flags, const uint16_t duration, const char* client_id) {
    waiting_for_response = true;

    msg_connect* msg = (msg_connect*)message_buffer;

    msg->length = sizeof(message_header) + sizeof(msg_connect) + strlen(client_id);
    msg->type = CONNECT;
    msg->flags = flags;
    msg->protocol_id = PROTOCOL_ID;
    msg->duration = bswap(duration);
    strcpy(msg->client_id, client_id);

    send_message();
}

void MQTTSN::disconnect(const uint16_t duration) {
    waiting_for_response = true;

    msg_disconnect* msg = (msg_disconnect*)message_buffer;

    msg->length = sizeof(message_header);
    msg->type = DISCONNECT;

    if (duration > 0) {
        msg->length += sizeof(msg_disconnect);
        msg->duration = bswap(duration);
    }

    send_message();
}

bool MQTTSN::register_topic(const char* name) {
    if (/*ctx->state == CONNECTED && */!waiting_for_response && topic_count < MAX_TOPICS) {
        waiting_for_response = true;
//        ctx->state = REGISTERING;
        reg_msg_id = hash(name);

//        strcpy(topic_table[topic_count].name, name);
        topic_table[topic_count].name = name;
        topic_table[topic_count].id = 0;
        topic_table[topic_count].hash = reg_msg_id;
        ++topic_count;

        msg_register* msg = (msg_register*)message_buffer;

        msg->length = sizeof(message_header) + sizeof(msg_register) + strlen(name);
        msg->type = REGISTER;
        msg->topic_id = 0;
        msg->message_id = bswap(reg_msg_id);
        strcpy(msg->topic_name, name);

        send_message();
        return true;
    }

    return false;
}

void MQTTSN::publish(const uint8_t flags, const uint16_t topic_id, const uint16_t message_id, const void* data, const uint8_t data_len) {
    // TODO: We don't do any resending for QoS1 and there's no handshaking for QoS2, we just wait patiently for a response.
    if ((flags & QOS_MASK) == FLAG_QOS_1 || (flags & QOS_MASK) == FLAG_QOS_2) {
        waiting_for_response = true;
    }

//    ctx->state = PUBLISHING;

    msg_publish* msg = (msg_publish*)message_buffer;

    msg->length = sizeof(message_header) + sizeof(msg_publish) + data_len;
    msg->type = PUBLISH;
    msg->flags = flags;
    msg->topic_id = bswap(topic_id);
    msg->message_id = bswap(message_id);
    memcpy(msg->data, data, data_len);

    send_message();

    if ((flags & QOS_MASK) == FLAG_QOS_0 || (flags & QOS_MASK) == FLAG_QOS_M1) {
        // Cheesy hack to ensure two messages don't run-on into one send.
        delay(100);
//        ctx->state = SLEEPING;
    }
}

void MQTTSN::pingreq(const char* client_id) {
    waiting_for_response = true;

    msg_pingreq* msg = (msg_pingreq*)message_buffer;
    msg->length = sizeof(message_header) + strlen(client_id);
    msg->type = PINGREQ;
    strcpy(msg->client_id, client_id);

    send_message();
}

uint16_t MQTTSN::find_topic_id(const char* name) {
    for (uint8_t i = 0; i < topic_count; ++i) {
        if (strcmp(topic_table[i].name, name) == 0) {
            return topic_table[i].id;
        }
    }

    return 0xffff;
}