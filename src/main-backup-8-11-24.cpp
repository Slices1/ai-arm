#include "kiss_sdl.h"
#include "event_handlers.h"
#include <iostream>
#include <cstring> // For strcpy
#include <cmath> // For trig functions
using namespace std;

#define MAX_INPUTTERS 9
#define MAX_INFOBUTTONS 3

typedef struct {
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

typedef struct {
    int radius = 30;
    float coefficientOfRestitution = 0.7f;
    float coefficientOfFriction = 0.9f;

    float position[2];
    float velocity[2];
} Payload;





void button_event(kiss_button *button, SDL_Event *e, int *draw, int *quit, int *myCounter)
{
    if (kiss_button_event(button, e, draw)) {
        *myCounter+=1;
        string newText = to_string(*myCounter);
        snprintf(button->text, sizeof(button->text), "%s", newText.c_str());
        strcpy(button->text, (std::to_string(*myCounter)).c_str());
    }
}

void slider_event(Inputter *inputter, SDL_Event *e, int *draw) {
    if (kiss_hscrollbar_event(&inputter->slider, e, draw)) {
        // set the entry->text equal to the slider's value
        float value = (inputter->slider.fraction - 0.5f) * inputter->sliderRange + inputter->defaultValue; // calculates the slider's value
        char text[16];
        snprintf(text, sizeof(text), "%.2f", value);
        strcpy(inputter->entry.text, text);
        *draw = true;
    }
}

void control_panel_slider_event(kiss_vscrollbar *slider, Inputter inputters[], SDL_Event *e, int *draw) { //this passes the dereferenced inputters array and it works but not sure why.
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

void entry_event(Inputter *inputter, SDL_Event *e, int *draw) {
    if (kiss_entry_event(&inputter->entry, e, draw)) {
        // set the slider's value equal to the entry->text 
        float value = atof(inputter->entry.text); //string to float cast
        if (value >= inputter->defaultValue + (inputter->sliderRange / 2.0f)) {
            inputter->slider.fraction = 1.03f;
        } else if (value <= inputter->defaultValue - (inputter->sliderRange / 2.0f)) {
            inputter->slider.fraction = -0.03f;
        } else {
            inputter->slider.fraction = (value - inputter->defaultValue)/inputter->sliderRange + 0.5f; // calculates the new slider's pos
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

void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

            if (error <= 0)
            {
                ++y;
                error += ty;
                ty += 2;
            }

            if (error > 0)
            {
                --x;
                tx += 2;
                error += (tx - diameter);
            }

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
    // simulation declerations
        Payload payload;
        payload.velocity[0] = 0;
        payload.velocity[1] = 0;
        payload.position[0] = 300;
        payload.position[1] = 100;
        float gravity = 9.81f;
        int floorHeight = 589; //from top
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
        const char* nameArray[MAX_INPUTTERS] = {"Debug1 (-100 to 100)", "Debug2 (-50 to 150)", "Frame Delay (ms)", "Additional Sim Sub Steps (rounded)", "name5", "name6", "name7", "name8", "name9"};
        const float defaultArray[MAX_INPUTTERS] = {0.0f, 50.0f, 30.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        const float rangeArray[MAX_INPUTTERS] = {200.0f, 200.0f, 60.0f, 26.0f, 100.0f, 50.0f, 50.0f, 50.0f, 50.0f};

        for(int i =0; i<MAX_INPUTTERS; i++) {
            inputters[i].defaultValue = defaultArray[i];
            inputters[i].sliderRange = rangeArray[i];
        }
        int yPos = inputterY; // set to the starting y position.
        for(int i =0; i<MAX_INPUTTERS; i++) {
            kiss_hscrollbar_new(&inputters[i].slider, &panelControls, inputterX, kiss_textfont.fontheight + yPos, inputterWidth - 110);
            kiss_entry_new(&inputters[i].entry, &panelControls, 1, "0.00", inputterX + inputterWidth - 110, kiss_textfont.fontheight - 5 + yPos, 100);
            kiss_label_new(&inputters[i].label, &panelControls, const_cast<char *>(nameArray[i]), inputterX, yPos);
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
        show_popup(InitialPopupMessage, &popupIsActive, &popupLabel);
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
    
    //1116, 594
    bool showPendulum = false;
    float rotationAngle = 25; //relative to the horizontal
    float rotationSpeed = 0; // anticlockwise
    int originx = 1116/2;
    int originy =  594/2;
    int endEffectorx;
    int endEffectory;
    float debug1;

    bool showBall = true;

    while (!quit) {
        { // Frame delay
            frameDelayValue = abs(atoi(inputters[2].entry.text));
            SDL_Delay(frameDelayValue);
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
                slider_event(&inputters[i], &e, &draw);
                entry_event(&inputters[i], &e, &draw);
            }
            control_panel_slider_event(&controlPanelScrollbar, inputters, &e, &draw);

            // info button events
                for (int i = 0; i<MAX_INFOBUTTONS; i++) {
                    infoButton_event(&infoButtons[i], &popupIsActive, &popupLabel, &e, &draw);
                }
        }

        draw = true; // REMOVE LATER (once sim code is out of the draw block)

             
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

            if (showPendulum) { // Draw Pendulum Simulation 
                debug1 = atof(inputters[0].entry.text);
                rotationSpeed += cos(rotationAngle)*0.07f + -debug1*0.0007f;
                rotationSpeed *= 0.98f;
                rotationAngle += rotationSpeed;
                endEffectorx = 350*cos(rotationAngle) + originx;
                endEffectory = 350*sin(rotationAngle) + originy;
                SDL_RenderDrawLine(renderer, originx, originy, endEffectorx, endEffectory);
            }
            if (showBall) { // Draw Ball Bouncing Simulation
                int subStepsPerFrame = abs(atoi(inputters[3].entry.text)) +1;
                for (int step = 0; step < subStepsPerFrame; step++) {
                    if (payload.position[1] + payload.radius > floorHeight) {
                        payload.position[1] = floorHeight - payload.radius;
                        payload.velocity[1] *= -payload.coefficientOfRestitution;
                    }
                    payload.velocity[1] += gravity/(subStepsPerFrame);
                    payload.position[0] += payload.velocity[0];
                    payload.position[1] += payload.velocity[1];
                    DrawCircle(renderer, payload.position[0], payload.position[1], payload.radius);
                }
                SDL_Delay(140-frameDelayValue);
            }

            // Draw window titles
                kiss_makerect(&panelControlsTitleRect, 1220, 4, 120, 15);
                kiss_fillrect(renderer, &panelControlsTitleRect, kiss_white);
                kiss_label_draw(&panelControlsTitle, renderer);
                kiss_label_draw(&panelSimulationTitle, renderer);
                kiss_label_draw(&panelGraphsTitle, renderer);

            // Drawing info buttons
                for (int i =0; i<MAX_INFOBUTTONS; i++) {
                    kiss_button_draw(&infoButtons[i].button, renderer);
                }
            // Drawing pop up
                if (popupIsActive) {
                    kiss_makerect(&popupRect, kiss_screen_width/2 - 200, kiss_screen_height/2 - 300, 400, 460);
                    kiss_fillrect(renderer, &popupRect, kiss_lightblue);
                    kiss_decorate(renderer, &popupRect, kiss_blue, kiss_edge);
                    kiss_label_draw(&popupLabel, renderer);
                    kiss_button_draw(&popupButton, renderer);
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
