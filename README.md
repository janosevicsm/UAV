
# OpenGL C++ 3D Drone Simulation
## Project Overview
This project is an upgraded version of a 2D drone simulation, now extended to a 3D environment. The simulation involves drones flying above a mountainous terrain, and the user interacts with a control station.

## Features
### General Features (Retained from 2D Project):
- Two LED indicators for active drones (dark for inactive, bright for active).
- Topographic map of Mount Majevica and surrounding areas displayed on the screen.
- No-fly zone above the Lopare municipality represented as a semi-transparent circle.
- Two battery level progress bars for each drone.
- Two circles representing the drones' positions.

- Drone battery decreases over time while active.
- Drone reactions to key combinations (1+U, 2+U, 1+I, 2+I, WSAD, Arrow keys).
- Collision handling: drones collide, enter the no-fly zone, or run out of battery - they are destroyed and permanently deactivated.

### Additional 3D Features:
- Depth testing enabled.
- Back-face culling enabled.
- Drones loaded as 3D models.
- Drones move in 3D space:
    - Rotation
    - Altitude control
    - Drone destruction upon collision with the ground.
    - Gradual descent when shutting down the drone.
- Lights: red on the left, green on the right, white at the back; turned off when the drone is off.
- Specular map for the terrain, making water highly reflective compared to the ground.
- Drone circle size on the map proportional to its height.
- Video feed from each drone's camera displayed.
- Toggleable camera feed with a gradual battery drain.
- Camera activation/deactivation even when the drone is off.

## Project Image
![Image](https://github.com/janosevicsm/UAV/blob/main/Projekat3D/UAV/res/images/UAV.png)

## Usage
- Clone the repository.
- Compile the C++ code using an OpenGL-compatible compiler.
- Run the executable to launch the 3D drone simulation.

## Author
- [Marko Janošević](https://github.com/janosevicsm)

## Academic Context
3D Drone Simulation was developed for the purposes of the course [Computer graphics](http://ftn.uns.ac.rs/1758954196/racunarska-grafika).
### Course Assistant
- Nedeljko Tešanović
### Course Professor
- Dragan Ivetić
