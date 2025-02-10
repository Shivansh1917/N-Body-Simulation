#include "websocket_client.h"
#include <unistd.h>
#include <stdbool.h>
int main() {
    initWebSocket();

    // Send 10 messages using sendSocket
    int i=0;
    while(i<5){
        sendSocket(i, i * 2);
        i++;
        usleep(1);
    }

    cleanupWebSocket();
    return 0;
}

// gcc main.c websocket_client.c -o client -lwebsockets
