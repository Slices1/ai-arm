#include "kiss_sdl.h"
#include "event_handlers.h"
#include <iostream>
#include <cstring> // For strcpy
using namespace std;

#define MAX_INPUTTERS 9
#define MAX_INFOBUTTONS 3

typedef struct {
    char name[20];
    kiss_hscrollbar slider;
    kiss_entry entry;
    kiss_label label;
    float defaultValue; //the value of the middle of the slider. this is the value when the program starts.
    float sliderRange;
    int y;
} Inputter;

typedef struct {
    kiss_button button;
    string text;
} InfoButton;



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
        *draw = true;
    }
}

void control_panel_slider_event(kiss_vscrollbar *slider, Inputter (inputters[]), SDL_Event *e, int *draw) {
    if (kiss_vscrollbar_event(slider, e, draw)) {
        float multiplier = 250.0f;
        float amountScrolled = slider->fraction;
        float maxCanScroll = 1.0f;
        float ratio = amountScrolled / maxCanScroll;
        int controlPanelYOffSet = static_cast<int>(round(multiplier*(1.0f-ratio)));
        for (int i = 0; i< MAX_INPUTTERS; i++) {
            inputters[i].entry.rect.y = inputters[i].y + controlPanelYOffSet - multiplier;
            inputters[i].entry.texty = inputters[i].y + controlPanelYOffSet + 6 - multiplier;

            inputters[i].slider.leftrect.y = inputters[i].y + controlPanelYOffSet - multiplier + 6;
            inputters[i].slider.rightrect.y = inputters[i].y + controlPanelYOffSet - multiplier + 6;
            inputters[i].slider.sliderrect.y = inputters[i].y + controlPanelYOffSet - multiplier + 6;

            inputters[i].label.rect.y = inputters[i].y + controlPanelYOffSet - multiplier - 10;
        }

        *draw = true;
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
        *draw = true;
    }
}

void popup_event(bool *popupIsActive, kiss_button *popupButton, kiss_label *popupLabel, SDL_Rect *popupRect, SDL_Event *e, int *draw) {
    if (kiss_button_event(popupButton, e, draw)) {
        *popupIsActive = false;
        *draw = true;
    }
}

void show_popup(string message, bool *popupIsActive, kiss_label *popupLabel) {
    *popupIsActive = true;
    snprintf(popupLabel->text, sizeof(popupLabel->text), "%s", message.c_str());
}

void infoButton_event(InfoButton *infoButton, bool *popupIsActive, kiss_label *popupLabel, SDL_Event *e, int *draw) {
    if (kiss_button_event(&infoButton->button, e, draw)) {
        show_popup(infoButton->text, popupIsActive, popupLabel);
    }
}

int main(int argc, char **argv)
{
    // sdl and kiss_sdl declerations
        SDL_Renderer *renderer;
        SDL_Event e;
        kiss_array objects;
        kiss_window window;

        kiss_window panelSimulation;
        kiss_window panelControls;
        kiss_window panelGraphs;

        kiss_label panelSimulationTitle = {0};
        kiss_label panelGraphsTitle = {0};
        kiss_label panelControlsTitle = {0};
        SDL_Rect panelControlsTitleRect;
        //kiss_button button = {0};

    // inputter declerations
        int inputterWidth = 324 - 20;
        int inputterYSpacing = 60;
        int inputterX = 1116 + 10;
        int inputterY = 50;
        Inputter inputters[MAX_INPUTTERS]; 

    // control panel slider declerations
        kiss_vscrollbar controlPanelScrollbar;

    // driver code declerations
        int quit = false;
        int draw = true;
        kiss_array_new(&objects);
    // pop up
        bool popupIsActive = true;
        kiss_button popupButton = {0};
        kiss_label popupLabel = {0};
        SDL_Rect popupRect;

    // info buttons
        InfoButton infoButtons[MAX_INFOBUTTONS];
    // misc
        int frameDelayValue = 10;
        int myCounter = 0;


    {// initialise renderer, windows
        renderer = kiss_init("AI Controlled Manipulator Arm", &objects, 1440, 810);
        if (!renderer) {cout << "Renderer init failed"; return 1;}

        kiss_window_new(&window, NULL, 0, 0, 0, kiss_screen_width, kiss_screen_height);
        kiss_window_new(&panelSimulation, &window, 2, 0, 0, 1116, 594);
        kiss_window_new(&panelGraphs, &window, 2, 0, 594, 1116, 216);
        kiss_window_new(&panelControls, &window, 2, 1116, 0, 324, 810);
        kiss_label_new(&panelControlsTitle, &window,"Control Panel", 1220, 3);
        kiss_label_new(&panelSimulationTitle, &window,"Simulation Panel", 570, 3);
        kiss_label_new(&panelGraphsTitle, &window,"Graph Panel", 500, 597);
    }

    { // initialise control panel
        //slider values
        const char* nameArray[MAX_INPUTTERS] = {"Debug1 (-100 to 100)", "Debug2 (-50 to 150)", "Frame Delay (ms)", "name4", "name5", "name6", "name7", "name8", "name9"};
        const float defaultArray[MAX_INPUTTERS] = {0.0f, 50.0f, 30.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        const float rangeArray[MAX_INPUTTERS] = {200.0f, 200.0f, 60.0f, 100.0f, 100.0f, 50.0f, 50.0f, 50.0f, 50.0f};

        for(int i =0; i<MAX_INPUTTERS; i++) {
            strcpy(inputters[i].name, nameArray[i]);
            inputters[i].defaultValue = defaultArray[i];
            inputters[i].sliderRange = rangeArray[i];
        }
        int yPos = inputterY; // set to the starting y position.
        for(int i =0; i<MAX_INPUTTERS; i++) {
            kiss_hscrollbar_new(&inputters[i].slider, &panelControls, inputterX, kiss_textfont.fontheight + yPos, inputterWidth - 110);
            kiss_entry_new(&inputters[i].entry, &panelControls, 1, "0.00", inputterX + inputterWidth - 110, kiss_textfont.fontheight - 5 + yPos, 100);
            kiss_label_new(&inputters[i].label, &panelControls, inputters[i].name, inputterX, yPos);
            inputters[i].y = yPos;
            yPos += inputterYSpacing;
            //send an update
            inputters[i].slider.fraction = 0.5f;
            float value = (inputters[i].slider.fraction - 0.5f) * inputters[i].sliderRange + inputters[i].defaultValue; // calculates the slider's value
            char text[16];
            snprintf(text, sizeof(text), "%.2f", value);
            strcpy(inputters[i].entry.text, text);
        } // initialise slider inputters
        
        kiss_vscrollbar_new(&controlPanelScrollbar, &panelControls, 1440 - 20, 10, 810-10*2);
    }

    { // inistialise popup
        kiss_button_new(&popupButton, &window, "Okay", kiss_screen_width/2 - 30, kiss_screen_height/2 + 120);
        kiss_label_new(&popupLabel, &window, "My pop up heehee\nnew line test", kiss_screen_width/2 - 180, kiss_screen_height/2 - 280);
        string InitialPopupMessage = "            Welcome to Jonah's\n      Computer Science NEA Project\n      AI Controlled Manipulator Arm\n         (That you can train)!\n\nOverview:\nThis GUI is split into 3 separate panels.\n\nThe simulation panel displayes the\nevaluation process of each neural network\nand allows the user to see training,\nvalues, objectives & fitness criteria\nin real time.\n\nThe control panel is where you are\nable to see all of these options.\nStart training from the top\nof this panel.\n\nClick the ?s to find out more.";
        //show_popup(InitialPopupMessage, &popupIsActive, &popupLabel);
        popupIsActive = false;
    }

    { // initialise infoButtons
        kiss_button_new(&infoButtons[0].button, &panelGraphs, "?", 60-7, 597);
        infoButtons[0].text = "This is the graph panel.\nIt hopes to give insight into\nthe fitness of neural networks\nwith respect to time.\nThe left shows the average";
        kiss_button_new(&infoButtons[1].button, &panelSimulation, "?", 100, 3);
        infoButtons[1].text = "The simulation panel displays the\nevaluation process of each neural network\nand allows the user to see training,\nvalues, objectives & fitness criteria\nin real time.";
        kiss_button_new(&infoButtons[2].button, &panelControls, "?", 1340, 3);
        infoButtons[2].text = "The control panel is where you are\nable to see all of these options.\nStart training from the top\nof this panel...";
    }

    {// set the windows visible
        window.visible = true;
        panelControls.visible = true;
        panelGraphs.visible = true;
        panelSimulation.visible = true;
    } 
    
    while (!quit) {
        { // Frame delay
            frameDelayValue = static_cast<unsigned int>(round(atof(inputters[2].entry.text)));
            SDL_Delay(frameDelayValue);
        }
        { // test
        }
        while (SDL_PollEvent(&e)) { // Inputs, Events
            if (e.type == SDL_QUIT) quit = true;
            kiss_window_event(&window, &e, &draw);
            
            if (popupIsActive) {
                popup_event(&popupIsActive, &popupButton, &popupLabel, &popupRect, &e, &draw);
                //break;
            }
            
            //button_event(&button, &e, &draw, &quit, &myCounter); //makes quit,draw true?

            for(int i =0; i<MAX_INPUTTERS; i++) {
                slider_event(&inputters[i].slider, &inputters[i].entry, inputters[i].sliderRange, inputters[i].defaultValue, &e, &draw);
                entry_event(&inputters[i].entry, &inputters[i].slider, inputters[i].sliderRange, inputters[i].defaultValue, &e, &draw);
            }
            control_panel_slider_event(&controlPanelScrollbar, inputters, &e, &draw);

            // info button events
                for (int i = 0; i<MAX_INFOBUTTONS; i++) {
                    infoButton_event(&infoButtons[i], &popupIsActive, &popupLabel, &e, &draw);
                }
        }

             
        { // Drawing
            if (!draw) continue; //skips drawing
            // Draw windows, misc sdl
                SDL_RenderClear(renderer);
                kiss_window_draw(&window, renderer);
                kiss_window_draw(&panelSimulation, renderer);
                kiss_window_draw(&panelGraphs, renderer);
                kiss_window_draw(&panelControls, renderer);
                //kiss_button_draw(&button, renderer);


            for(int i =0; i<MAX_INPUTTERS; i++) {
                kiss_hscrollbar_draw(&inputters[i].slider, renderer);
                kiss_entry_draw(&inputters[i].entry, renderer);
                kiss_label_draw(&inputters[i].label, renderer);
            }
            kiss_vscrollbar_draw(&controlPanelScrollbar, renderer);

            //Draw window titles
                kiss_makerect(&panelControlsTitleRect, 1220, 4, 120, 15);
                kiss_fillrect(renderer, &panelControlsTitleRect, kiss_white);
                kiss_label_draw(&panelControlsTitle, renderer);
                kiss_label_draw(&panelSimulationTitle, renderer);
                kiss_label_draw(&panelGraphsTitle, renderer);

            //Drawing pop up
                if (popupIsActive) {
                    kiss_makerect(&popupRect, kiss_screen_width/2 - 200, kiss_screen_height/2 - 300, 400, 460);
                    kiss_fillrect(renderer, &popupRect, kiss_lightblue);
                    kiss_decorate(renderer, &popupRect, kiss_blue, kiss_edge);
                    kiss_label_draw(&popupLabel, renderer);
                    kiss_button_draw(&popupButton, renderer);
                }
            // Drawing info buttons
                for (int i =0; i<MAX_INFOBUTTONS; i++) {
                    kiss_button_draw(&infoButtons[i].button, renderer);
                }

            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
    kiss_clean(&objects);
    return 0;
}

/*todo:
-change combobox functionality: remove drop down from the entry box, remove the 2 labels, remove output box, move down the entry box.
-store my inputers in an array(pointer, min, max)
-pop up pseudocode, code. info box (suggested value ranges are that of the slider. this does does this. press enter to stop inputting to an inputter.   etc)
*/
