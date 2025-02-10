#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

void initWebSocket();           // Initialize and connect WebSocket
void sendSocket(int id, float x, float y);  // Send data
void cleanupWebSocket();        // Close WebSocket

#ifdef __cplusplus
}
#endif

#endif
