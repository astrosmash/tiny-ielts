#include "main.h"

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

        Gui* main_gui = NULL;
        if (((main_gui = Gui_Init()) == NULL)) {
            fprintf(stderr, "Cannot launch GUI\n");
        }

        fprintf(stdout, "GUI launched\n");

        gtk_main();
    }

    exit(EXIT_SUCCESS);
}
