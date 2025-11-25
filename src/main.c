#include "stdio.h"
#include "stdlib.h"
#include "http_client.h"
#include "stdbool.h"
#include "discord_rpc/socket_connection.h"

static bool isRunning;

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void clear_screen() {
    printf("\033[2J\033[H");
}

void clear_line() {
    printf("\033[2K\r");
}

void *run_lws_client_t(void *arg) {
	run_lws_client();
	return NULL;
}

void load_env(const char *file_name) {
	FILE *file = fopen(file_name, "r");
	if (!file)
		return;
	char line[256];
	while(fgets(line, sizeof(line), file) != NULL) {
		line[strcspn(line, "\n")] = 0;

		char *key = strtok(line, "=");
		char *value = strtok(NULL, "=");

		if (key != NULL && value != NULL)
			setenv(key, value, 1);
	}
	fclose(file);
}

int main() {
	load_env(".env");
	char *client_id = getenv("CLIENT_ID");
	printf("CLIENT ID: %s\n", client_id);
	pthread_t socket_thread;
	char *path = malloc(256);
	sprintf(path, "/?v=1&client_id=%s", client_id);
	int init_lws = init_lws_client("127.0.0.1", 6463, 9, path);
	printf("init lws %d\n", init_lws);
	if (!init_lws)
		printf("LWS Initialization error\n");
	pthread_create(&socket_thread, NULL, run_lws_client_t, NULL);
    isRunning = true;
    while (isRunning) {
        //clear_screen();
		printf("Выберите действие\n1. GET запрос\n2. POST запрос\n3. Выйти\n");
        
        int action;
        if (scanf("%d", &action) != 1) {
            //clear_input_buffer();
            printf("Ошибка ввода! Нажмите Enter для продолжения...");
            getchar();
            continue;
        }
        //clear_input_buffer();
        
        Memory response;
        switch (action) {
            case 1:
                //clear_screen();
                response = http_get("https://httpbin.org/get");
                printf("GET Response:\n%s\n", response.memory);
                printf("Нажмите Enter для продолжения...");
                getchar();
                free_memory(&response);
                break;
                
            case 2:
                //clear_screen();
                response = http_post_json("https://httpbin.org/post", "{\"name\": \"Test\"}");
                printf("POST Response:\n%s\n", response.memory);
                printf("Нажмите Enter для продолжения...");
                getchar();
                free_memory(&response);
                break;
                
            case 3:
                isRunning = false;
				stop_lws_client();
                break;
                
            default:
                printf("Неверный параметр! Нажмите Enter для продолжения...");
                getchar();
                break;
        }
    }
    printf("Программа завершена.\n");
    return 0;
}
