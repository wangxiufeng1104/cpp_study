#ifndef __PROTOBUS_H
#define __PROTOBUS_H
#include "message.pb.h"
#define TCP_SUB "tcp://127.0.0.1:5555"
#define TCP_PUB "tcp://127.0.0.1:5556"
typedef struct protobus_handle protobus_handle_t;
typedef void (*protobus_cb)(const MSG::WrapperMessage &msg);
protobus_handle_t *protobus_init(const char *node_name);
protobus_handle_t *protobus_init(const char *node_name, std::vector<std::string> topics, protobus_cb cb);
void protobus_cleanup(protobus_handle_t *handle);
void protobus_send(protobus_handle_t *handle, const MSG::WrapperMessage &msg);
void protobus_add_subscriber(protobus_handle_t *handle, const char *topic, protobus_cb cb);
void protobus_del_subscriber(protobus_handle_t *handle, const char *topic);
#endif