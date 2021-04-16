#include "main.h"

void* malloc_memset(size_t size)
{
    void* block = malloc(size);
    assert(block);

    memset(block, 0, size);

    debug(3, "Allocating clear object with size %zu on %p", size, block);
    return block;
}

int main(int argc, char** argv)
{
    ssize_t opt = 0;
    size_t gui = 0;

    while ((opt = getopt(argc, argv, "g")) != -1) {
        switch (opt) {
        case 'g':
            gui = 1;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-g]\n", *argv);
            exit(EXIT_FAILURE);
        }
    }

    if (gui) {
        gtk_init(&argc, &argv);
        curl_global_init(CURL_GLOBAL_DEFAULT);

        Gui* main_gui = NULL;
        if (((main_gui = Gui_Init()) == NULL)) {
            fprintf(stderr, "Cannot launch GUI\n");
        }

        fprintf(stdout, "GUI launched\n");

        gtk_main();
    }

    curl_global_cleanup();
    exit(EXIT_SUCCESS);
}
