#include "main.h"

bool track_allocated_blocks(void* ptr, size_t flag)
{
    static void* allocated_blocks[MAX_ALLOCABLE_MEM_BLOCKS] = { NULL };
    static size_t allocated_counter = 0;

    if (!(flag & GLOBAL_CLEANUP)) {
        assert(ptr);
        if (flag & ALLOCATION) { // This is an allocated block. Insert it so we can track it later, if it was freed or not
            assert(allocated_counter <= MAX_ALLOCABLE_MEM_BLOCKS);
            allocated_blocks[allocated_counter] = ptr;
            debug(4, "Inserted %p at pos #%zu\n", ptr, allocated_counter);
            ++allocated_counter;
            return true;

        } else if (flag & REMOVAL) { // this is free func, let's show what we have so it compares
            for (size_t i = allocated_counter; i > 0; --i) {
                debug(4, "Looking for %p, found %p at pos #%zu\n", ptr, allocated_blocks[i], i);
                if (ptr == allocated_blocks[i]) {
                    allocated_blocks[i] = NULL;
                    debug(4, "Removed %p at pos #%zu\n", ptr, allocated_counter);
                    --allocated_counter;
                    return true;
                }
            }
        }
    } else {
        for (size_t i = allocated_counter; i > 0; --i) {
            if (allocated_blocks[i]) {
                debug(4, "Removing leftover %p at pos #%zu\n", allocated_blocks[i], i);
                free(allocated_blocks[i]);
                allocated_blocks[i] = NULL;
                --allocated_counter;
            }
        }
        debug(4, "%zu blocks did not require cycle\n", allocated_counter);
        return true;
    }
    debug(4, "%zu blocks MISSED in the pool...\n", allocated_counter);
    return false;
}

void* malloc_memset(size_t size)
{
    void* block = malloc(size);
    assert(block);

    memset(block, 0, size);
    if (track_allocated_blocks(block, ALLOCATION)) {
        debug(3, "Safe allocating %p of size %zu", block, size);
        return block;
    }
    return NULL;
}

void safe_free(void** ptr)
{
    assert(*ptr);
    debug(3, "Safe freeing %p", *ptr);
    track_allocated_blocks(*ptr, REMOVAL);
    free(*ptr);
    *ptr = NULL;
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
        curl_global_init(CURL_GLOBAL_DEFAULT);
        gtk_init(&argc, &argv);

        Gui* main_gui = NULL;
        if (((main_gui = Gui_Init()) == NULL)) {
            fprintf(stderr, "Cannot launch GUI\n");
        }

        fprintf(stdout, "GUI launched\n");

        gtk_main();
        curl_global_cleanup();
    }

    // Clean leftover blocks
    if (track_allocated_blocks(NULL, GLOBAL_CLEANUP)) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}
