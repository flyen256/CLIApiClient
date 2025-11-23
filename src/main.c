#include "stdio.h"
#include "http_client.h"

int main() {
	Memory response = http_get("https://httpbin.org/get");
	printf("%s", response.memory);
	free_memory(&response);
	Memory response2 = http_post_json("https://httpbin.org/post", "{\"name\": \"Test\"}");
	printf("%s", response2.memory);
	free_memory(&response2);
	return 0;
}
