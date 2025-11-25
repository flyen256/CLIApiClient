#include "libwebsockets.h"

#define MAX_QUEUE_SIZE 100
#define MAX_MESSAGE_SIZE 4096

typedef struct {
	char *messages[MAX_QUEUE_SIZE];
	int front;
	int rear;
	int count;
} message_queue_t;

struct client_data {
	int connected;
	struct lws *wsi;
	message_queue_t queue;
	pthread_mutex_t queue_mutex;
};

typedef struct client_data client_data_t;

int init_lws_client(char *address, int port, int portRange, char *path);
int lws_is_connected();
int lws_send_message(const char *message);
int run_lws_client();
void stop_lws_client();
