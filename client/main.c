#include "websocket_client.h"
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

float timeStep = 0.01;
float G = 10.0f;

/* 
   Returns a pointer to a dynamically allocated array of 2 floats,
   containing the x and y components of the force applied.
   Caller must free the returned pointer.
*/
float *forceApplied(float m1, float m2, float pos1[], float pos2[]) {
    /* Calculate gravitational force magnitude */
    float distsqr = pow(pos2[1] - pos1[1], 2) + pow(pos2[0] - pos1[0], 2);
    float forceMagnitude = (G * m1 * m2) / distsqr;
    float dist = sqrt(distsqr);

    /* Get the difference in position */
    float xdif = pos2[0] - pos1[0];
    float ydif = pos2[1] - pos1[1];

    /* Normalize the vector and multiply by the force magnitude */
    float *result = malloc(2 * sizeof(float));
    if (result == NULL) {
        perror("malloc failed");
        exit(1);
    }
    result[0] = forceMagnitude * (xdif / dist);
    result[1] = forceMagnitude * (ydif / dist);

    return result;
}

int main() {
    initWebSocket();
    usleep(5000);
    int numParticles = 2;
    float position[numParticles][2];
    float velocity[numParticles][2];
    float mass[2];

    /* Initialize positions */
    position[0][0] = 240.2; position[0][1] = 240.4;
    position[1][0] = 260.5; position[1][1] = 260.3;

    /* Initialize velocities */
    velocity[0][0] = 0; velocity[0][1] = 5.4;
    velocity[1][0] = -5.4; velocity[1][1] = 0;

    mass[0] = 3.1;
    mass[1] = 2.2;

    float dt = 0.01;
    while (true) {
        for (int i = 0; i < numParticles; i++) {
            float fx = 0.0f, fy = 0.0f;
            for (int j = 0; j < numParticles; j++) {
                if (i != j) {
                    /* Get the force vector from particle j on i.
                       Note: We now declare force as a pointer. */
                    float *force = forceApplied(mass[i], mass[j], position[i], position[j]);
                    fx += force[0];
                    fy += force[1];
                    free(force);  // free the dynamically allocated memory
                }
            }
            /* Update position and velocity */
            position[i][0] += velocity[i][0] * dt;
            position[i][1] += velocity[i][1] * dt;
            velocity[i][0] += fx * dt;
            velocity[i][1] += fy * dt;

            sendSocket(i, position[i][0], position[i][1]);
            usleep(10000);
        }
    }

    cleanupWebSocket();
    return 0;
}
