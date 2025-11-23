#include "curl/curl.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "cjson/cJSON.h"
#include "http_client.h"

void free_memory(Memory *mem) {
	if (mem->memory)
		free(mem->memory);
}

CURL *http_client_init() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *curl = curl_easy_init();
	if (curl)
		return curl;
	return NULL;
}

void http_client_cleanup(CURL *curl) {
	if (curl)
		curl_easy_cleanup(curl);
	curl_global_cleanup();
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t total_size = size * nmemb;
	Memory *mem = (Memory*)userp;
	mem->memory = realloc(mem->memory, mem->size + total_size + 1);
	if (mem->memory == NULL)
	{
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), contents, total_size);
	mem->size += total_size;
	mem->memory[mem->size] = 0;
	return total_size;
}
 
Memory http_get(const char *url) {
	CURL *curl = http_client_init();
	CURLcode resCode;
	Memory mem;
	mem.memory = malloc(1);
	mem.size = 0;
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
		resCode = curl_easy_perform(curl);
		if (resCode != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(resCode));
		}
	}
	http_client_cleanup(curl);
	return mem;
}

Memory http_post_json(const char *url, const char *json) {
	CURL *curl = http_client_init();
	CURLcode resCode;
	Memory mem;
	mem.memory = malloc(1);
	mem.size = 0;
	if (curl) {
		struct curl_slist *headers = NULL;
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json));
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		resCode = curl_easy_perform(curl);
		if (resCode != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(resCode));
		}
	}
	http_client_cleanup(curl);
	return mem;
}
