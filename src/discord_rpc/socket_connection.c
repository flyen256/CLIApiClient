#include "stdio.h"
#include "string.h"
#include "socket_connection.h"

static volatile int keep_running = 1;
static int exit_flag = 0;
static struct lws_context *global_context = NULL;
static client_data_t global_client_data = {0};

static void queue_init(message_queue_t *q) {
    q->front = q->rear = q->count = 0;
    for (int i = 0; i < MAX_QUEUE_SIZE; i++) {
        q->messages[i] = NULL;
    }
}

static int queue_push(message_queue_t *q, const char *message) {
    if (q->count >= MAX_QUEUE_SIZE) {
        return 0;
    } 
    q->messages[q->rear] = strdup(message);
    if (!q->messages[q->rear]) {
        return 0;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->count++;
    return 1;
}

static char *queue_pop(message_queue_t *q) {
    if (q->count <= 0) {
        return NULL;
    } 
    char *msg = q->messages[q->front];
    q->messages[q->front] = NULL;
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->count--;
    return msg;
}

static void queue_cleanup(message_queue_t *q) {
    while (q->count > 0) {
        char *msg = queue_pop(q);
        if (msg) free(msg);
    }
}

static int callback_client(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
	client_data_t *client_data = (client_data_t *)user;
	switch (reason) {
		default:
			break;
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			client_data->connected = 1;
			client_data->wsi = wsi;
			puts("Socket connection established");
			lws_callback_on_writable(wsi);
			break;
		case LWS_CALLBACK_CLIENT_WRITEABLE:
			puts("Socket client writeable");
			if (client_data->connected) {
				pthread_mutex_lock(&client_data->queue_mutex);
				while (client_data->queue.count > 0) {
					char *message = queue_pop(&client_data->queue);
					if (message) {
						size_t message_len = strlen(message);
						unsigned char *buf = malloc(LWS_PRE + message_len);
						if (buf) {
							memcpy(buf + LWS_PRE, message, message_len);
							int ret = lws_write(wsi, buf + LWS_PRE, message_len, LWS_WRITE_TEXT);
							if (ret < 0) {
								printf("Failed to send message: %s\n", message);
							}
							else {
								printf("Sent: %s\n", message);
							}
							free(buf);
						}
						free(message);
					}
				}
				pthread_mutex_unlock(&client_data->queue_mutex);
			}
			break;
		case LWS_CALLBACK_CLIENT_CLOSED:
			client_data->connected = 0;
			puts("Socket connection closed.");
			break;
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			client_data->connected = 0;
			puts("Socket connection error.");
			break;
		case LWS_CALLBACK_CLIENT_RECEIVE:
			printf("Socket received: %.*s\n", (int)len, (char*)in);
			break;
	}
	return 0;
}

static struct lws_protocols protocols[] = {
	{ "ws", callback_client, sizeof(client_data_t), 0 },
	{ NULL, NULL, 0, 0 }
};

int init_lws_client(char *address, int port, int portRange, char *path) {
	printf("INIT LWS CLIENT START: address: %s, port: %d, portRange: %d\n", address, port, portRange);
	struct lws_context_creation_info info;
	queue_init(&global_client_data.queue);
	pthread_mutex_init(&global_client_data.queue_mutex, NULL);
	memset(&info, 0, sizeof(info));
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	global_context = lws_create_context(&info);
	if (!global_context) {
		puts("Global context not created");
		return 0;
	}
	struct lws_client_connect_info connect_info = {0};
	connect_info.context = global_context;
	connect_info.address = address;
	connect_info.port = port;
	connect_info.path = path ? path : "/";
	connect_info.host = connect_info.address;
	connect_info.origin = connect_info.address;
	connect_info.protocol = "ws";
	connect_info.userdata = &global_client_data;
	int connected = 0;
	if (portRange > 0) {
		for (int i = 0; i < portRange; i++) {
			printf("port tried: %d\n", connect_info.port);
			if (lws_client_connect_via_info(&connect_info)) {
				connected = 1;
				break;
			}
			connect_info.port = connect_info.port++;
		}
	}
	else if (lws_client_connect_via_info(&connect_info))
		connected = 1;
	if (!connected)
	{
		puts("LWS Connection error");
		lws_context_destroy(global_context);
		global_context = NULL;
		return 0;
	}
	return 1;
}

int run_lws_client() {
	if (!global_context) return 0;
	keep_running = 1;
	int ret = 0;
	while (ret >= 0 && keep_running) {
		ret = lws_service(global_context, 100);
		static int counter = 0;
		if (++counter >= 10) {
			if (global_client_data.connected && global_client_data.queue.count > 0) {
				lws_callback_on_writable(global_client_data.wsi);
			}
			counter = 0;
		}
	}
	printf("LWS Root loop stopped");
	return 1;
}

void stop_lws_client() {
	keep_running = 0;
	printf("Keep running: %d", keep_running);
}

int lws_send_message(const char *message) {
	if (!global_client_data.connected) return 0;
	if (strlen(message) > MAX_MESSAGE_SIZE) return 0;
	pthread_mutex_lock(&global_client_data.queue_mutex);
	int result = queue_push(&global_client_data.queue, message);
	pthread_mutex_unlock(&global_client_data.queue_mutex);
	if (result) {
		if (global_client_data.wsi) {
			lws_callback_on_writable(global_client_data.wsi);
		}
	}
	return result;
}

int lws_is_connected() {
	return global_client_data.connected;
}
