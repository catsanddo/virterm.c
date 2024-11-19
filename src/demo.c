#include <stdio.h>
#include <assert.h>
#define VIRTERM_IMPLEMENTATION
#include <virterm.h>

void rainbow_print(const char *s) {
    static unsigned int random = 1;
    while (*s != 0) {
        random ^= random << 17;
        random ^= random >> 11;
        random ^= random << 7;
        int color = random % 6 + 1;
        assert(1 <= color && color <= 6);
        vt_set_color(color, BLACK);
        vt_emit(*s++);
    }
}

void color_demo(void)
{
    vt_clear();
    vt_move(1, 1);
    int width = 32;
    for (int i = 0; i < 256; ++i) {
        if (i != 0 && i % width == 0) {
            vt_reset_mode();
            printf("\n");
        }
        vt_set_color(BLACK, i);
        printf(" ");
    }

    vt_reset_mode();
    printf("\n\n");

    width = 80;
    char *band = "Red Band";
    int length = strlen(band);
    vt_set_color(BRIGHT_WHITE, BLACK);
    for (int i = 0; i < width; ++i) {
        vt_set_tru_bg(256 / width * i, 0, 0);
        if (i < length) {
            printf("%c", band[i]);
        } else {
            printf(" ");
        }
    }
    vt_reset_mode();
    printf("\n");
    band = "Green Band";
    length = strlen(band);
    vt_set_color(BRIGHT_WHITE, BLACK);
    for (int i = 0; i < width; ++i) {
        vt_set_tru_bg(0, 256 / width * i, 0);
        if (i < length) {
            printf("%c", band[i]);
        } else {
            printf(" ");
        }
    }
    vt_reset_mode();
    printf("\n");
    band = "Blue Band";
    length = strlen(band);
    vt_set_color(BRIGHT_WHITE, BLACK);
    for (int i = 0; i < width; ++i) {
        vt_set_tru_bg(0, 0, 256 / width * i);
        if (i < length) {
            printf("%c", band[i]);
        } else {
            printf(" ");
        }
    }
    vt_reset_mode();
    printf("\n");

    vt_key();
}

void rainbow_demo(void)
{
    vt_clear();
    vt_move(1, 1);

    rainbow_print("The quick brown fox jumped over the lazy dog.\n");
    rainbow_print("All the kings horses and all the kings men.\n");
    rainbow_print("your mother\n");

    vt_reset_mode();
    vt_key();
}

void keys_demo(void)
{
    int old_delay = vt_delay(5);
    vt_reset_mode();
    vt_clear();
    vt_move(1, 1);

    int row = 0;
    for (;;) {
        if (row > 10) {
            row = 0;
            vt_clear();
            vt_move(1, 1);
        }
        char key_press = vt_key();
        printf("%c : %#X\n", key_press, key_press);

        row += 1;
    }

    vt_delay(old_delay);
}

int main(void)
{
    if (!vt_init()) {
        printf("Couldn't init.\n");
        return 1;
    }
    vt_rare();

    int running = 1;
    int option = 0;
    char *menu[] = {
        "Color demo",
        "Rainbow demo",
        "Keys demo",
        "Quit",
    };
    int menu_items = 4;

    while (running) {
        vt_reset_mode();
        vt_clear();
        vt_move(1, 1);

        for (int i = 0; i < menu_items; ++i) {
            vt_set_color(BRIGHT_WHITE, BLACK);
            if (option == i) {
                vt_set_color(BLACK, BRIGHT_WHITE);
            }

            printf(" %s \n", menu[i]);
            vt_reset_mode();
        }

        char input = vt_key();
        if (input == 'j') {
            option += 1;
        } else if (input == 'k') {
            option -= 1;
        } else if (input == VT_RETURN) {
            switch (option) {
                case 0: color_demo(); break;
                case 1: rainbow_demo(); break;
                case 2: keys_demo(); break;
                case 3: running = 0; break;
            }
        }

        if (option < 0) {
            option = 0;
        } else if (option >= menu_items) {
            option = menu_items - 1;
        }
    }

    vt_deinit();

    return 0;
}
