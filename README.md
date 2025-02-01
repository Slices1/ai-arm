# ai-arm
Hi I made this project for my computer science NEA. I took it as an opportunity to create the biggest coding project of my life yet. My write up is currently at ~170 pages... yup and Im not done yet!

I will try to put gifs here to show the program working or even make a youtube video on it but thats a WIP atm.

# Project description
This is a 

involving:
- Physical interactions
- Continuous collision detection (god this took ages)
- Neural networks
- Genetic algorithm
- Save / load
- ... WIP


## Neural network implementation
It uses a feedforward neural network, so each layer has weights, biases, and activation functions
I ended up using tanh() activation functions and had fun optimising it a bunch

## Neural network training
I use a genetic algorithm (GA), involving 

## Physics interactions
I use the semi-implicit euler integration method for my acceleration, velocity and position calculations.

I use a [line intersecting algorithm](https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/) for continuous collision detection. I designed all of the physics myself, and ended up kind of making a point capsule collider model.

Balls can and will bounce off collider corners accurately. So for example instead of always bouncing normal to the collider, it could bounce at 45 deg to it if it hits the corner right.
