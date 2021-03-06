#include "life.h"
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <png.h>

struct Game *Game_Init() {
    int cellcount = WIDTH * HEIGHT;
    struct Game *game = malloc(sizeof(struct Game));
    game->generation = 0;
    game->len = sizeof(bool) * cellcount;
    game->grid = malloc(game->len);
    game->past_grid = malloc(game->len);

    for (int i = 0; i < cellcount; i++) {
        game->grid[i] = false;
        game->past_grid[i] = false;
    }

    return game;
}

void Game_Random(struct Game *game) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        game->grid[i] = (rand() % 2) ? false : true;
    }
}

void Game_Render(struct Game *game) {

    static char fname[20];
    static png_byte *buffer = NULL;
    static png_image *imagep = NULL;
    int buffer_size = WIDTH * HEIGHT * SCALE * SCALE * sizeof(png_byte);

    // allocate the buffer once
    if (buffer == NULL) {
        buffer = malloc(buffer_size);
        if (buffer == NULL) {
            fprintf(stderr, "Error: malloc failed\n");
            exit(EXIT_FAILURE);
        }
    }
    memset(buffer, 0, buffer_size);

    if (imagep == NULL) {
        imagep = malloc(sizeof(png_image));
        if (imagep == NULL) {
            fprintf(stderr, "Error: malloc failed\n");
            exit(EXIT_FAILURE);
        }
    }

    memset(imagep, 0, sizeof(png_image));
    imagep->version = PNG_IMAGE_VERSION;
    imagep->opaque = NULL;
    imagep->width = WIDTH * SCALE;
    imagep->height = HEIGHT * SCALE;
    imagep->format = PNG_FORMAT_GRAY;
    imagep->flags = 0;
    imagep->colormap_entries = 0;

    sprintf(fname, "life_%05lu.png", game->generation);
    for (int h = 0; h < HEIGHT; h++) {
        for (int sh = 0; sh < SCALE; sh++)  //scale for height
            for (int w = 0; w < WIDTH; w++) {
                for (int s = 0; s < SCALE; s++) {
                    int index = w + h * WIDTH;
                    int buffer_index = (w * SCALE) + s + (((h * SCALE) + sh) * WIDTH * SCALE); // h * scale * game->w + sh * game->w
                    buffer[buffer_index] = (game->grid[index]) ? 0 : 255;
                }
            }
    }

    png_image_write_to_file(imagep, fname, 0, buffer, 0, 0);
}


/* check the rules to see whether the cell at (x, y)
 * should be alive next generation or not */
bool check_rules(bool *grid, int x, int y) {
    int c = 0; /* number of neighbours */
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) /* we don't wanna count the cell itself */
                continue;
            int index = (x + i) + (WIDTH * (y + j));
            if (index < 0 || index > (HEIGHT * WIDTH - 1))
                continue;
            if (grid[index])
                c++;
        }
    }

    if (grid[x + WIDTH * y]) { // cell is alive
        if (c == 2 || c == 3) {
            return true;
        } else {
            return false;
        }
    } else { // cell is dead
        if (c == 3) {
            return true;
        } else {
            return false;
        }
    }
}


void Game_Tick(struct Game *game) {

    game->generation++;
    memcpy(game->past_grid, game->grid, game->len);

    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            game->grid[i + WIDTH * j] = check_rules(game->past_grid, i, j);
        }
    }
}


int main(int argc, char *argv[]) {

    int gen_from;
    int gen_to;
    struct Game *game;

    game = Game_Init();

    if (argc != 4 && argc != 3) {
        printf(
            "Usage: %s [init_file] gen_from gen_to\n"
            "    init_file:    The file containing generation 0,\n"
            "                  or omit it to use a random initial state\n"
            "    gen_from:     Produce output from this generation onward\n"
            "    gen_to:       Stop generating output after this generation\n",
            argv[0]);
        return 1;
    }

    if (argc == 3) {
        Game_Random(game);
        gen_from = strtol(argv[1], NULL, 10);
        gen_to = strtol(argv[2], NULL, 10);
    }

    for (int i = gen_from; i <= gen_to; i++) {
        Game_Render(game);
        Game_Tick(game);
    }

    return 0;
}
