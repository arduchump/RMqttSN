/*
mqttsn-messages.h
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

#ifndef __MQTTSN_MESSAGES_H__
#define __MQTTSN_MESSAGES_H__

#include "mqttsn.h"

#define MAX_TOPICS 10
#define MAX_BUFFER_SIZE 66

class MQTTSN {
public:
    MQTTSN();
    virtual ~MQTTSN();

    uint16_t find_topic_id(const char* name);
    bool wait_for_response();
    void parse_stream();

    void connect(const uint8_t flags, const uint16_t duration, const char* client_id);
    void pingreq(const char* client_id);
    void disconnect(const uint16_t duration);
    bool register_topic(const char* name);
    void publish(const uint8_t flags, const uint16_t topic_id, const uint16_t message_id, const void* data, const uint8_t data_len);

protected:
    virtual bool connack_handler(msg_connack* msg);
    virtual void disconnect_handler(msg_disconnect* msg);
    virtual bool regack_handler(msg_regack* msg);
    virtual void pingresp_handler();
    virtual void puback_handler(msg_puback* msg);

private:
    struct topic {
        const char* name;
        uint16_t hash;
        uint8_t id;
    };

    void dispatch();
    uint16_t hash(const char* str);
    uint16_t hash_P(const char* str);
    uint16_t bswap(const uint16_t val);
    void send_message();

    // Set to true when we're waiting for some sort of acknowledgement from the server that will transition our state.
    bool waiting_for_response;
    uint16_t reg_msg_id;
    uint8_t topic_count;

    uint8_t message_buffer[MAX_BUFFER_SIZE];
    uint8_t response_buffer[MAX_BUFFER_SIZE];
    topic topic_table[MAX_TOPICS];
};

#endif