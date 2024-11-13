#include "kiss_sdl.h"
#include "event_handlers.h"
#include <iostream>
//#include <cstring>  // For strcpy
#include <cstring>

#define MAX_INPUTTERS 9

typedef struct {
    char name[20];
    kiss_hscrollbar slider;
    kiss_entry entry;
    kiss_label label;
    float defaultValue; //the value of the middle of the slider. this is the value when the program starts.
    float sliderRange;
    int y;
} Inputter;

using namespace std;


void button_event(kiss_button *button, SDL_Event *e, int *draw, int *quit, int *myCounter)
{
    if (kiss_button_event(button, e, draw)) {
        *myCounter+=1;
        string newText = to_string(*myCounter);
        snprintf(button->text, sizeof(button->text), "%s", newText.c_str());
        strcpy(button->text, (std::to_string(*myCounter)).c_str());
    
    }
}

void slider_event(kiss_hscrollbar *slider, kiss_entry *entry, float range, float defaultValue, SDL_Event *e, int *draw) {
    if (kiss_hscrollbar_event(slider, e, draw)) {
        // set the entry->text equal to the slider's value
        float value = (slider->fraction - 0.5f) * range + defaultValue; // calculates the slider's value
        char text[16];
        snprintf(text, sizeof(text), "%.2f", value);
        strcpy(entry->text, text);
        *draw = 1;
    }
}

void control_panel_slider_event(kiss_vscrollbar *slider, kiss_window *panelControls, SDL_Event *e, int *draw) {
    if (kiss_vscrollbar_event(slider, e, draw)) {
        //code
        float multiplier = 250.0f;

        float amountScrolled = slider->fraction;
        float maxCanScroll = 1.0f;
        float ratio = amountScrolled / maxCanScroll;
        int controlPanelYOffSet = static_cast<int>(round(multiplier*(1.0f-ratio)));
        panelControls->rect.y = controlPanelYOffSet - multiplier;

        *draw = 1;
    }
}

void entry_event(kiss_entry *entry, kiss_hscrollbar *slider, float range, float defaultValue, SDL_Event *e, int *draw) {
    if (kiss_entry_event(entry, e, draw)) {
        // set the slider's value equal to the entry->text 
        float value = atof(entry->text); //string to float cast
        if (value >= defaultValue + (range / 2.0f)) {
            slider->fraction = 1.03f;
        } else if (value <= defaultValue - (range / 2.0f)) {
            slider->fraction = -0.03f;
        } else {
            slider->fraction = (value - defaultValue)/range + 0.5f; // calculates the new slider's pos
        }
        *draw = 1;
    }
}

int main(int argc, char **argv)
{
    //sdl and kiss_sdl declerations
    SDL_Renderer *renderer;
    SDL_Event e;
    kiss_array objects;
    kiss_window window;
    kiss_window panelSimulation;
    kiss_window panelControls;
    kiss_window panelGraphs;
    kiss_label label = {0};
    kiss_button button = {0};
    char message[KISS_MAX_LENGTH];

    // inputter declerations
	int inputterWidth = 324 - 20;
	int inputterYSpacing = 60;
    int inputterX = 1116 + 10;
    int inputterY = 50;
    Inputter inputters[MAX_INPUTTERS]; 

    //control panel slider declerations
    kiss_vscrollbar controlPanelScrollbar;

    //driver code declerations
    int quit = 0;
    int draw = 1;
    int myCounter = 0;
    kiss_array_new(&objects);

    renderer = kiss_init("Hello kiss_sdl", &objects, 1440, 810);
    if (!renderer) {cout << "Renderer init failed"; return 1;}
    kiss_window_new(&window, NULL, 0, 0, 0, kiss_screen_width, kiss_screen_height);
    strcpy(message, "Hello World!");
    kiss_label_new(&label, &window, message, window.rect.w / 2 - strlen(message) * kiss_textfont.advance / 2,
                                    window.rect.h / 2 - (kiss_textfont.fontheight + 2 * kiss_normal.h) / 2);
    
    kiss_window_new(&panelSimulation, &window, 2, 0, 0, 1116, 594);
    kiss_window_new(&panelGraphs, &window, 2, 0, 594, 1116, 216);
    kiss_window_new(&panelControls, &window, 2, 1116, 0, 324, 810);
    strcpy(message, "pop up");
    kiss_label_new(&label, &panelControls, message, panelControls.rect.w / 2 - strlen(message) * kiss_textfont.advance / 2,
                                    panelControls.rect.h / 2 - (kiss_textfont.fontheight + 2 * kiss_normal.h) / 2);
    label.textcolor.r = 255;
    kiss_button_new(&button, &panelControls, "OK", panelControls.rect.x, label.rect.y + kiss_textfont.fontheight + kiss_normal.h);

    { // initialise inputters
        const char* nameArray[MAX_INPUTTERS] = {"Debug1 (-100 to 100)", "Debug2 (-50 to 150)", "name3", "name4", "name5", "name6", "name7", "name8", "name9"};
        const float defaultArray[MAX_INPUTTERS] = {0.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        const float rangeArray[MAX_INPUTTERS] = {200.0f, 200.0f, 100.0f, 100.0f, 100.0f, 50.0f, 50.0f, 50.0f, 50.0f};
        for(int i =0; i<MAX_INPUTTERS; i++) {
            strcpy(inputters[i].name, nameArray[i]);
            inputters[i].defaultValue = defaultArray[i];
            inputters[i].sliderRange = rangeArray[i];
        }
        int yPos = inputterY; // set to the starting y position.
        for(int i =0; i<MAX_INPUTTERS; i++) {
            kiss_hscrollbar_new(&inputters[i].slider, &panelControls, &panelControls.rect.x, kiss_textfont.fontheight + yPos, inputterWidth - 110);
            kiss_entry_new(&inputters[i].entry, &panelControls, 1, "0.00", inputterX + inputterWidth - 110, kiss_textfont.fontheight - 5 + yPos, 100);
            kiss_label_new(&inputters[i].label, &panelControls, inputters[i].name, inputterX, yPos);
            yPos += inputterYSpacing;
            inputters[i].slider.fraction = 0.5f;
            float value = (inputters[i].slider.fraction - 0.5f) * inputters[i].sliderRange + inputters[i].defaultValue; // calculates the slider's value
            char text[16];
            snprintf(text, sizeof(text), "%.2f", value);
            strcpy(inputters[i].entry.text, text);
        }
        
    } // initialise inputters
    kiss_vscrollbar_new(&controlPanelScrollbar, &panelControls, 1440 - 20, 10, 810-10*2);

    window.visible = 1;
    panelControls.visible = 1;
    panelGraphs.visible=1;
    panelSimulation.visible=1;

    

    while (!quit) {
        SDL_Delay(10);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
            kiss_window_event(&window, &e, &draw);
            button_event(&button, &e, &draw, &quit, &myCounter); //makes quit,draw true?


            for(int i =0; i<MAX_INPUTTERS; i++) {
                slider_event(&inputters[i].slider, &inputters[i].entry, inputters[i].sliderRange, inputters[i].defaultValue, &e, &draw);
                entry_event(&inputters[i].entry, &inputters[i].slider, inputters[i].sliderRange, inputters[i].defaultValue, &e, &draw);
            }
            control_panel_slider_event(&controlPanelScrollbar, &panelControls, &e, &draw);
        }
        
        if (!draw) continue; //skips drawing
        SDL_RenderClear(renderer);
        kiss_window_draw(&window, renderer);
        kiss_window_draw(&panelSimulation, renderer);
        kiss_window_draw(&panelGraphs, renderer);
        kiss_window_draw(&panelControls, renderer);
        //kiss_label_draw(&label, renderer);
        //kiss_button_draw(&button, renderer);


        for(int i =0; i<MAX_INPUTTERS; i++) {
            kiss_hscrollbar_draw(&inputters[i].slider, renderer);
            kiss_entry_draw(&inputters[i].entry, renderer);
            kiss_label_draw(&inputters[i].label, renderer);
        }
        kiss_vscrollbar_draw(&controlPanelScrollbar, renderer);


        
        //DrawGUILabels(renderer);
        //DrawGUIControlPanel(renderer);
        SDL_RenderPresent(renderer);
        draw = 0;
    }
    kiss_clean(&objects);
    return 0;
}

/*todo:
-change combobox functionality: remove drop down from the entry box, remove the 2 labels, remove output box, move down the entry box.
-store my inputers in an array(pointer, min, max)
-pop up pseudocode, code. info box (suggested value ranges are that of the slider. this does does this. press enter to stop inputting to an inputter.   etc)
-scrolling control panel pseudocode

*/


// IF scrollbar has recieved an update:
//     //Use the ratio of how far the scrollbar is scrolled to calculate the controls y pos
//     amountScrolled = get amount scrolled
//     maxCanScroll = get the max that it can scroll
//     ratio = amountScrolled / maxCanScroll
//     ControlYOffSet = 1 - ratio //must move inversely to scrollbar
// ENDIF
