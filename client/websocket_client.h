#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

void initWebSocket();   // Initialize and connect WebSocket
void sendSocket(int x, int y);  // Send data
void cleanupWebSocket(); // Close WebSocket

#endif
