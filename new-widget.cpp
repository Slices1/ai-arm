#include "kiss_sdl.h"
#include <stdio.h>

void slider_event(kiss_hscrollbar *hscrollbar, kiss_entry *entry, SDL_Event *e, int *draw) {
    if (kiss_hscrollbar_event(hscrollbar, e, draw)) {
        // Update the text box based on the slider's value
        float value = hscrollbar->fraction * 100.0f; // Assuming the slider value is between 0 and 100
        char text[16];
        snprintf(text, sizeof(text), "%.2f", value);
        strcpy(entry->text, text);
        *draw = 1;
    }
}

void entry_event(kiss_entry *entry, kiss_hscrollbar *hscrollbar, SDL_Event *e, int *draw) {
    if (kiss_entry_event(entry, e, draw)) {
        // Update the slider based on the text input
        float value = atof(entry->text);
        if (value >= 0.0f && value <= 100.0f) {
            hscrollbar->fraction = value / 100.0f; // Assuming the slider value is between 0 and 100
            *draw = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    kiss_array objects;
    kiss_window window;
    kiss_hscrollbar hscrollbar;
    kiss_entry entry;
    SDL_Event e;
    SDL_Renderer *renderer;
    int draw = 1, quit = 0;

    kiss_array_new(&objects);
    renderer = kiss_init("Slider and Input Example", &objects, 640, 480);
    if (!renderer) return 1;

    kiss_window_new(&window, NULL, 1, 0, 0, 640, 480);
    kiss_hscrollbar_new(&hscrollbar, &window, 100, 200, 400);
    kiss_entry_new(&entry, &window, 1, "0.00", 100, 170, 100);

    window.visible = 1;

    while (!quit) {
        SDL_Delay(10);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;

            kiss_window_event(&window, &e, &draw);
            slider_event(&hscrollbar, &entry, &e, &draw);
            entry_event(&entry, &hscrollbar, &e, &draw);
        }

        if (draw) {
            SDL_RenderClear(renderer);
            kiss_window_draw(&window, renderer);
            kiss_hscrollbar_draw(&hscrollbar, renderer);
            kiss_entry_draw(&entry, renderer);
            SDL_RenderPresent(renderer);
            draw = 0;
        }
    }

    kiss_clean(&objects);
    return 0;
}
