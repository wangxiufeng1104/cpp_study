#ifndef __PROTOBUS_H
#define __PROTOBUS_H
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct protobus_handle protobus_handle_t;
protobus_handle_t *protobus_init(const char *node_name);
void protobus_cleanup(protobus_handle_t* handle);

#ifdef __cplusplus
}
#endif
#endif