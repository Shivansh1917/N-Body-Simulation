#include "websocket_client.h"
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<time.h>
#include <errno.h>

float timeStep = 0.01f;   // simulation time step
float G = 100.0f;          // gravitational constant



/*
   Returns a pointer to a dynamically allocated array of 2 floats,
   containing the x and y components of the gravitational force.
   Caller must free the returned pointer.
*/
float *forceApplied(float m1, float m2, float pos1[2], float pos2[2]) {
    // Calculate squared distance between the two particles
    float distsqr = pow(pos2[1] - pos1[1], 2) + pow(pos2[0] - pos1[0], 2);
    float forceMagnitude = (G * m1 * m2) / distsqr;
    float dist = sqrt(distsqr);

    // Difference in positions
    float xdif = pos2[0] - pos1[0];
    float ydif = pos2[1] - pos1[1];

    // Allocate and compute the force components (normalizing the difference)
    float *result = malloc(2 * sizeof(float));
    if (result == NULL) {
        perror("malloc failed");
        exit(1);
    }
    result[0] = forceMagnitude * (xdif / dist);
    result[1] = forceMagnitude * (ydif / dist);
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s numBodies csvFile\n", argv[0]);
        exit(1);
    }


    FILE *fout = fopen("myFile.txt", "a"); // Open file for writing
    if (!fout) {
        fprintf(stderr, "Failed to open file for writing: %s\n", strerror(errno));
        return 1;
    }

    // Get number of bodies and CSV file name from command-line.
    int numBodies = atoi(argv[1]);
    char *filename = argv[2];

    // Allocate arrays for mass, positions and velociti
    float *mass = malloc(numBodies * sizeof(float));
    float (*position)[2] = malloc(numBodies * sizeof(*position));
    float (*velocity)[2] = malloc(numBodies * sizeof(*velocity));
    if (mass == NULL || position == NULL || velocity == NULL) {
        perror("malloc failed");
        exit(1);
    }

    // Open the CSV file for reading.
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening CSV file");
        exit(1);
    }

    char line[1024];
    int i = 0;

    // Read first line – if it contains a header (checks for the string "mass"), skip it.
    if (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "mass") != NULL)
            ; // Detected header, do nothing.
        else {
            // No header detected; rewind to start to process this line again.
            fseek(fp, 0, SEEK_SET);
        }
    }
  
    // Read each body data from the CSV file.
    while (i < numBodies && fgets(line, sizeof(line), fp)) {
        // Remove any newline character.
        line[strcspn(line, "\n")] = '\0';
        
        // Tokenize the line using comma as separator.
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        mass[i] = (float)atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        position[i][0] = (float)atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        position[i][1] = (float)atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        velocity[i][0] = (float)atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        // The CSV's 5th column (named final_x_velocity) is used as the y velocity.
        velocity[i][1] = (float)atof(token);

        i++;
    }
    fclose(fp);

    // If the CSV file contained fewer bodies than expected, adjust.
    if (i < numBodies) {
        printf("Warning: Only %d bodies read from CSV file.\n", i);
        numBodies = i;
    }

    // Initialize WebSocket connection.
    initWebSocket();
    usleep(5000);

    float dt = timeStep;
    // Main simulation loop.
    while (true) {
        clock_t begin = clock();
        for (int i = 0; i < numBodies; i++) {
            float fx = 0.0f, fy = 0.0f;
            // Calculate net force on body i from all other bodies.
            for (int j = 0; j < numBodies; j++) {
                if (i != j) {
                    float *force = forceApplied(mass[i], mass[j], position[i], position[j]);
                    fx += force[0];
                    fy += force[1];
                    free(force);
                }
            }
            // Update positions and velocities (simple Euler integration).
            position[i][0] += velocity[i][0] * dt;
            position[i][1] += velocity[i][1] * dt;
            velocity[i][0] += fx * dt;
            velocity[i][1] += fy * dt;

            // Send updated position via WebSocket.
            sendSocket(i, position[i][0], position[i][1]);
            // usleep(10);
        }
        clock_t end = clock();
        double time_spent = (double)(end - begin)/CLOCKS_PER_SEC;
        fprintf(fout, "%f \n", time_spent);
        fflush(fout);
    }
    
    // Cleanup (never reached in this infinite loop).
    cleanupWebSocket();
    free(mass);
    free(position);
    free(velocity);
    fclose(fout); 
    return 0;
}
