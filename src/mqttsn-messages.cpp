/*
mqttsn-messages.cpp

The MIT License (MIT)

Copyright (C) 2014 John Donovan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <Arduino.h>

#include "mqttsn-messages.h"
#include "mqttsn.h"

#ifdef USE_RF12
#include <JeeLib.h>
#endif

MQTTSN::MQTTSN(const bool rf12) :
waiting_for_response(false),
reg_msg_id(0),
topic_count(0),
_rf12(rf12),
_gateway_id(0)
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

uint16_t MQTTSN::find_topic_id(const char* name) {
    for (uint8_t i = 0; i < topic_count; ++i) {
        if (strcmp(topic_table[i].name, name) == 0) {
            return topic_table[i].id;
        }
    }

    return 0xffff;
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

#ifdef USE_RF12
void MQTTSN::parse_rf12() {
    memcpy(response_buffer, (const void*)rf12_data, RF12_MAXDATA < MAX_BUFFER_SIZE ? RF12_MAXDATA : MAX_BUFFER_SIZE);
    dispatch();
    waiting_for_response = false;
}
#endif

void MQTTSN::dispatch() {
    message_header* response_message = (message_header*)response_buffer;

    switch (response_message->type) {
    case ADVERTISE:
        advertise_handler((msg_advertise*)response_buffer);
        break;

    case GWINFO:
        gwinfo_handler((msg_gwinfo*)response_buffer);
        break;

    case CONNACK:
        connack_handler((msg_connack*)response_buffer);
        break;

    case WILLTOPICREQ:
        willtopicreq_handler(response_message);
        break;

    case WILLMSGREQ:
        willmsgreq_handler(response_message);
        break;

    case REGACK:
        regack_handler((msg_regack*)response_buffer);
        break;

    case PUBLISH:
        publish_handler((msg_publish*)response_buffer);
        break;

    case PUBACK:
        puback_handler((msg_puback*)response_buffer);
        break;

    case SUBACK:
        suback_handler((msg_suback*)response_buffer);
        break;

    case UNSUBACK:
        unsuback_handler((msg_unsuback*)response_buffer);
        break;

    case PINGREQ:
        pingreq_handler((msg_pingreq*)response_buffer);
        break;

    case PINGRESP:
        pingresp_handler();
        break;

    case DISCONNECT:
        disconnect_handler((msg_disconnect*)response_buffer);
        break;

    case WILLTOPICRESP:
        willtopicresp_handler((msg_willtopicresp*)response_buffer);
        break;

    case WILLMSGRESP:
        willmsgresp_handler((msg_willmsgresp*)response_buffer);
        break;

    default:
        return;
    }
}

void MQTTSN::send_message() {
    message_header* hdr = (message_header*)message_buffer;

#ifdef USE_RF12
    if (_rf12) {
        while (!rf12_canSend()) {
            rf12_recvDone();
            Sleepy::loseSomeTime(32);
        }
        rf12_sendStart(_gateway_id, message_buffer, hdr->length);
        rf12_sendWait(2);
    } else {
        Serial.write(message_buffer, hdr->length);
        Serial.flush();
    }
#else
    Serial.write(message_buffer, hdr->length);
    Serial.flush();
#endif
}

void MQTTSN::advertise_handler(const msg_advertise* msg) {
    _gateway_id = msg->gw_id;
}

bool MQTTSN::connack_handler(const msg_connack* msg) {
    return msg->return_code == 0;
}

bool MQTTSN::regack_handler(const msg_regack* msg) {
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
    if (!waiting_for_response && topic_count < MAX_TOPICS) {
        waiting_for_response = true;
        reg_msg_id = hash(name);

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

    msg_publish* msg = (msg_publish*)message_buffer;

    msg->length = sizeof(message_header) + sizeof(msg_publish) + data_len;
    msg->type = PUBLISH;
    msg->flags = flags;
    msg->topic_id = bswap(topic_id);
    msg->message_id = bswap(message_id);
    memcpy(msg->data, data, data_len);

    send_message();

    if (!waiting_for_response) {
        // Cheesy hack to ensure two messages don't run-on into one send.
        delay(100);
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
