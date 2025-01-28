// Includes
    #include "kiss_sdl.h"
    // #include "event_handlers.h"
    #include <iostream>
    #include <cstring> // For strcpy
    #include <cmath> // For trig functions
    #include <algorithm> // For std::max and std::min
    #include <random> // For testing
    #include <ctime>   // Needed for time() for random seed
    #include <memory> // for vectors ::make_shared
    #include <vector>
    #include <cstdlib> // for evolve

    
    using namespace std;

// Defines
    #define MAX_INPUTTERS 12
    #define MAX_INFOBUTTONS 3
    #define MAX_STATIC_COLLIDERS 1
    #define MAX_LINKAGES 8

// New types
    class Vec2 {
        public: 
            float x, y;
        
        Vec2(float a = 0.0, float b = 0.0) {
            x = a;
            y = b;
        }
        
        // Operator overloads
            // Scalar Vector product using operator*
            Vec2 operator*(float other) {
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

            // Vec2 unary subtraction using operator-
            Vec2 operator-() const {
                return Vec2(-x, -y);
            }

            // Scalar Vector quotient using operator/
            Vec2 operator/(float other) {
                return Vec2(x / other, y / other);
            }

        float magnitude_squared() {
            return x*x + y*y;
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

    void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius) {
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

    float optimisedTanh(float x) {
        // return 1 or -1 if x is 20 from 0, respectively
        // this prevents temp evaluating to nan for large x values and optimises the function
        if (x > 20) {return 1;}
        if (x < -20) {return -1;}
        float temp = pow(2, x); // optimisation since I dont think pow() is memoised
        return (temp - 1) / (temp + 1);
    }

    float ReLU(float x) {
        if (x>0) {return x;}
        return 0;
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
// Neural Network classes
    class Layer {
        private:
            int numNeurons; // number of neurons in the current layer
            int numInputNeurons; // number of neurons in the previous layer

        public:
            std::vector<std::vector<float>> weights;
            std::vector<float> biases;
            // Constructor - biases gets initialised with length numNeurons and weights gets initialised with dimensions numNeurons x numInputNeurons
            Layer(int numInputNeurons, int numNeurons) : numNeurons(numNeurons), numInputNeurons(numInputNeurons),
                                    weights(numNeurons, std::vector<float>(numInputNeurons)), biases(numNeurons) {
                // Initialise random weights and biases from -1.00 to 1.00
                for (int i = 0; i < numNeurons; i++) {
                    for (int j = 0; j < numInputNeurons; j++) {
                        weights[i][j] = (rand() % 200 - 100) / 100.0f;
                    }
                    biases[i] = (rand() % 100 - 50) / 100.0f;
                }
            }

            std::vector<float> CalculateLayerOutput(const std::vector<float>& inputs) {
                std::vector<float> outputs(numNeurons, 0.0f); // initialise output tensor with all 0s
                for (int i = 0; i < numNeurons; i++) {
                    // add the weighted sums of inputs
                    for (int j = 0; j < numInputNeurons; j++) {
                        outputs[i] += weights[i][j] * inputs[j];
                    }
                    // add biases
                    outputs[i] += biases[i];
                    // make it not do the output layer
                    if (i < numNeurons-1) {
                        outputs[i] = optimisedTanh(outputs[i]); // apply activation function
                    }

                }
                return outputs;
            }
    };
    class NeuralNetwork {
        private:
            const int numLayers = 7; // 3 layers: hidden, hidden, output. input is not technically a layer here.

        public:
            std::vector<std::shared_ptr<Layer>> layers;
            NeuralNetwork(const int numInputs, const int numOutputs) { // Constructor
                const int numberOfNeurons[numLayers+1] = {numInputs, 10, 9, 8, 7, 6, 5, numOutputs};
                // initialise layers tensor
                for (int i = 0; i < numLayers; i++) {
                    layers.push_back(std::make_shared<Layer>(numberOfNeurons[i], numberOfNeurons[i + 1]));
                }
            }

            NeuralNetwork(const NeuralNetwork& other) : numLayers(other.numLayers){ // Deep copy constructor
                layers.reserve(other.layers.size()); // Optimisation to prepare the layers vector for the adding of other.layers 
                for (const auto& layer : other.layers) {
                    layers.push_back(std::make_shared<Layer>(*layer));
                }
            }

            std::vector<float> CalculateOutputs(std::vector<float>& inputs) {
                for (auto& layer : layers) {
                    // feed the outputs back in as inputs
                    inputs = layer->CalculateLayerOutput(inputs);
                }
                return inputs;
            }
    };

// Contexts
    class Simulation; // Forward declaration

    struct SimulationContext {
        const int numInstances = 1000; // the number of simulations/NNs that will be stored at once.
        const int maxTrainingFrames = 15*45; // 15 seconds // the number of frames that the simulation will run for each training instance.
        // simulation declerations
            const int numOfLinkages = 4; // Arm linkage count
            bool isTraining = false;
            float gravity = 1.2f;
            float coefOfRestitution = 0.7f;
            float coefOfFriction = 0.92f;
            const int floorHeight = 594-3;
            float constraintAngle = M_PI/3; // max radians from the centre angle.
            Vec2 armOriginPos = Vec2(558.0f, 591.0f);
            int radius = 30; // payload radius
            int linkageLength = 90; // Arm linkage length
            Vec2 payloadSpawnPosition = Vec2(radius + 100, floorHeight - radius);
            Vec2 targetPosition = Vec2(1000.0f, floorHeight - radius);
            int generationCount = 0;
            

            // colliders
                Collider staticColliders[MAX_STATIC_COLLIDERS] = {
                    // Collider(Vec2(450.,600.), Vec2(450.,200)),
                    // Collider(Vec2(100.,400.), Vec2(750.,400)),
                    Collider(Vec2(420,floorHeight), Vec2(508,563)),
                    };
        
        vector<shared_ptr<Simulation>> simulations;
        float debug1;
        float debug2;
        // NeuralNetwork, GeneticAlgorithm
            vector<shared_ptr<NeuralNetwork>> neuralNetworks;
            vector<float> fitnesses;
            float mutationRate = 0.2f;
            float crossOverRate = 0.5f;
            float survivabilityModifier = 0.5f; // the percentage of the population that will survive to the next generation.

            int numOfGenerationsToTrain;
            int showEveryNthGeneration; // the n value in 'display simulations every nth generation'
            

    };

    struct GUIContext {
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

            int draw = true;
            int quit = false;

        // inputter declerations
            int inputterWidth = 324 - 20;
            int inputterYSpacing = 60;
            int inputterX = 1116 + 10;
            int inputterY = 50;
            Inputter inputters[MAX_INPUTTERS]; 
        // control panel misc declerations
            kiss_vscrollbar controlPanelScrollbar;
        // control panel start training declerations
            kiss_button buttonStartTrainingPopup;
            kiss_label labelStartTraining = {0};
            int buttonStartTrainingPopupY;
            kiss_entry trainForNGenerationsEntry = {0};
            kiss_entry showEveryNthGenerationEntry = {0};
            bool trainingPopupIsActive = false;
            kiss_button buttonStartTraining = {0};
            kiss_button buttonStopTraining = {0};
            kiss_label labelStopTraining = {0};

        // pop up
            bool popupIsActive = false;
            kiss_button popupButton = {0};
            kiss_label popupLabel = {0};
            SDL_Rect popupRect;

        // Info buttons
            InfoButton infoButtons[MAX_INFOBUTTONS];
        // misc
            int frameDelay = 0;
            int myCounter = 0;
            Uint32 ticksLastTime = SDL_GetTicks(); //the last recorded time
            Uint32 fpsCurrent; //the current FPS.
            Uint32 fpsFrameCount = 0; //frames passed since the last recorded fps
            kiss_label fpsLabel = {0};
            kiss_label targetPositionLabel = {0};
            kiss_label generationCountLabel = {0};
    

        // Constructor
        GUIContext() {
            // initialise random seed
                srand(time(0));
            {// initialise renderer, windows
                kiss_array_new(&objects);
                renderer = kiss_init("AI Controlled Manipulator Arm", &objects, 1440, 810);

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
                const char* nameArray[MAX_INPUTTERS] = {"Debug1 (-100 to 100)", "Debug2 (-50 to 150)", "Frame Delay (ms)", "Coef of Friction", "Gravity", "Coef of Restitution", "Arm Angle Constraints", "Radius", "Linkage Length", "Mutation Rate", "Crossover Split", "Survivability Modifier"};
                const float defaultArray[MAX_INPUTTERS] = {     0.0f,                    0.9f,            21.0f,                 0.9f,              1.2f,             0.7f,             M_PI/3,             30.0f,       90.0f,               0.2f,    0.5f,                 0.5f};
                const float rangeArray[MAX_INPUTTERS] = {       200.0f,                  4.0f,            700.0f,                2*0.1f,              10.0f,            2*0.7f,          2*M_PI/3,          2*30.0f,     2*90.0f,           2*0.2f,  2*0.5f,                1.0f};

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
                
                // Add start training gui elements
                    buttonStartTrainingPopupY = yPos -3;
                    kiss_label_new(&labelStartTraining, &panelControls, "Start Training", inputterX, buttonStartTrainingPopupY);
                    kiss_button_new(&buttonStartTrainingPopup, &panelControls, "Start", inputterX, kiss_textfont.fontheight+buttonStartTrainingPopupY);
                    kiss_button_new(&buttonStartTraining, &panelControls, "Train", kiss_screen_width/2 - 30, kiss_screen_height/2 + 120 - 2*kiss_textfont.fontheight);
                    yPos += inputterYSpacing;
                    //buttonStopTraining
                    kiss_label_new(&labelStopTraining, &panelControls, "Stop Training", inputterX, buttonStartTrainingPopupY);
                    kiss_button_new(&buttonStopTraining, &panelControls, "Stop", inputterX, kiss_textfont.fontheight+buttonStartTrainingPopupY);

                kiss_vscrollbar_new(&controlPanelScrollbar, &panelControls, 1440 - 20, 10, 810-10*2);
            }

            { // inistialise popup
                kiss_button_new(&popupButton, &window, "Okay", kiss_screen_width/2 - 30, kiss_screen_height/2 + 120);
                kiss_label_new(&popupLabel, &window, "My pop up\nnew line test", kiss_screen_width/2 - 180, kiss_screen_height/2 - 280);
            }

            { // initialise start training popup
                kiss_entry_new(&trainForNGenerationsEntry, &panelControls, 1, "0", kiss_screen_width/2 - 180 + 80, kiss_screen_height/2 - 280 + 2*kiss_textfont.fontheight -5, 100);
                kiss_entry_new(&showEveryNthGenerationEntry, &panelControls, 1, "0", kiss_screen_width/2 - 180 + 90, kiss_screen_height/2 - 280 + 4*kiss_textfont.fontheight -5, 100);
            }

            // Info buttons
                kiss_button_new(&infoButtons[0].button, &panelGraphs, "?", 60-7, 597);
                infoButtons[0].text = "This is the graph panel.\nIt hopes to give insight into\nthe fitness of neural networks\nwith respect to time.\nThe left shows the average";
                kiss_button_new(&infoButtons[1].button, &panelSimulation, "?", 100, 3);
                infoButtons[1].text = "The simulation panel displays the\nevaluation process of each neural network\nand allows the user to see training,\nvalues, objectives & fitness criteria\nin real time.";
                kiss_button_new(&infoButtons[2].button, &panelControls, "?", 1340, 3);
                infoButtons[2].text = "The control panel is where you are\nable to see all of these options.\nStart training from the top\nof this panel...";
        
            // MISC
                kiss_label_new(&fpsLabel, &window, "FPS: 0", 1040, 4);
                kiss_label_new(&targetPositionLabel, &window, "X", 0, 0);
                kiss_label_new(&generationCountLabel, &window, "Generation 0", 200, 4);
                
                

            {// set the windows visible
                window.visible = true;
                panelControls.visible = true;
                panelGraphs.visible = true;
                panelSimulation.visible = true;
            }
            
        }
    };

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
    // 0 --> p, q and r are collinear Perpendicular
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
        // // p1, q1 and p2 are collinear and p2 lies on segment p1q1
        // if (o1 == 0 && onSegment(p1, p2, q1)) return true;

        // // p1, q1 and q2 are collinear and q2 lies on segment p1q1
        // if (o2 == 0 && onSegment(p1, q2, q1)) return true;

        // // p2, q2 and p1 are collinear and p1 lies on segment p2q2
        // if (o3 == 0 && onSegment(p2, p1, q2)) return true;

        // // p2, q2 and q1 are collinear and q1 lies on segment p2q2
        // if (o4 == 0 && onSegment(p2, q1, q2)) return true;

        return false; // Doesn't fall in any of the above cases
    } // doesn't detect all parallel cases atm.
 //
// Simulation classes
    class Arm {
        private:
            SimulationContext &simulationContext;
            
        public:
            vector<float> angles; // 0 radians is directly up from the prev linkage. with y-axis down, clockwise is positive
            vector<float> angularVelocities; // with y-axis down, clockwise is positive
            vector<std::shared_ptr<Collider>> linkages;
            Collider endEffector = Collider(Vec2(558.0-simulationContext.linkageLength/2,591-simulationContext.numOfLinkages*simulationContext.linkageLength),Vec2(558+simulationContext.linkageLength/2.0,591-simulationContext.numOfLinkages*simulationContext.linkageLength));

        public:
            Arm(SimulationContext &mySimulationContext) : simulationContext(mySimulationContext) {
                for (int i=0; i<MAX_LINKAGES; i++) {
                    angles.emplace_back(0.0f);
                    angularVelocities.emplace_back(0.0f);
                    linkages.emplace_back(std::make_shared<Collider>(Collider(Vec2(simulationContext.armOriginPos.x, simulationContext.armOriginPos.y-i*simulationContext.linkageLength),Vec2(simulationContext.armOriginPos.x, simulationContext.armOriginPos.y-(i+1)*simulationContext.linkageLength))));
                }
                // endEffector = Collider(Vec2(558.0-simulationContext.linkageLength/2,591-simulationContext.numOfLinkages*simulationContext.linkageLength),Vec2(558+simulationContext.linkageLength/2.0,591-simulationContext.numOfLinkages*simulationContext.linkageLength));
                // test temporary displacement
                angles[simulationContext.numOfLinkages-1] += 0.2f; // remove later
            }
            void update() {
                // resolve gravity from base link to 2nd to last link
                    for (int i=0; i<simulationContext.numOfLinkages-1; i++) {
                        // apply the moment induced from this linkage's mass to all joints below this link
                        for (int j=i; j<simulationContext.numOfLinkages; j++) {
                            float centreOfGravity = (linkages[j]->startPos.x + linkages[j]->endPos.x)/2;
                            angularVelocities[i] += 0.0001*(centreOfGravity - linkages[i]->startPos.x)*simulationContext.gravity/simulationContext.linkageLength;
                        }
                    }
                // resolve top link gravity (bc of end effector)
                    float centreOfGravity = (linkages[simulationContext.numOfLinkages-1]->startPos.x + 3*linkages[simulationContext.numOfLinkages-1]->endPos.x)/4; // centre of gravity x position is 3/4 of the way along the top link because of the end effector.
                    // apply this moment to all joints as all joints are below this joint so will experience the force
                    // there is a multiple of 2 since this is the mass of the last link + the mass of the end effector
                    for (int i=0; i<simulationContext.numOfLinkages; i++) {
                        angularVelocities[i] += 2*0.0001*(centreOfGravity - linkages[i]->startPos.x)*simulationContext.gravity/simulationContext.linkageLength;
                    }
                    // apply offset moment to the top link to avoid it being reaaaaallly slow
                    angularVelocities[simulationContext.numOfLinkages-1] += 0.0001*(centreOfGravity - linkages[simulationContext.numOfLinkages-1]->startPos.x)*simulationContext.gravity/simulationContext.linkageLength;
                // apply friction, update and constrain angles based on angular velocities. Also update some positions for calculation purposes
                    Vec2 prevPos = simulationContext.armOriginPos; // initialised to ground attachment point. Used to calculate relative position of succeeding links.
                    float prevAngle = 0.0f; // initialised to base attachment angle. Used to calculate relative angle of succeeding links.
                    for (int i=0; i<simulationContext.numOfLinkages; i++) {
                        // apply friction damping
                        angularVelocities[i] *= simulationContext.coefOfFriction;
                        // update angles
                        angles[i] += angularVelocities[i];

                        // constrain angles
                        if (angles[i] > simulationContext.constraintAngle) { 
                            angles[i] = simulationContext.constraintAngle;
                            angularVelocities[i] = 0.0f;
                        } else if (angles[i] < -simulationContext.constraintAngle) {
                            angles[i] = -simulationContext.constraintAngle;
                            angularVelocities[i] = 0.0f;
                        }

                        // update some positions for calculation purposes
                        // skip calculating 'startPos's because we can just use the previous endPos. 
                        linkages[i]->endPos = prevPos + Vec2(simulationContext.linkageLength*sin(angles[i]+prevAngle), -simulationContext.linkageLength*cos(angles[i]+prevAngle)); // find new end
                        prevPos = linkages[i]->endPos; // store new end
                        prevAngle += angles[i];
                    }
                // constrain positions from base link to 2nd to last link
                    prevAngle = 0.0f; // initialised to base attachment angle. Used to calculate relative angle of succeeding links.
                    for (int i=0; i<simulationContext.numOfLinkages-1; i++) {
                        if (linkages[i]->endPos.y > simulationContext.floorHeight) {
                            float displacement = linkages[i]->endPos.x - linkages[0]->startPos.x; // find which side we are on
                            int direction = (displacement > 0) - (displacement < 0); // direction is 1 if we're on right side and -1 if we're on left side.
                            // calculate new angle such that the endPos is on the floor, not under.
                            angles[i] = direction*(M_PI - acos((simulationContext.floorHeight - linkages[i-1]->endPos.y) / simulationContext.linkageLength)) - prevAngle;
                            angularVelocities[i] = 0.0f;
                            // apply reaction force
                            for (int j = 0; j<i; j++) {
                                angularVelocities[j] *= - 0.5f;
                            }
                        }
                        prevAngle += angles[i];
                    }
                // constrain position for top link (bc of end effector)
                    // check if end effector is below floor
                    if (linkages[simulationContext.numOfLinkages-1]->endPos.y + abs(Perpendicular(linkages[simulationContext.numOfLinkages-1]->direction).y*simulationContext.linkageLength/2) > simulationContext.floorHeight) {
                        float displacement = linkages[simulationContext.numOfLinkages-1]->endPos.x - linkages[0]->startPos.x; // find which side we are on
                        int direction = (displacement > 0) - (displacement < 0); // direction is 1 if we're on right side and -1 if we're on left side.
                        // calculate new angle such that the endPos is on the floor, not under.
                        // I use linkages[simulationContext.numOfLinkages-2]->endPos.y as it is the y-value of the height above the floor. I make it relative to the simulation floor using floorHeight.
                        angles[simulationContext.numOfLinkages-1] = direction*(M_PI - acos((simulationContext.floorHeight - linkages[simulationContext.numOfLinkages-2]->endPos.y - abs(Perpendicular(linkages[simulationContext.numOfLinkages-1]->direction).y*simulationContext.linkageLength/2)) / simulationContext.linkageLength)) - prevAngle;
                        angularVelocities[simulationContext.numOfLinkages-1] = 0.0f;
                        // apply reaction force
                        for (int j = 0; j<simulationContext.numOfLinkages; j++) {
                            angularVelocities[j] *= - 0.99f;
                        }
                    }

                // update positions for real
                    prevPos = simulationContext.armOriginPos; // initialised to ground attachment point. Used to calculate relative position of succeeding links.
                    prevAngle = 0.0f; // initialised to base attachment angle. Used to calculate relative angle of succeeding links.
                    for (int i=0; i<simulationContext.numOfLinkages; i++) {
                        linkages[i]->startPos = prevPos; // set start to prev end
                        linkages[i]->endPos = prevPos + Vec2(simulationContext.linkageLength*sin(angles[i]+prevAngle), -simulationContext.linkageLength*cos(angles[i]+prevAngle)); // find new end
                        prevPos = linkages[i]->endPos;
                        prevAngle += angles[i];
                    }
                    endEffector.startPos = linkages[simulationContext.numOfLinkages-1]->endPos + Vec2(3*sin(prevAngle),-3*cos(prevAngle)) + Perpendicular(linkages[simulationContext.numOfLinkages-1]->direction)*simulationContext.linkageLength/2;
                    endEffector.endPos = linkages[simulationContext.numOfLinkages-1]->endPos   + Vec2(3*sin(prevAngle),-3*cos(prevAngle)) - Perpendicular(linkages[simulationContext.numOfLinkages-1]->direction)*simulationContext.linkageLength/2;
                updateColliders(); // updates directions for payload collision purposes
            }

            void updateColliders() {
                for (int i=0; i<simulationContext.numOfLinkages; i++) {
                    linkages[i]->direction = Normalise(linkages[i]->endPos - linkages[i]->startPos);
                }
                endEffector.direction = Normalise(endEffector.endPos - endEffector.startPos);
            }

        private:
        
    };

    class Payload {
        private:
            SimulationContext &simulationContext;
            Vec2 directionFromIntersectedCollider; // used for collision resolution
            float displacementIntoCollider; // used for collision resolution
            const int maxElectroMagnetStrength = 5; // the maximum strength of the electromagnet
        public:
            Vec2 prevPosition;
            Vec2 position = simulationContext.payloadSpawnPosition;
            Vec2 velocity;

        

        private:
            bool isColliding(Collider collider) {
                // condition: is behind
                if (pointToColliderDisplacement(position, collider) > simulationContext.radius) {return false;}
                // condition: centre path intersects collider
                Vec2 expandedStartPos = collider.startPos - collider.direction*simulationContext.radius;
                Vec2 expandedEndPos = collider.endPos + collider.direction*simulationContext.radius;
                if (lineSegmentsIntersect(prevPosition, position, expandedStartPos, expandedEndPos)) {return true;}
                // condition: centre to adjusted point path intersects collider
                Vec2 adjustedPayloadPos = position - Perpendicular(collider.direction)*simulationContext.radius;
                if (lineSegmentsIntersect(prevPosition, adjustedPayloadPos, expandedStartPos, expandedEndPos)) {return true;}
                return false;
            } // collider.direction must be a unit vector

            void ballToStaticColliderCollision(Collider collider) {
                float displacementIntoCollider = simulationContext.radius - pointToColliderDisplacement(position, collider);
                Vec2 normal = Perpendicular(collider.direction);
                position += normal*displacementIntoCollider; // Move to collider exterior surface.
                velocity += normal * -(simulationContext.coefOfRestitution +1)*(velocity*normal); // Resolve collision
            }

            bool isCollidingVer2(Collider collider) {
                // could instead find the closest point on the payloard to the collider and check if the adjusted circle path intersects collider.
                // can then use the direction to direct the collision rebound. this would create a capsule effect.
                // this would remove the need for the expanded collider path, thus fixing my collision jank issues.
                // the colliders would go from rectangles to capsules as they should.

                // how do I know which of the 3 values to use?
                // I shall retain most of the previous logic but replace the expanded end positions to the above method

                // the reason for the ordering being end point, middle section, start point is because the end point needs priority for when touching the the end effector.

                Vec2 directionToColliderEndPos = Normalise(collider.endPos - position);
                Vec2 adjustedPayloadPos = position + directionToColliderEndPos*simulationContext.radius;
                if (lineSegmentsIntersect(prevPosition, adjustedPayloadPos, collider.endPos, collider.startPos)) {
                    // also have to return the direction of the payload centre to the closest point on collider if it is colliding.
                    directionFromIntersectedCollider = - directionToColliderEndPos;
                    displacementIntoCollider = LengthOfVector(collider.endPos - adjustedPayloadPos);
                    return true;
                }

                // condition: adjusted point path intersects collider or centre path intersects collider
                adjustedPayloadPos = position - Perpendicular(collider.direction)*simulationContext.radius;
                if (lineSegmentsIntersect(prevPosition, adjustedPayloadPos, collider.startPos, collider.endPos)
                    || lineSegmentsIntersect(prevPosition, position, collider.startPos, collider.endPos)) {
                    directionFromIntersectedCollider = Perpendicular(collider.direction);
                    displacementIntoCollider = simulationContext.radius - pointToColliderDisplacement(position, collider);
                    return true;
                }
                
                // condition: adjusted point path intersects collider ends
                Vec2 directionToColliderStartPos = Normalise(collider.startPos - position);
                adjustedPayloadPos = position + directionToColliderStartPos*simulationContext.radius;
                if (lineSegmentsIntersect(prevPosition, adjustedPayloadPos, collider.endPos, collider.startPos)) {
                    // also have to return the direction of the payload centre to the closest point on collider if it is colliding.
                    directionFromIntersectedCollider = - directionToColliderStartPos;
                    displacementIntoCollider = LengthOfVector(collider.startPos - adjustedPayloadPos);
                    return true;
                }

                return false;
            } // collider.direction must be a unit vector

            void ballToStaticColliderCollisionVer2(Collider collider) {
                Vec2 normal = directionFromIntersectedCollider;
                position += normal*displacementIntoCollider; // Move to collider exterior surface.
                // cout << "Normal: " << normal.x << ", " << normal.y << endl;
                velocity += normal * -(simulationContext.coefOfRestitution +1)*(velocity*normal); // Resolve collision
            }
        public:
            Payload(SimulationContext &mySimulationContext) : simulationContext(mySimulationContext) {
                position.x= simulationContext.radius + 100;
                position.y= simulationContext.floorHeight - simulationContext.radius;
            }
        
            void update(Arm &arm) {
                    // Start Semi-Implicit Euler Integration
                        prevPosition = position;
                        velocity.y += simulationContext.gravity;
                    // Apply electromagnet force proportional to end effector displacement
                        Vec2 displacementToEndEffector = arm.linkages[simulationContext.numOfLinkages-1]->endPos - position;
                        float distance = LengthOfVector(displacementToEndEffector);
                        // use tanh() although physically inaccurate to clamp min dist and max strength
                        velocity += Normalise(displacementToEndEffector) * maxElectroMagnetStrength * (-optimisedTanh((distance-simulationContext.radius-25)/5) + 1) / 2;
                    // Finish Semi-Implicit Euler Integration
                        position += velocity;
                    
                    // Floor collision
                    if (position.y + simulationContext.radius > simulationContext.floorHeight) { // If below floor
                        position.y = simulationContext.floorHeight - simulationContext.radius; // Set pos to on floor
                        velocity.y *= -simulationContext.coefOfRestitution; // Resolve collision
                        // friction
                        if (abs(velocity.x) > simulationContext.coefOfFriction*0.07f) {
                            velocity.x += -simulationContext.coefOfFriction*0.07f*velocity.x; 
                        } else {velocity.x = 0.0f;}
                    }
                    // Left wall collision
                    if (position.x < simulationContext.radius) { // If left of left wall
                        position.x = simulationContext.radius; // Set pos to left wall
                        velocity.x *= -simulationContext.coefOfRestitution; // Resolve collision
                    }
                    // Ceiling collision
                    if (position.y < simulationContext.radius) { // If above ceiling
                        position.y = simulationContext.radius; // Set pos to ceiling
                        velocity.y *= -simulationContext.coefOfRestitution; // Resolve collision
                    }
                    // Right wall collision
                    if (position.x > 1116 - simulationContext.radius) { // If right of right wall
                        position.x = 1116 - simulationContext.radius; // Set pos to right wall
                        velocity.x *= -simulationContext.coefOfRestitution; // Resolve collision
                    }
                    for(int i = 0; i<MAX_STATIC_COLLIDERS; i++) {
                        if (isCollidingVer2(simulationContext.staticColliders[i])) {
                            ballToStaticColliderCollisionVer2(simulationContext.staticColliders[i]);
                        }
                    }
                    // arm collisions. arm collider directions have already been updated.
                    for (int i=0; i<simulationContext.numOfLinkages; i++) {
                        if (isCollidingVer2(*arm.linkages[i])) {
                            ballToStaticColliderCollisionVer2(*arm.linkages[i]);
                        }
                    }
                    // end effector collision
                    if (isCollidingVer2(arm.endEffector)) {
                            ballToStaticColliderCollisionVer2(arm.endEffector);
                            // friction
                            velocity = velocity*simulationContext.coefOfFriction;
                    }

                    // end effector collision
            }
        
    };

    class Simulation {
        private:
            SimulationContext &simulationContext;
        public:
            Arm arm;
            Payload payload;

        public:
            Simulation(SimulationContext &mySimulationContext) : simulationContext(mySimulationContext), arm(mySimulationContext), payload(mySimulationContext) {}
            void draw_simulation_objects(SDL_Renderer *renderer) {
                // set draw colour to black
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                // Draw payload
                DrawCircle(renderer, payload.position.x, payload.position.y, simulationContext.radius);
                // Draw arm
                for (int i=0; i<simulationContext.numOfLinkages; i++) {
                    SDL_RenderDrawLine(renderer, arm.linkages[i]->startPos.x,arm.linkages[i]->startPos.y,arm.linkages[i]->endPos.x,arm.linkages[i]->endPos.y);
                    // code to show expanded collider for collision detection for debugging
                    // Vec2 expandedStartPos = arm.linkages[i]->startPos - arm.linkages[i]->direction*simulationContext.radius;
                    // Vec2 expandedEndPos = arm.linkages[i]->endPos + arm.linkages[i]->direction*simulationContext.radius;
                    // SDL_RenderDrawLine(renderer, expandedStartPos.x,expandedStartPos.y,expandedEndPos.x,expandedEndPos.y);
                }
                // Draw end effector
                SDL_RenderDrawLine(renderer, arm.endEffector.startPos.x,arm.endEffector.startPos.y,arm.endEffector.endPos.x,arm.endEffector.endPos.y);
            }

        //     void reset() {
        //         // Reset arm, payload, and other simulation variables
        //         arm.reset();
        //         payload.reset();
        //     }

        //     void apply_controls(const vector<float> &controls) {
        //         // Use controls (e.g., motor torques) to update arm state
        //         arm.apply_controls(controls);
        //     }

            void advance() {
                payload.update(arm);
                arm.update();
                // if training repeat for xyz frames, if displaying then end
            }

            void reset() {
                // Reset arm, payload, and other simulation variables
                    // arm
                        // reset angles and angular velocities
                        for (int i=0; i<simulationContext.numOfLinkages; i++) {
                            arm.angles[i] = 0.0f;
                            arm.angularVelocities[i] = 0.0f;
                        }
                    // payload
                        // reset position and velocity
                        payload.position = simulationContext.payloadSpawnPosition;
                        payload.velocity = Vec2(0.0f, 0.0f);
            }


        //     float calculate_fitness() const {
        //         // Compute fitness based on simulation goals (e.g., payload position, energy usage)
        //         return payload.get_position().distance_to(targetPosition);
        //     }

        //     vector<float> get_state() const {
        //         // Return current state variables, e.g., joint angles, velocities
        //         return arm.get_state();
        //     }

    };
// Neural Network Management
    void applyNeuralNetworkOutputsToSimulations(SimulationContext &simulationContext, int instancesToAdvance) {
        for (int i=0; i<instancesToAdvance; i++) {
            // find inputs:
            // end effector-payload displacement
            // payload-targetPos displacement.
            // payload velo
            // angles & velos for each joint
            Vec2 payloadDisplacementToEndEffector = simulationContext.simulations[i]->arm.linkages[simulationContext.numOfLinkages-1]->endPos - simulationContext.simulations[i]->payload.position;
            Vec2 payloadDisplacementToTarget = Vec2(1000.0f, simulationContext.floorHeight) - simulationContext.simulations[i]->payload.position;
            vector<float> inputs = {payloadDisplacementToEndEffector.x, payloadDisplacementToEndEffector.y,
                                    payloadDisplacementToTarget.x, payloadDisplacementToTarget.y,
                                    simulationContext.simulations[i]->payload.velocity.x, simulationContext.simulations[i]->payload.velocity.y};
            for (int j=0; j<5; j++) {
                inputs.push_back(simulationContext.simulations[i]->arm.angles[j]);
                inputs.push_back(simulationContext.simulations[i]->arm.angularVelocities[j]);
            }            
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << "inputs " << i << ":" << endl;
            // for (int j=0; j<16; j++) {
            //     cout << inputs[j] << endl;
            // }

            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // calculate outputs
            vector<float> outputs = simulationContext.neuralNetworks[i]->CalculateOutputs(inputs);
            // cout << "outputs " << i << ":" << endl;
            // // restrict and round outputs before displaying
            // for (int j=0; j<5; j++) {
            //     cout << std::floor(outputs[j]*10000)/10000 << endl;
            //     outputs[j] *= 0.01;
            // }

            // apply force outputs
            for (int j=0; j<simulationContext.numOfLinkages; j++) {
                // apply force
                simulationContext.simulations[i]->arm.angularVelocities[j] += optimisedTanh(outputs[j])*0.01;
            }
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // cout << endl;
        }
    }

    // Sort neural networks by fitness in descending order
        // void sort_by_fitness( // deprecated
        //     vector<std::shared_ptr<NeuralNetwork>>& neuralNetworks,
        //     vector<std::shared_ptr<float>>& fitnesses) {
        //     auto comparator = [&](size_t i, size_t j) {
        //         return *fitnesses[i] > *fitnesses[j];
        //     };
        //     vector<size_t> indices(neuralNetworks.size());
        //     for (size_t i = 0; i < indices.size(); ++i) {
        //         indices[i] = i;
        //     }
        //     std::sort(indices.begin(), indices.end(), comparator);
        //     vector<std::shared_ptr<NeuralNetwork>> sortedNetworks;
        //     vector<std::shared_ptr<float>> sortedFitnesses;
        //     for (size_t i : indices) {
        //         sortedNetworks.push_back(neuralNetworks[i]);
        //         sortedFitnesses.push_back(fitnesses[i]);
        //     }
        //     neuralNetworks = sortedNetworks;
        //     fitnesses = sortedFitnesses;
        // }
    void sort_by_fitness(std::vector<std::shared_ptr<NeuralNetwork>>& neuralNetworks, std::vector<float>& fitnesses) {
        std::vector<size_t> indices(neuralNetworks.size());
        for (size_t i = 0; i < indices.size(); ++i) {
            indices[i] = i;
        }

        auto comparator = [&](size_t i, size_t j) {
            return fitnesses[i] > fitnesses[j];
        };

        std::sort(indices.begin(), indices.end(), comparator);

        std::vector<std::shared_ptr<NeuralNetwork>> sortedNetworks;
        std::vector<float> sortedFitnesses;

        for (size_t i : indices) {
            sortedNetworks.push_back(neuralNetworks[i]);
            sortedFitnesses.push_back(fitnesses[i]);
        }

        neuralNetworks = sortedNetworks;
        fitnesses = sortedFitnesses;
        }


    // Perform crossover to create a child network
    std::shared_ptr<NeuralNetwork> crossover(const std::shared_ptr<NeuralNetwork> &parent1, const std::shared_ptr<NeuralNetwork> &parent2, const float &crossOverRate) {
        std::shared_ptr<NeuralNetwork> child = std::make_shared<NeuralNetwork>(*parent1); // Use deep copy constructor
        for (size_t l = 0; l < parent1->layers.size(); ++l) {
            auto& layer1 = parent1->layers[l];
            auto& layer2 = parent2->layers[l];
            auto& childLayer = child->layers[l];

            for (size_t i = 0; i < layer1->weights.size(); ++i) {
                for (size_t j = 0; j < layer1->weights[i].size(); ++j) {
                    if ( (float)rand() / RAND_MAX < crossOverRate ) {
                        childLayer->weights[i][j] = layer1->weights[i][j];
                    } else {
                        childLayer->weights[i][j] = layer2->weights[i][j];
                    }
                }
                if ( (float)rand() / RAND_MAX < crossOverRate ) {
                    childLayer->biases[i] = layer1->biases[i];
                } else {
                    childLayer->biases[i] = layer2->biases[i];
                }
            }
        }
        return child;
    }
    // Select a random parent from the population
    std::shared_ptr<NeuralNetwork> select_parent(const vector<std::shared_ptr<NeuralNetwork>> &population) {
        size_t index = rand() % population.size();
        return population[index];
    }
    // Mutate a child network
    void mutate(std::shared_ptr<NeuralNetwork> &child, SimulationContext &simulationContext) {
        for (auto& layer : child->layers) {
            for (size_t i = 0; i < layer->weights.size(); ++i) {
                for (size_t j = 0; j < layer->weights[i].size(); ++j) {
                    if ((float)rand() / RAND_MAX < simulationContext.mutationRate) {
                        // randomly mutate in one of these 3 ways: 1.5x, 0.5x, random value between -2 and 2
                        int mutationType = rand() % 3;
                        if (mutationType == 0) {
                            layer->weights[i][j] *= 1.5;
                        } else if (mutationType == 1) {
                            layer->weights[i][j] *= 0.5;
                        } else {
                            layer->weights[i][j] = ((rand() % 4000 - 2000) / 1000.0f);
                        }
                    }
                }
                if ((float)rand() / RAND_MAX < simulationContext.mutationRate) {
                    // randomly mutate in one of these 3 ways: 1.5x, 0.5x, random value between -1 and 1
                    int mutationType = rand() % 3;
                    if (mutationType == 0) {
                        layer->biases[i] *= 1.5;
                    } else if (mutationType == 1) {
                        layer->biases[i] *= 0.5;
                    } else {
                        layer->biases[i] = ((rand() % 2000 - 1000) / 1000.0f);
                    }
                }
            }
        }
    }
    // Check if an individual survives based on its survival chance
    bool survived(float survivalChance) {
        return ((float)rand() / RAND_MAX) < survivalChance;
    }
    // Evolve the population
    void evolve(SimulationContext &simulationContext) {
        // debug print
            // cout << "before sorting" << endl;
            // for (int i = 0; i < 1000; ++i) {
            //     cout << simulationContext.fitnesses[i] << endl;
            // } 
        sort_by_fitness(simulationContext.neuralNetworks, simulationContext.fitnesses);
        // print fitnesses
            // cout << "after sorting" << endl;
            // for (int i = 0; i < 1000; ++i) {
                // cout << simulationContext.fitnesses[i] << endl;
            // } 
        vector<std::shared_ptr<NeuralNetwork>> newPopulation;
        size_t populationSize = simulationContext.neuralNetworks.size();

        // Survival step
        for (size_t i = 0; i < populationSize; ++i) {
            float survivalChance = 0.5f - (float)i / (populationSize) + simulationContext.survivabilityModifier;
            if (survived(survivalChance)) {
                newPopulation.push_back(simulationContext.neuralNetworks[i]);
            }
        }

        // If none survived, place the best individual back into the population
        if (newPopulation.empty()) {
            newPopulation.push_back(simulationContext.neuralNetworks[0]);
        }

        // Reproduction step
        while (newPopulation.size() < populationSize) {
            std::shared_ptr<NeuralNetwork> parent1 = select_parent(newPopulation);
            std::shared_ptr<NeuralNetwork> parent2 = select_parent(newPopulation);
            std::shared_ptr<NeuralNetwork> newChild = crossover(parent1, parent2, simulationContext.crossOverRate);
            mutate(newChild, simulationContext);
            newPopulation.push_back(newChild);

            // std::shared_ptr<NeuralNetwork>  newChild = select_parent(simulationContext.neuralNetworks);
            // mutate(newChild, simulationContext);
            // newPopulation.push_back(newChild);

            // newPopulation.push_back(select_parent(newPopulation));
        }

        simulationContext.neuralNetworks = newPopulation;
    }


// Event handlers
    void button_event(kiss_button &button, SDL_Event &e, int &draw, int &quit, int &myCounter) {
        if (kiss_button_event(&button, &e, &draw)) {
            myCounter+=1;
            string newText = to_string(myCounter);
            snprintf(button.text, sizeof(button.text), "%s", newText.c_str());
            strcpy(button.text, (std::to_string(myCounter)).c_str());
        }
    }

    void slider_event(Inputter &inputter, SDL_Event &e, int &draw) {
        if (kiss_hscrollbar_event(&inputter.slider, &e, &draw)) {
            // set the entry->text equal to the slider's value
            float value = (inputter.slider.fraction - 0.5f) * inputter.sliderRange + inputter.defaultValue; // calculates the slider's value
            char text[16];
            snprintf(text, sizeof(text), "%.2f", value);
            strcpy(inputter.entry.text, text);
            draw = true;
        }
    }

    void control_panel_slider_event(GUIContext &guiContext) { //this passes the dereferenced inputters array and it works but not sure why.
        if (kiss_vscrollbar_event(&guiContext.controlPanelScrollbar, &guiContext.e, &guiContext.draw)) {
            float multiplier = 400.0f;
            float amountScrolled = guiContext.controlPanelScrollbar.fraction; 
            float maxCanScroll = 1.0f;
            float ratio = amountScrolled / maxCanScroll;
            int controlPanelYOffSet = static_cast<int>(round(multiplier*(1.0f-ratio)));
            for (int i = 0; i< MAX_INPUTTERS; i++) {
                guiContext.inputters[i].entry.rect.y = guiContext.inputters[i].y + controlPanelYOffSet - multiplier;
                guiContext.inputters[i].entry.texty = guiContext.inputters[i].y + controlPanelYOffSet - multiplier + 6;

                guiContext.inputters[i].slider.leftrect.y = guiContext.inputters[i].y + controlPanelYOffSet - multiplier + 6;
                guiContext.inputters[i].slider.rightrect.y = guiContext.inputters[i].y + controlPanelYOffSet - multiplier + 6;
                guiContext.inputters[i].slider.sliderrect.y = guiContext.inputters[i].y + controlPanelYOffSet - multiplier + 6;

                guiContext.inputters[i].label.rect.y = guiContext.inputters[i].y + controlPanelYOffSet - multiplier - 10;
            }
            // move start training button
            guiContext.buttonStartTrainingPopup.rect.y = guiContext.buttonStartTrainingPopupY + controlPanelYOffSet - multiplier + 6;
            guiContext.buttonStartTrainingPopup.texty = guiContext.buttonStartTrainingPopupY + controlPanelYOffSet - multiplier + 6 + 2;
            guiContext.labelStartTraining.rect.y = guiContext.buttonStartTrainingPopupY + controlPanelYOffSet - multiplier - 10;

            guiContext.buttonStopTraining.rect.y = guiContext.buttonStartTrainingPopupY + controlPanelYOffSet - multiplier + 6;
            guiContext.buttonStopTraining.texty = guiContext.buttonStartTrainingPopupY + controlPanelYOffSet - multiplier + 6 + 2;
            guiContext.labelStopTraining.rect.y = guiContext.buttonStartTrainingPopupY + controlPanelYOffSet - multiplier - 10;

            guiContext.draw = true;
        }
    }

    void entry_event(Inputter &inputter, SDL_Event &e, int &draw) {
        if (kiss_entry_event(&inputter.entry, &e, &draw)) {
            // set the slider's value equal to the entry.text 
            float value = atof(inputter.entry.text); //string to float cast
            if (value >= inputter.defaultValue + (inputter.sliderRange / 2.0f)) {
                inputter.slider.fraction = 1.03f;
            } else if (value <= inputter.defaultValue - (inputter.sliderRange / 2.0f)) {
                inputter.slider.fraction = -0.03f;
            } else {
                inputter.slider.fraction = (value - inputter.defaultValue)/inputter.sliderRange + 0.5f; // calculates the new slider's pos
            }
            draw = true;
        }
    }

    void disable_popup(GUIContext &guiContext) {
        guiContext.popupIsActive = false;
        guiContext.trainingPopupIsActive = false; // if its showing the training popup
        // reset popup text to "Okay" in case the training popup changed it
        string newText = "Okay";
        snprintf(guiContext.popupButton.text, sizeof(guiContext.popupButton.text), "%s", newText.c_str());
        guiContext.popupButton.textx = 707;
        guiContext.draw = true;
    }

    void popup_event(GUIContext &guiContext) { // Occurs when "Okay"/"Cancel" button is pressed on the popup
        if (kiss_button_event(&guiContext.popupButton, &guiContext.e, &guiContext.draw)) {
            disable_popup(guiContext);
        }
    }

    void show_popup(string message, bool &popupIsActive, kiss_label &popupLabel) {
        popupIsActive = true;
        snprintf(popupLabel.text, sizeof(popupLabel.text), "%s", message.c_str());
    }

    void infoButton_event(InfoButton &infoButton, bool &popupIsActive, kiss_label &popupLabel, SDL_Event &e, int &draw) {
        if (kiss_button_event(&infoButton.button, &e, &draw)) {
            show_popup(infoButton.text, popupIsActive, popupLabel);
        }
    }

    void start_training_popup_button_event(GUIContext &guiContext) {
        if (kiss_button_event(&guiContext.buttonStartTrainingPopup, &guiContext.e, &guiContext.draw)) {
            // show popup
            show_popup("Please enter the training options:\n\nTrain for           generations.\n\nShow every           th generation.\n\n-1 to not show anything.\n\n\nEach generation takes the simulation\n20 seconds real time or max 0.5 seconds\ntraining time.\n\nThere will be a button to stop the\ntraining session early.", guiContext.popupIsActive, guiContext.popupLabel);
            // change popup "Okay" button text to "Cancel"
            string newText = "Cancel";
            snprintf(guiContext.popupButton.text, sizeof(guiContext.popupButton.text), "%s", newText.c_str());
            guiContext.popupButton.textx = 707 - 7;
            // show the entry boxes & train button
            guiContext.trainingPopupIsActive = true;
        }
    }

    void start_training_confirm_button_event(GUIContext &guiContext, SimulationContext &simulationContext) {
        if (kiss_button_event(&guiContext.buttonStartTraining, &guiContext.e, &guiContext.draw)) {
            simulationContext.numOfGenerationsToTrain = atoi(guiContext.trainForNGenerationsEntry.text);
            simulationContext.showEveryNthGeneration = atoi(guiContext.showEveryNthGenerationEntry.text);
            disable_popup(guiContext);
            simulationContext.isTraining = true; // can't call training_loop() from here as we'd get circular dependency
        }
    }

    void stop_training_button_event(GUIContext &guiContext, SimulationContext &simulationContext) {
        if (kiss_button_event(&guiContext.buttonStopTraining, &guiContext.e, &guiContext.draw)) {
            simulationContext.numOfGenerationsToTrain = 0;
            simulationContext.isTraining = false; 
        }
    }

// Main loop function
    void check_if_program_quitted(GUIContext &guiContext) {
        if (guiContext.quit) {
            kiss_clean(&guiContext.objects);
            exit(0);
        }
    }
    void handle_fps(GUIContext &guiContext) {
        // Calculate FPS
            guiContext.fpsFrameCount++;
            if (guiContext.ticksLastTime < SDL_GetTicks() - 1000)
            {
                guiContext.ticksLastTime = SDL_GetTicks();
                guiContext.fpsCurrent = guiContext.fpsFrameCount;
                guiContext.fpsFrameCount = 0;
                sprintf(guiContext.fpsLabel.text, "FPS: %d", guiContext.fpsCurrent);
            }

        // Frame delay    
            SDL_Delay(guiContext.frameDelay);
    }
    void handle_events_and_inputs(GUIContext &guiContext, SimulationContext &simulationContext) {
        while (SDL_PollEvent(&guiContext.e)) { // Inputs, Events
            if (guiContext.e.type == SDL_QUIT) guiContext.quit = true;

            kiss_window_event(&guiContext.window, &guiContext.e, &guiContext.draw);
            
            if (guiContext.popupIsActive) {
                popup_event(guiContext);
                //break;
            }

            for(int i =0; i<MAX_INPUTTERS; i++) {
                slider_event(guiContext.inputters[i], guiContext.e, guiContext.draw);
                entry_event(guiContext.inputters[i], guiContext.e, guiContext.draw);
            }
            control_panel_slider_event(guiContext);
            // start/stop training popup events
                if (simulationContext.isTraining) {
                    stop_training_button_event(guiContext, simulationContext);
                } else {
                    start_training_popup_button_event(guiContext);
                }
                if (guiContext.trainingPopupIsActive) {
                    // trainForNGenerationsEntry, showEveryNthGenerationEntry events
                    kiss_entry_event(&guiContext.trainForNGenerationsEntry, &guiContext.e, &guiContext.draw);
                    kiss_entry_event(&guiContext.showEveryNthGenerationEntry, &guiContext.e, &guiContext.draw);
                    start_training_confirm_button_event(guiContext, simulationContext);
                }

            // info button events
                for (int i = 0; i<MAX_INFOBUTTONS; i++) {
                    infoButton_event(guiContext.infoButtons[i], guiContext.popupIsActive, guiContext.popupLabel, guiContext.e, guiContext.draw);
                }
            
            // Inputs
            guiContext.frameDelay = abs(atoi(guiContext.inputters[2].entry.text));
            // if (guiContext.e.type == SDL_KEYDOWN) { //input detections
                // switch(guiContext.e.key.keysym.sym) {
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
            // Update simulation controls
                simulationContext.debug1 = atof(guiContext.inputters[0].entry.text);
                simulationContext.debug2 = atof(guiContext.inputters[1].entry.text);
                simulationContext.coefOfFriction = atof(guiContext.inputters[3].entry.text);
                simulationContext.gravity = atof(guiContext.inputters[4].entry.text);
                simulationContext.coefOfRestitution = atof(guiContext.inputters[5].entry.text);
                simulationContext.constraintAngle = abs(atof(guiContext.inputters[6].entry.text));
                simulationContext.radius = abs(atoi(guiContext.inputters[7].entry.text));
                simulationContext.linkageLength = abs(atoi(guiContext.inputters[8].entry.text))+1;
                simulationContext.mutationRate = atof(guiContext.inputters[9].entry.text);
                simulationContext.crossOverRate = atof(guiContext.inputters[10].entry.text);
                simulationContext.survivabilityModifier = atof(guiContext.inputters[11].entry.text);

                // move payload to clicked pos
                if (guiContext.e.type == SDL_MOUSEBUTTONDOWN && simulationContext.isTraining == false) {
                    if (guiContext.e.button.button == SDL_BUTTON_LEFT && guiContext.e.button.x < 1116 && guiContext.e.button.y < 594) {
                        simulationContext.simulations[0]->payload.position = Vec2(guiContext.e.button.x,guiContext.e.button.y);
                        simulationContext.simulations[0]->payload.velocity = Vec2(0,0);
                        
                    }
                }
        }
    }
    void draw_gui_beneath_simulation(GUIContext &guiContext) {
        SDL_RenderClear(guiContext.renderer);
        kiss_window_draw(&guiContext.window, guiContext.renderer);
        kiss_window_draw(&guiContext.panelSimulation, guiContext.renderer);
    }
    void draw_gui(GUIContext &guiContext, SimulationContext &simulationContext) {
        // Draw windows, misc sdl
            // these 2 windows get drawn before the simulation so they are behind it.
                // kiss_window_draw(&guiContext.window, guiContext.renderer);
                // kiss_window_draw(&guiContext.panelSimulation, guiContext.renderer);
            kiss_window_draw(&guiContext.panelGraphs, guiContext.renderer);
            kiss_window_draw(&guiContext.panelControls, guiContext.renderer);
            //kiss_button_draw(&guiContext.button, guiContext.renderer);


        for(int i =0; i<MAX_INPUTTERS; i++) {
            kiss_hscrollbar_draw(&guiContext.inputters[i].slider, guiContext.renderer);
            kiss_entry_draw(&guiContext.inputters[i].entry, guiContext.renderer);
            kiss_label_draw(&guiContext.inputters[i].label, guiContext.renderer);
        }
        kiss_vscrollbar_draw(&guiContext.controlPanelScrollbar, guiContext.renderer);
        // Draw colliders
            // set draw colour black
            SDL_SetRenderDrawColor(guiContext.renderer, 0, 0, 0, 255);
            for(int i =0; i<MAX_STATIC_COLLIDERS; i++) { 
                SDL_RenderDrawLine(guiContext.renderer, simulationContext.staticColliders[i].startPos.x, simulationContext.staticColliders[i].startPos.y, simulationContext.staticColliders[i].endPos.x, simulationContext.staticColliders[i].endPos.y);
            }
        // Draw other collider lines
            SDL_RenderDrawLine(guiContext.renderer, 508, simulationContext.floorHeight, 508, 563);
        // Draw window titles
            kiss_makerect(&guiContext.panelControlsTitleRect, 1220, 4, 120, 15);
            kiss_fillrect(guiContext.renderer, &guiContext.panelControlsTitleRect, kiss_white);
            kiss_label_draw(&guiContext.panelControlsTitle, guiContext.renderer);
            kiss_label_draw(&guiContext.panelSimulationTitle, guiContext.renderer);
            kiss_label_draw(&guiContext.panelGraphsTitle, guiContext.renderer);

        // Draw info buttons
            for (int i =0; i<MAX_INFOBUTTONS; i++) {
                kiss_button_draw(&guiContext.infoButtons[i].button, guiContext.renderer);
            }
        // Draw pop up
            if (guiContext.popupIsActive) {
                kiss_makerect(&guiContext.popupRect, kiss_screen_width/2 - 200, kiss_screen_height/2 - 300, 400, 460);
                kiss_fillrect(guiContext.renderer, &guiContext.popupRect, kiss_lightblue);
                kiss_decorate(guiContext.renderer, &guiContext.popupRect, kiss_blue, kiss_edge);
                kiss_label_draw(&guiContext.popupLabel, guiContext.renderer);
                kiss_button_draw(&guiContext.popupButton, guiContext.renderer);
            }
        // Draw start/stop training button and popup
            if (simulationContext.isTraining) {
                kiss_button_draw(&guiContext.buttonStopTraining, guiContext.renderer);
                kiss_label_draw(&guiContext.labelStopTraining, guiContext.renderer);
            } else {
                kiss_button_draw(&guiContext.buttonStartTrainingPopup, guiContext.renderer);
                kiss_label_draw(&guiContext.labelStartTraining, guiContext.renderer);
            }
            if (guiContext.trainingPopupIsActive) {
                // draw trainForNGenerationsEntry, showEveryNthGenerationEntry, buttonStartTraining
                kiss_entry_draw(&guiContext.trainForNGenerationsEntry, guiContext.renderer);
                kiss_entry_draw(&guiContext.showEveryNthGenerationEntry, guiContext.renderer);
                kiss_button_draw(&guiContext.buttonStartTraining, guiContext.renderer);
            } 
        // Draw FPS counter
            kiss_label_draw(&guiContext.fpsLabel, guiContext.renderer);
        // Draw Target Position marker
            kiss_label_draw(&guiContext.targetPositionLabel, guiContext.renderer);
        // Draw generation counter label
            kiss_label_draw(&guiContext.generationCountLabel, guiContext.renderer);
        
        SDL_RenderPresent(guiContext.renderer);
        guiContext.draw = false;
    }

// Driver code loops
void display_generation_loop(GUIContext &guiContext, SimulationContext &simulationContext) {
    for (int i=0; i<simulationContext.maxTrainingFrames; i++) {
        handle_fps(guiContext);
        draw_gui_beneath_simulation(guiContext);
        // set colour black
        SDL_SetRenderDrawColor(guiContext.renderer, 0, 0, 0, 255);
        for (int j=0; j<simulationContext.numInstances; j++) {
            simulationContext.simulations[j]->advance();
            // simulation->calculateFitness();

            // show all agents
            if ((j%1)==0) {    simulationContext.simulations[j]->draw_simulation_objects(guiContext.renderer);    }
            // show every 200th agent
            // if ((j%50)==0) {    simulationContext.simulations[j]->draw_simulation_objects(guiContext.renderer);    }
        }
        applyNeuralNetworkOutputsToSimulations(simulationContext, simulationContext.numInstances);
        handle_events_and_inputs(guiContext, simulationContext);
        check_if_program_quitted(guiContext);
        draw_gui(guiContext, simulationContext);
    }
}

void train1GenerationASAP(SimulationContext &simulationContext) {
    for (int i=0; i<simulationContext.maxTrainingFrames; i++) {
        for (const auto &simulation : simulationContext.simulations) {
            simulation->advance();
            // simulation->calculateFitness();
        }
        applyNeuralNetworkOutputsToSimulations(simulationContext, simulationContext.numInstances);
    }
}

void training_loop(GUIContext &guiContext, SimulationContext &simulationContext, int i) {
    // update the generation counter
        simulationContext.generationCount += 1;
        string newText = "Generation " + to_string(simulationContext.generationCount);
        snprintf(guiContext.generationCountLabel.text, sizeof(guiContext.generationCountLabel.text), "%s", newText.c_str());
    // reset all simulations
        for (const auto &simulation : simulationContext.simulations) {
            simulation->reset();
        }
    // reset fitnesses
        for (int i=0; i<simulationContext.numInstances; i++) {
            simulationContext.fitnesses[i] = 0.0f;
        }
    // draw gui beneath so we clear the frame from the popup.
        draw_gui_beneath_simulation(guiContext);

    // show every nth generation fully. Otherwise, train ASAP.
        if (((i % simulationContext.showEveryNthGeneration) == 0) && (simulationContext.showEveryNthGeneration != -1)) {
            display_generation_loop(guiContext, simulationContext);
        } else {
            train1GenerationASAP(simulationContext);
        }

    // temp reward functions
        for (int i=0; i<simulationContext.numInstances; i++) {

            // optimal solution: end effector close to ball
            // Vec2 dist = simulationContext.simulations[i]->arm.linkages[simulationContext.numOfLinkages-1]->endPos - simulationContext.simulations[i]->payload.position;
            // simulationContext.fitnesses[i] = - dist.magnitude_squared();

            // optimal solution: scrunched up clockwise
            // for (int j=0; j<simulationContext.numOfLinkages; j++) {
            //     simulationContext.fitnesses[i] += simulationContext.simulations[i]->arm.angles[j];
            // }
            if (simulationContext.debug1 < 300) {
                // optimal solution: end effector close to ball and ball high up
                Vec2 dist = simulationContext.simulations[i]->arm.linkages[simulationContext.numOfLinkages-1]->endPos - simulationContext.simulations[i]->payload.position;
                simulationContext.fitnesses[i] = - LengthOfVector(dist) - simulationContext.simulations[i]->payload.position.y;
            } else {
                // optimal solution: ball close to target
                Vec2 payloadDisplacementToTarget = simulationContext.targetPosition - simulationContext.simulations[i]->payload.position;
                simulationContext.fitnesses[i] = - payloadDisplacementToTarget.magnitude_squared();
            }
        }

    evolve(simulationContext);

    handle_events_and_inputs(guiContext, simulationContext);
    draw_gui(guiContext, simulationContext);
    check_if_program_quitted(guiContext);
}

void idle_loop(GUIContext &guiContext, SimulationContext &simulationContext) {
    handle_events_and_inputs(guiContext, simulationContext);
    handle_fps(guiContext);
    // start training if button is pressed otherwise just show simulation 0.
    if (simulationContext.isTraining) {
        // train the specified num of generations
        for (int i=0; i<simulationContext.numOfGenerationsToTrain; i++) {
            training_loop(guiContext, simulationContext, i);
        }
        simulationContext.isTraining = false;
    } else {
        draw_gui_beneath_simulation(guiContext);
        // advance the first simulation
        simulationContext.simulations[0]->advance();
        // pass in 1 to the instancesToAdvance parameter to only do the first simulation.
        applyNeuralNetworkOutputsToSimulations(simulationContext, 1);
        // draw the simulation objects
        simulationContext.simulations[0]->draw_simulation_objects(guiContext.renderer);
    }
    draw_gui(guiContext, simulationContext);
}

int main(int argc, char **argv) {
    // Initialise
        cout << ">Initialising" << endl;
        // Init Contexts
            GUIContext guiContext = GUIContext();
            SimulationContext simulationContext = SimulationContext();
        { // Init GUI
            if (!guiContext.renderer) {cout << "Renderer init failed"; return 1;}    
            guiContext.targetPositionLabel.rect.x = simulationContext.targetPosition.x;
            guiContext.targetPositionLabel.rect.y = simulationContext.targetPosition.y;
            
            // string InitialPopupMessage = "            Welcome to Jonah's\n      Computer Science NEA Project\n      AI Controlled Manipulator Arm\n         (That you can train)!\n\nOverview:\nThis GUI is split into 3 separate panels.\n\nThe simulation panel displayes the\nevaluation process of each neural network\nand allows the user to see training,\nvalues, objectives & fitness criteria\nin real time.\n\nThe control panel is where you are\nable to see all of these options.\nStart training from the top\nof this panel.\n\nClick the ?s to find out more.";
            // show_popup(InitialPopupMessage, guiContext.popupIsActive, guiContext.popupLabel);
        }
        { // Add Simulations
            // inputs: payload Displacement To End Effector. payload Displacement To Target. payload velocity. angles & velos for each joint.
            const int numInputs = 2 + 2 + 2 + 2*simulationContext.numOfLinkages;
            const int numOutputs = simulationContext.numOfLinkages; // each output is the angular force on a joint.
            for (int i=0; i<simulationContext.numInstances; i++) {
                simulationContext.simulations.push_back(std::make_shared<Simulation>(simulationContext));
                simulationContext.neuralNetworks.push_back(std::make_shared<NeuralNetwork>(numInputs, numOutputs));
                simulationContext.fitnesses.push_back(0.0f);
            }
        }
        cout << ">Initialised" << endl;
    // Main loop
    while (!guiContext.quit) {
        // idle_loop() runs the main loop where no training is happening and just the best arm is being displayed.
        // idle_loop() will call training_loop() if the start training button is pressed.
        // training_loop() will run the training process.
        idle_loop(guiContext, simulationContext);
    }
    // Program closing. Clean before exit.
    kiss_clean(&guiContext.objects);
    return 0;
}







// added relu
// added targetPosition variable and label
// theres a bias for the arm to fall to the right after adding relu despite it not affecting the output layer (or atleast thats how it should work).
// its working
// I changed brain size
// I reduced mutation rate