typedef enum {
    false,
    true
} bool;

struct curl_string {
    char* ptr;
    size_t len;
};
