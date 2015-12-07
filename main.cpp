#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* json-c (https://github.com/json-c/json-c) */
#include <curl/curl.h>

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <json-c/json.h>

const char *DEFAULT_URL = "http://127.0.0.1:9081/add";

// CURLRawData is a holder for curl fetch.
class CURLRawData {
public:
    char *payload;
    size_t size;
};

// curl_callback is a callback for curl fetch.
size_t curl_callback(void *contents, size_t size, size_t nmemb,
        void *userp) {
    size_t realsize = size * nmemb;

    CURLRawData *p = (struct CURLRawData *) userp;

    // Expand buffer.
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);
    // Check if buffer was expanded correctly.
    if (p->payload == NULL) {
        fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
        // Do I have to free buffer if I'm exiting?
        // Or is it some different thread exiting?
        // TODO(att): check how it works.
        free(p->payload);
        return -1;
    }

    memcpy(&(p->payload[p->size]), contents, realsize);
    p->size += realsize;
    p->payload[p->size] = 0;
    // TODO(att): Refactor realsize name to something else?
    return realsize;
}


CURLcode curl_fetch_url(CURL *ch, const char *url,
        CURLRawData *fetch) {
    CURLcode rcode;
    // I wonder how much it actually is? Size of initialized char string.
    // TODO(att): check it!
    fetch->payload = (char *) calloc(1, sizeof(fetch->payload));
    if (fetch->payload == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        return CURLE_FAILED_INIT;
    }
    fetch->size = 0;
    curl_easy_setopt(ch, CURLOPT_URL, url);
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);

    rcode = curl_easy_perform(ch);
    return rcode;
}

const char* getURL(int argc, char *argv[]) {
    //TODO(att): refactor into strings instead of char arrays.
    return DEFAULT_URL;
}

class JSONResponse {
    public:
        json_object* getJSON(){
            return obj;
        }
        const char *getStatusCode() {
            return status_code;
        }
        JSONResponse (const char *status_code, json_object *obj) : status_code(status_code), obj(obj) {}
    private:
        const char *status_code;
        json_object *obj;
};

class Client {
public:
    Client() {
        ch = curl_easy_init();
        if (ch == NULL) {
            fprintf(stderr, "ERROR: Failed to create curl handle.");
        }
    }

    JSONResponse* Post(const char *url, json_object *data) {
        CURLcode rcode;
        CURLRawData curl_fetch;
        CURLRawData *cf = &curl_fetch;
        curl_slist *headers = NULL; // headers to send with request

        if ((ch = curl_easy_init()) == NULL) {
            fprintf(stderr, "ERROR: Failed to create curl handle.");
            return NULL;
        }

        curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(ch, CURLOPT_POSTFIELDS, json_object_to_json_string(data));

        rcode = curl_fetch_url(ch, url, cf);
        const char *statusCode = curl_easy_strerror(rcode);
        curl_easy_cleanup(ch);
        curl_slist_free_all(headers);
        // Free json object.
        json_object_put(data);

        if (rcode != CURLE_OK || cf->size < 1) {
            fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s", url, statusCode);
            return NULL;
        }

        if (cf->payload == NULL) {
            fprintf(stderr, "ERROR: Failed to populate payload");
            free(cf->payload);
            return NULL;
        }

        json_object *json;
        json_tokener_error jerr = json_tokener_success;
        json = json_tokener_parse_verbose(cf->payload, &jerr);
        free(cf->payload);

        if (jerr != json_tokener_success) {
            fprintf(stderr, "ERROR: Failed to parse json string");
            json_object_put(json);
            return NULL;
        }
        JSONResponse *response = new JSONResponse(statusCode, json);
        return response;
    }
private:
   CURL *ch;
};

int main(int argc, char *argv[]) {
    json_object *json;
    json = json_object_new_object();
    json_object_object_add(json, "url", json_object_new_string("http://atte.ro/"));
    const char *url = getURL(argc, argv);
    Client *client = new Client();
    JSONResponse *response;
    response = client->Post(url, json);
    if (response != NULL) {
        printf("Parsed JSON: %s\n", json_object_to_json_string(response->getJSON()));
    }
    return 0;
}
