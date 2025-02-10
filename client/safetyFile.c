#include "websocket_client.h"
#include <unistd.h>
#include <stdbool.h>
int main() {
    initWebSocket();

    // Send 10 messages using sendSocket
    usleep(5000);
    int i=0; int j=0;
    while(i<5){
        sendSocket(i, i, i * j*0.001+j*0.001);
        i++;
        usleep(1);
        if(i==5){
            i=0;
        }
        j++;
    }

    cleanupWebSocket();
    return 0;
}

// gcc main.c websocket_client.c -o client -lwebsockets
