// Includes
    #include "kiss_sdl.h"
    // #include "event_handlers.h"
    #include <iostream>
    #include <cstring> // For strcpy
    #include <cmath> // For trig functions
    #include <algorithm> // For std::max and std::min
    #include <random> // For testing
    using namespace std;

// Defines
    #define MAX_INPUTTERS 9
    #define MAX_INFOBUTTONS 3
    #define MAX_STATIC_COLLIDERS 20

// New types
    class Vec2 {
        public: 
            float x, y;
        
        Vec2(float a = 0.0, float b = 0.0) {
            x = a;
            y = b;
        }
        
        // Scalar Vector product using operator*
        public: Vec2 operator*(float other) {
            return Vec2(x * other, y * other);
        }

        // Dot product using operator*
        float operator*(const Vec2& other) const {
            return (x * other.x) + (y * other.y);
        }

        // Vec2 addition using operator+
        Vec2 operator+(const Vec2& other) const {
            return Vec2(x + other.x, y + other.y);
        }

        // Vec2 += addition using operator+=
        Vec2& operator+=(const Vec2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        // Vec2 subtraction using operator-
        Vec2 operator-(const Vec2& other) const {
            return Vec2(x - other.x, y - other.y);
        }

        // Scalar Vector quotient using operator/
        public: Vec2 operator/(float other) {
            return Vec2(x / other, y / other);
        }
    };

// Utility functions
    float LengthOfVector(Vec2 myVector) {
        return sqrt(myVector.x*myVector.x + myVector.y*myVector.y);
    }
    Vec2 Normalise(Vec2 myVector) {
        return myVector/LengthOfVector(myVector);
    }

    Vec2 Perpendicular(Vec2 myVector) { // 90 degree anti-clockwise rotation graphically. // but clockwise if y-axis is up
        return Vec2(myVector.y, -myVector.x);
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
// Structs
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
        float coefOfRestitution = 0.7f;
        float coefOfFriction = 0.95f;

        Vec2 position;
        Vec2 prevPosition;
        Vec2 velocity;
    } Payload;

    typedef struct Collider { // must be such that left side of collider is the exterior
        Vec2 startPos;
        Vec2 endPos;
        Vec2 direction;
        Collider(Vec2 startPos1, Vec2 endPos1) {
            startPos = startPos1;
            endPos = endPos1;
            direction = Normalise(endPos - startPos);
        } // constructor
    } Collider;

// Simulation utility functions

    float pointToColliderDisplacement(Vec2 point, Collider collider) { // c is wrong I think.
        float c = collider.direction.x * collider.startPos.y - collider.direction.y * collider.startPos.x;
        return collider.direction.y * point.x - collider.direction.x * point.y + c;
    }

    // Given three collinear points p, q, r, the function checks if 
    // point q lies on line segment 'pr' 
    bool onSegment(const Vec2& p, const Vec2& q, const Vec2& r) {
        if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
            q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)) {
            return true;
        }
        return false;
    }

    // To find orientation of ordered triplet (p, q, r). 
    // The function returns following values 
    // 0 --> p, q and r are collinear Perpendicuar
    // 1 --> Clockwise 
    // 2 --> Counterclockwise 
    int orientation(const Vec2& p, const Vec2& q, const Vec2& r) {
        float val = (q.y - p.y) * (r.x - q.x) -
                    (q.x - p.x) * (r.y - q.y);

        if (val == 0) return 0; // collinear
        return (val > 0) ? 1 : 2; // clock or counterclock wise
    }

    // The main function that returns true if line segment 'p1q1' 
    // and 'p2q2' intersect.
    bool lineSegmentsIntersect(const Vec2& p1, const Vec2& q1, const Vec2& p2, const Vec2& q2) {
        // Find the four orientations needed for general and special cases
        int o1 = orientation(p1, q1, p2);
        int o2 = orientation(p1, q1, q2);
        int o3 = orientation(p2, q2, p1);
        int o4 = orientation(p2, q2, q1);

        // General case
        if (o1 != o2 && o3 != o4)
            return true;

        // Special Cases
        // p1, q1 and p2 are collinear and p2 lies on segment p1q1
        if (o1 == 0 && onSegment(p1, p2, q1)) return true;

        // p1, q1 and q2 are collinear and q2 lies on segment p1q1
        if (o2 == 0 && onSegment(p1, q2, q1)) return true;

        // p2, q2 and p1 are collinear and p1 lies on segment p2q2
        if (o3 == 0 && onSegment(p2, p1, q2)) return true;

        // p2, q2 and q1 are collinear and q1 lies on segment p2q2
        if (o4 == 0 && onSegment(p2, q1, q2)) return true;

        return false; // Doesn't fall in any of the above cases
    }

// Simlulation functions
    bool isColliding(Payload payload, Collider collider) {
        // condition: is behind
        if (pointToColliderDisplacement(payload.position, collider) > payload.radius) {return false;}
        // condition: centre path intersects collider
        Vec2 expandedStartPos = collider.startPos - collider.direction*payload.radius;
        Vec2 expandedEndPos = collider.endPos + collider.direction*payload.radius;
        if (lineSegmentsIntersect(payload.prevPosition, payload.position, expandedStartPos, expandedEndPos)) {return true;}
        // condition: centre to adjusted point path intersects collider
        Vec2 adjustedPayloadPos = payload.position - Perpendicular(collider.direction)*payload.radius;
        if (lineSegmentsIntersect(payload.prevPosition, adjustedPayloadPos, expandedStartPos, expandedEndPos)) {return true;}
        return false;
    } // collider.direction must be a unit vector

    void ballToStaticColliderCollision(Payload *payload, Collider collider) {
        float displacementIntoCollider = payload->radius - pointToColliderDisplacement(payload->position, collider);
        Vec2 normal = Perpendicular(collider.direction);
        payload->position += normal*displacementIntoCollider; // Move to collider exterior surface.
        payload->velocity += normal * -(payload->coefOfRestitution +1)*(payload->velocity*normal); // Resolve collision
    }

// Event handlers
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


int main(int argc, char **argv)
{
    // Declerations
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
            bool simulationIsActive = true;
            bool isTraining = false;
            // constants
                const float gravity = 0.65 * 9.81f;
                const int floorHeight = 589; //from top
            // payload
                Payload payload;
                payload.position.x = 300;
                payload.position.y = 30;
            // colliders
                Collider staticColliders[MAX_STATIC_COLLIDERS] = {
                    // Collider(Vec2(450.,600.), Vec2(450.,200)),
                    // Collider(Vec2(100.,400.), Vec2(750.,400)),
                    Collider(Vec2(0,125+( rand() % 20 - 10)), Vec2(100,125+( rand() % 20 - 10))),
                    Collider(Vec2(100,250+( rand() % 20 - 10)), Vec2(200,250+( rand() % 20 - 10))),
                    Collider(Vec2(0,375+( rand() % 20 - 10)), Vec2(100,375+( rand() % 20 - 10))),
                    Collider(Vec2(100,500+( rand() % 20 - 10)), Vec2(200,500+( rand() % 20 - 10))),

                    Collider(Vec2(0+200,125+( rand() % 20 - 10)), Vec2(100+200,125+( rand() % 20 - 10))),
                    Collider(Vec2(100+200,250+( rand() % 20 - 10)), Vec2(200+200,250+( rand() % 20 - 10))),
                    Collider(Vec2(0+200,375+( rand() % 20 - 10)), Vec2(100+200,375+( rand() % 20 - 10))),
                    Collider(Vec2(100+200,500+( rand() % 20 - 10)), Vec2(200+200,500+( rand() % 20 - 10))),

                    Collider(Vec2(0+400,125+( rand() % 20 - 10)), Vec2(100+400,125+( rand() % 20 - 10))),
                    Collider(Vec2(100+400,250+( rand() % 20 - 10)), Vec2(200+400,250+( rand() % 20 - 10))),
                    Collider(Vec2(0+400,375+( rand() % 20 - 10)), Vec2(100+400,375+( rand() % 20 - 10))),
                    Collider(Vec2(100+400,500+( rand() % 20 - 10)), Vec2(200+400,500+( rand() % 20 - 10))),

                    Collider(Vec2(0+600,125+( rand() % 20 - 10)), Vec2(100+600,125+( rand() % 20 - 10))),
                    Collider(Vec2(100+600,250+( rand() % 20 - 10)), Vec2(200+600,250+( rand() % 20 - 10))),
                    Collider(Vec2(0+600,375+( rand() % 20 - 10)), Vec2(100+600,375+( rand() % 20 - 10))),
                    Collider(Vec2(100+600,500+( rand() % 20 - 10)), Vec2(200+600,500+( rand() % 20 - 10))),

                    Collider(Vec2(0+800,125+( rand() % 20 - 10)), Vec2(100+800,125+( rand() % 20 - 10))),
                    Collider(Vec2(100+800,250+( rand() % 20 - 10)), Vec2(200+800,250+( rand() % 20 - 10))),
                    Collider(Vec2(0+800,375+( rand() % 20 - 10)), Vec2(100+800,375+( rand() % 20 - 10))),
                    Collider(Vec2(100+800,500+( rand() % 20 - 10)), Vec2(200+800,500+( rand() % 20 - 10))),

                    //1116, 594
                    };
        // misc
            int frameDelayValue;
            int myCounter = 0;
            Uint32 ticksLastTime = SDL_GetTicks(); //the last recorded time
            Uint32 fpsCurrent; //the current FPS.
            Uint32 fpsFrameCount = 0; //frames passed since the last recorded fps
            kiss_label fpsLabel = {0};
    
    // Initialisations
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
            const float defaultArray[MAX_INPUTTERS] = {0.0f, 0.9f, 65.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
            const float rangeArray[MAX_INPUTTERS] = {200.0f, 4.0f, 240.0f, 26.0f, 100.0f, 50.0f, 50.0f, 50.0f, 50.0f};

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

        // MISC
            kiss_label_new(&fpsLabel, &window, "FPS: 0", 1040, 4);

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
    Vec2 origin = Vec2(1116/2, 594/2);
    Vec2 endEffector;
    float debug1;
    int x,y;

    bool showBall = true;

    while (!quit) {
        // fps code
            fpsFrameCount++;
            if (ticksLastTime < SDL_GetTicks() - 1000)
            {
                ticksLastTime = SDL_GetTicks();
                fpsCurrent = fpsFrameCount;
                fpsFrameCount = 0;
                sprintf(fpsLabel.text, "FPS: %d", fpsCurrent);
            }
            // Frame delay    
            SDL_Delay(abs(atoi(inputters[2].entry.text)););
            
        while (SDL_PollEvent(&e)) { // Inputs, Events
            if (e.type == SDL_QUIT) quit = true;
            // if (e.type == SDL_KEYDOWN) { //input detections
                // switch(e.key.keysym.sym) {
                //     case SDLK_RIGHT: 
                //         staticColliders[1].endPos.x += 1; 
                //         cout << "Right was pressed" << endl;
                //         break;
                //     case SDLK_LEFT:
                //         staticColliders[1].endPos.x += -1;
                //         cout << "Left was pressed" << endl;
                //         break;
                //     case SDLK_UP:
                //         staticColliders[1].endPos.y += -1; 
                //         cout << "Up was pressed" << endl;
                //         break;
                //     case SDLK_DOWN:
                //         staticColliders[1].endPos.y += 1;
                //         cout << "Down was pressed" << endl;
                //         break;
                    
                //}
            //}

            kiss_window_event(&window, &e, &draw);
            
            if (popupIsActive) {
                popup_event(&popupIsActive, &popupButton, &popupLabel, &popupRect, &e, &draw);
                //break;
            }

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

        

        // { // training and simulation
        //     if (isTraining) {
        //         train_generation();
        //         display_generation(); // has its own draw functionality
        //     } else if (simulationIsActive) {
        //         run_simulation();
        //         draw = true;
        //     }

        //     // create generation

        //     // run simulation environment
            
        //     // select/crossbreed/mutate a new generation
        // }


        draw = true; // REMOVE LATER

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
                endEffector.x = 350*cos(rotationAngle) + origin.x;
                endEffector.y = 350*sin(rotationAngle) + origin.y;
                SDL_RenderDrawLine(renderer, origin.x, origin.y, endEffector.x, endEffector.y);
            }
            if (showBall) { // Draw Ball Bouncing Simulation
                int subStepsPerFrame = abs(atoi(inputters[3].entry.text)) +1;
                payload.coefOfRestitution = atof(inputters[1].entry.text);
                for (int step = 0; step < subStepsPerFrame; step++) {

                    payload.prevPosition = payload.position;
                    payload.velocity.y += gravity/(subStepsPerFrame);
                    payload.position += payload.velocity;
                    
                    // Floor collision
                    if (payload.position.y + payload.radius > floorHeight) { // If below floor
                        payload.position.y = floorHeight - payload.radius; // Set pos to on floor
                        payload.velocity.y *= -payload.coefOfRestitution; // Resolve collision
                        payload.velocity.x *= payload.coefOfFriction; // friction
                    }
                    if (payload.position.x < payload.radius) { // If below floor
                        payload.position.x = payload.radius; // Set pos to on floor
                        payload.velocity.x *= -payload.coefOfRestitution; // Resolve collision
                    }
                    if (payload.position.y < payload.radius) { // If below floor
                        payload.position.y = payload.radius; // Set pos to on floor
                        payload.velocity.y *= -payload.coefOfRestitution; // Resolve collision
                    }
                    if (payload.position.x > 1116 - payload.radius) { // If below floor
                        payload.position.x = 1116 - payload.radius; // Set pos to on floor
                        payload.velocity.x *= -payload.coefOfRestitution; // Resolve collision
                    }
                    for(int i =0; i<MAX_STATIC_COLLIDERS; i++) {
                        if (isColliding(payload, staticColliders[i])) {
                            ballToStaticColliderCollision(&payload, staticColliders[i]);
                        }
                    }

                DrawCircle(renderer, payload.position.x, payload.position.y, payload.radius);
                }

        
                // SDL_Delay(140-frameDelayValue);
            }

            // Drawing colliders
                for(int i =0; i<MAX_STATIC_COLLIDERS; i++) { 
                    SDL_RenderDrawLine(renderer, staticColliders[i].startPos.x, staticColliders[i].startPos.y, staticColliders[i].endPos.x, staticColliders[i].endPos.y);
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
            // Drawing FPS counter
                kiss_label_draw(&fpsLabel, renderer);

            SDL_RenderPresent(renderer);
            draw = false;
        }
    }
    kiss_clean(&objects);
    return 0;
}


