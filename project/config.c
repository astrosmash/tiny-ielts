char* read_file(const char* filename)
{
    fprintf(stdout, "read_file(%s)\n", filename);
    const char* mode = "r";
    FILE* file = NULL;

    if ((file = fopen(filename, mode)) == NULL) {
        fprintf(stderr, "fopen(%s) no file\n", filename);
        return NULL;
    }

    size_t buf_size = 1024; // FIXME: will overflow on large file
    char* buf = (char*)malloc(buf_size);
    if (buf == NULL) {
        fprintf(stderr, "read_file malloc error\n");
        fclose(file);
        return NULL;
    }

    memset(buf, 0, buf_size);

    size_t ret = fread(buf, sizeof(char), buf_size, file);
    fclose(file);
    fprintf(stdout, "fread(%s) %zu bytes\n", filename, ret);
    return buf;
}

config_t* allocate_config(config_t* config)
{
    config = malloc(sizeof(config_t));
    config->port = (char*)malloc(4 + 1);
    config->address = (char*)malloc(14 + 1);
    config->css = (char*)malloc(63 + 1);
    config->html = (char*)malloc(63 + 1);

    if (config->port == NULL || config->address == NULL || config->css == NULL || config->html == NULL) {
        fprintf(stderr, "allocate_config malloc error\n");
        free(config);
        return NULL;
    }
    memset(config->port, 0, 4 + 1);
    memset(config->address, 0, 14 + 1);
    memset(config->css, 0, 63 + 1);
    memset(config->html, 0, 63 + 1);
    return config;
}

void free_config(config_t* config)
{
    free(config->address);
    free(config->port);
    free(config->css);
    free(config->html);
    free(config);
    fprintf(stderr, "free_config done\n");
}

config_t* read_config(const char* filename)
{
    fprintf(stdout, "read_config(%s)\n", filename);
    char* file = NULL;

    if ((file = read_file(filename)) == NULL) {
        fprintf(stderr, "read_file(%s) error\n", filename);
        return NULL;
    }

    config_t* config = NULL;
    if ((config = allocate_config(config)) == NULL) {
        return NULL;
    }

    char* strtok_saveptr = NULL;
    char* line = strtok_r(file, "\n", &strtok_saveptr);
    while (line != NULL) {
        if (sscanf(line, "address = %14s\n", config->address) == 1) {
            fprintf(stdout, "scanned address %s\n", config->address);
        }
        if (sscanf(line, "port = %4s\n", config->port) == 1) {
            fprintf(stdout, "scanned port %s\n", config->port);
        }
        if (sscanf(line, "css = %63s\n", config->css) == 1) {
            fprintf(stdout, "scanned css %s\n", config->css);
        }
        if (sscanf(line, "html = %63s\n", config->html) == 1) {
            fprintf(stdout, "scanned html %s\n", config->html);
        }
        line = strtok_r(NULL, "\n", &strtok_saveptr);
    }

    fprintf(stdout, "read_config address: %s port: %s css: %s html: %s\n",
        config->address, config->port, config->css, config->html);
    free(file);
    return config;
}
