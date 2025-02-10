#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "websocket_client.h"

static struct lws *websocket = NULL;
static struct lws_context *context = NULL;
static int connected = 0;

#define SERVER_ADDRESS "localhost"
#define SERVER_PORT 8080
#define PROTOCOL_NAME "ws"

// WebSocket callback function
static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("Connected to WebSocket server\n");
            connected = 1;
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            // Do nothing here, sendSocket() handles sending
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Disconnected from WebSocket server\n");
            connected = 0;
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    { PROTOCOL_NAME, callback_websocket, 0, 256 },
    { NULL, NULL, 0, 0 }
};

void initWebSocket() {
    struct lws_client_connect_info connect_info;
    memset(&connect_info, 0, sizeof(connect_info));

    struct lws_context_creation_info context_info;
    memset(&context_info, 0, sizeof(context_info));

    context_info.port = CONTEXT_PORT_NO_LISTEN;
    context_info.protocols = protocols;

    context = lws_create_context(&context_info);
    if (!context) {
        fprintf(stderr, "Failed to create WebSocket context\n");
        exit(1);
    }

    connect_info.context = context;
    connect_info.address = SERVER_ADDRESS;
    connect_info.port = SERVER_PORT;
    connect_info.path = "/";
    connect_info.host = connect_info.address;
    connect_info.origin = connect_info.address;
    connect_info.protocol = PROTOCOL_NAME;
    connect_info.ssl_connection = 0;
    context_info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
context_info.timeout_secs = 10000;  // Extend timeout

    websocket = lws_client_connect_via_info(&connect_info);
    if (!websocket) {
        fprintf(stderr, "WebSocket connection failed\n");
        lws_context_destroy(context);
        exit(1);
    }

    printf("Attempting to connect to ws://%s:%d\n", SERVER_ADDRESS, SERVER_PORT);

    // Run event loop to establish connection
    while (!connected) {
        lws_service(context, 100);
    }
}

void sendSocket(int x, int y) {
    if (!connected) {
        printf("WebSocket not connected. Cannot send data.\n");
        return;
    }

    char message[256];
    snprintf(message, sizeof(message), "{\"x\": %d, \"y\": %d}", x, y);

    unsigned char buf[LWS_PRE + 256];
    memset(buf, 0, sizeof(buf));
    memcpy(&buf[LWS_PRE], message, strlen(message));

    lws_write(websocket, &buf[LWS_PRE], strlen(message), LWS_WRITE_TEXT);
    printf("Sent: %s\n", message);
    
    lws_callback_on_writable(websocket);  // Ensure WebSocket stays active
}

void cleanupWebSocket() {
    if (context) {
        lws_context_destroy(context);
        printf("WebSocket connection closed.\n");
    }
}
