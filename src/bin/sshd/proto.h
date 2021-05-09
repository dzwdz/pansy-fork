#include "conn.h"

extern const char *SERVER_ID; // main.c

void id_exchange(connection *conn);
void algo_negotiation(connection *conn);
void key_exchange(connection *conn);
