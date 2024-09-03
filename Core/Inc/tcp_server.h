#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#include "lwip/sockets.h"

void tcpReceiveTask(void *pvParameters);

#endif // __TCP_SERVER_H
