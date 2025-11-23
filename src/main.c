#include "stdio.h"
#include "http_client.h"

int main() {
	Memory response = http_get("https://httpbin.org/get");
	printf("%s", response.memory);
	free_memory(&response);
	return 0;
}
