#include "stdio.h"
#include "http_client.h"
#include "stdbool.h"

bool isRunning;

void clear_screen() {
    printf("\033[2J\033[H");
}

void clear_line() {
    printf("\033[2K\r");
}

int main() {
    isRunning = true;
    while (isRunning) {
        clear_screen();
		printf("Выберите действие\n1. GET запрос\n2. POST запрос\n3. Выйти\n");
        
        int action;
        
        Memory response;
        switch (action) {
            case 1:
                clear_screen();
                response = http_get("https://httpbin.org/get");
                printf("GET Response:\n%s\n", response.memory);
                printf("Нажмите Enter для продолжения...");
                getchar();
                free_memory(&response);
                break;
                
            case 2:
                clear_screen();
                response = http_post_json("https://httpbin.org/post", "{\"name\": \"Test\"}");
                printf("POST Response:\n%s\n", response.memory);
                printf("Нажмите Enter для продолжения...");
                getchar();
                free_memory(&response);
                break;
                
            case 3:
                isRunning = false;
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
