#include "main.h"

static bool track_allocated_blocks(void* ptr, size_t flag)
{
    static void* allocated_blocks[MAX_ALLOCABLE_MEM_BLOCKS] = { NULL };
    static size_t allocated_counter = 0;
    static size_t freed_pos = 0;

    if (!(flag & GLOBAL_CLEANUP)) {
        assert(ptr);
        if (flag & ALLOCATION) { // This is an allocated block. Insert it so we can track it later, if it was freed or not
            if (freed_pos && !allocated_blocks[freed_pos]) { // We see position with NULL that was previously removed, use it
                debug(4, "Will insert %p at cached free pos #%zu\n", ptr, freed_pos);
                allocated_blocks[freed_pos] = ptr;
                return true;
            }

            ++allocated_counter;
            assert(allocated_counter <= MAX_ALLOCABLE_MEM_BLOCKS);

            if (!allocated_blocks[allocated_counter]) {
                allocated_blocks[allocated_counter] = ptr;
            } else {
                debug(4, "Will not insert %p at pos #%zu - found %p there. Will try +1\n", ptr, allocated_counter, allocated_blocks[allocated_counter]);

                ++allocated_counter;
                assert(allocated_counter <= MAX_ALLOCABLE_MEM_BLOCKS);

                if (!allocated_blocks[allocated_counter]) {
                    allocated_blocks[allocated_counter] = ptr;
                } else {
                    debug(4, "+1 failed! Will not insert %p at pos #%zu - found %p there.\n", ptr, allocated_counter, allocated_blocks[allocated_counter]);
                    return false;
                }
            }

            debug(4, "Inserted %p at pos #%zu\n", ptr, allocated_counter);
            return true;
        }

        if (flag & REMOVAL) { // this is free func, let's show what we have so it compares
            if (freed_pos && allocated_blocks[freed_pos] == ptr) {
                debug(5, "Looking for %p, found %p at cached pos #%zu\n", ptr, allocated_blocks[freed_pos], freed_pos);
                allocated_blocks[freed_pos] = NULL;
                return true;
            }

            for (size_t i = allocated_counter; i > 0; --i) {
                debug(5, "Looking for %p, found %p at pos #%zu\n", ptr, allocated_blocks[i], i);
                if (ptr == allocated_blocks[i]) {
                    allocated_blocks[i] = NULL;
                    freed_pos = i;
                    debug(4, "Removed %p at pos #%zu\n", ptr, i);
                    --allocated_counter;
                    debug(4, "Now allocated: %zu\n", allocated_counter);
                    return true;
                }
            }
            debug(5, "REMOVAL was set, cannot find %p\n", ptr);
            return false;
        }
    } else {
        for (size_t i = allocated_counter; i > 0; --i) {
            if (allocated_blocks[i]) {
                debug(5, "Removing leftover %p at pos #%zu\n", allocated_blocks[i], i);
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
        debug(5, "Safe allocating %p of size %zu", block, size);
        return block;
    }
    return NULL;
}

void safe_free(void** ptr)
{
    assert(*ptr);
    debug(5, "Safe freeing %p", *ptr);
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
