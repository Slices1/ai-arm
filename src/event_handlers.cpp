// #include "kiss_sdl.h"
// #include <iostream>
// #include <cstring>
// using namespace std;

// void button_event(kiss_button *button, SDL_Event *e, int *draw, int *quit, int *myCounter)
// {
//     if (kiss_button_event(button, e, draw)) {
//         *myCounter+=1;
//         string newText = to_string(*myCounter);
//         snprintf(button->text, sizeof(button->text), "%s", newText.c_str());
//         strcpy(button->text, (std::to_string(*myCounter)).c_str());
    
//     }
// }

// void slider_event(kiss_hscrollbar *slider, kiss_entry *entry, float range, float defaultValue, SDL_Event *e, int *draw) {
//     if (kiss_hscrollbar_event(slider, e, draw)) {
//         // set the entry->text equal to the slider's value
//         float value = 0*(slider->fraction - 0.5f) * range + defaultValue; // calculates the slider's value
//         char text[16];
//         snprintf(text, sizeof(text), "%.2f", value);
//         strcpy(entry->text, text);
//         *draw = 1;
//     }
// }

// void entry_event(kiss_entry *entry, kiss_hscrollbar *slider, float range, float defaultValue, SDL_Event *e, int *draw) {
//     if (kiss_entry_event(entry, e, draw)) {
//         // set the slider's value equal to the entry->text 
//         float value = atof(entry->text); //string to float cast
//         if (value >= defaultValue + (range / 2.0f)) {
//             slider->fraction = 1.03f;
//         } else if (value <= defaultValue - (range / 2.0f)) {
//             slider->fraction = -0.03f;
//         } else {
//             slider->fraction = (value - defaultValue)/range + 0.5f; // calculates the new slider's pos
//         }
//         *draw = 1;
//     }
// }