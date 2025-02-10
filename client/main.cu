#include "websocket_client.h"
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <cuda_runtime.h>

// Simulation parameters
float timeStep = 0.01f;   // simulation time step
float G = 100.0f;         // gravitational constant

// Block size for CUDA kernel launch
#define BLOCK_SIZE 256

// The CUDA kernel: each thread computes net force on one body, then updates its velocity and position.
__global__ void updateBodies(int numBodies, float dt, float G, const float *masses, float2 *d_positions, float2 *d_velocities) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= numBodies) return;

    // Load body i’s current state.
    float2 pos_i = d_positions[i];
    float2 vel_i = d_velocities[i];
    float mass_i = masses[i];
    float fx = 0.0f, fy = 0.0f;

    // Loop over all other bodies.
    for (int j = 0; j < numBodies; j++) {
        if (i == j)
            continue;
        float2 pos_j = d_positions[j];
        float dx = pos_j.x - pos_i.x;
        float dy = pos_j.y - pos_i.y;
        float distSqr = dx*dx + dy*dy;
        float dist = sqrtf(distSqr);
        if (dist > 0.0f) {
            // Compute force magnitude and add components
            float forceMagnitude = (G * mass_i * masses[j]) / distSqr;
            fx += forceMagnitude * (dx / dist);
            fy += forceMagnitude * (dy / dist);
        }
    }

    // Update the velocity and then the position (simple Euler integration)
    vel_i.x += fx * dt;
    vel_i.y += fy * dt;
    pos_i.x += vel_i.x * dt;
    pos_i.y += vel_i.y * dt;

    // Write back to global memory.
    d_velocities[i] = vel_i;
    d_positions[i] = pos_i;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s numBodies csvFile\n", argv[0]);
        exit(1);
    }

    // Open file for logging timing
    FILE *fout = fopen("myFile.txt", "a");
    if (!fout) {
        fprintf(stderr, "Failed to open file for writing: %s\n", strerror(errno));
        return 1;
    }

    // Get simulation parameters from command line.
    int numBodies = atoi(argv[1]);
    char *csvFilename = argv[2];

    // Allocate host memory.
    float *h_mass = (float *)malloc(numBodies * sizeof(float));
    // Using float2 for positions and velocities; CUDA already defines float2 in cuda_runtime.h.
    float2 *h_positions = (float2 *)malloc(numBodies * sizeof(float2));
    float2 *h_velocities = (float2 *)malloc(numBodies * sizeof(float2));
    if (h_mass == NULL || h_positions == NULL || h_velocities == NULL) {
        perror("malloc failed");
        exit(1);
    }

    // Open the CSV file for reading.
    FILE *fp = fopen(csvFilename, "r");
    if (fp == NULL) {
        perror("Error opening CSV file");
        exit(1);
    }

    char line[1024];
    int i = 0;
    // Check if the first line is a header (containing "mass")
    if (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "mass") != NULL) {
            // Header detected; do nothing.
        } else {
            // No header; rewind to process this line.
            fseek(fp, 0, SEEK_SET);
        }
    }
  
    // Read each body data from the CSV file.
    while (i < numBodies && fgets(line, sizeof(line), fp)) {
        // Remove any newline character.
        line[strcspn(line, "\n")] = '\0';
        
        // Tokenize the line using comma as the separator.
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        h_mass[i] = atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        h_positions[i].x = atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        h_positions[i].y = atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        h_velocities[i].x = atof(token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        // The CSV's 5th column (named final_x_velocity) is used as the y velocity.
        h_velocities[i].y = atof(token);

        i++;
    }
    fclose(fp);

    // Adjust numBodies if fewer lines were read.
    if (i < numBodies) {
        printf("Warning: Only %d bodies read from CSV file.\n", i);
        numBodies = i;
    }

    // Initialize WebSocket connection.
    initWebSocket(); 
    usleep(5000);

    // Allocate device memory.
    float *d_mass;
    float2 *d_positions, *d_velocities;
    cudaMalloc((void**)&d_mass, numBodies * sizeof(float));
    cudaMalloc((void**)&d_positions, numBodies * sizeof(float2));
    cudaMalloc((void**)&d_velocities, numBodies * sizeof(float2));

    // Copy host data to device.
    cudaMemcpy(d_mass, h_mass, numBodies * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_positions, h_positions, numBodies * sizeof(float2), cudaMemcpyHostToDevice);
    cudaMemcpy(d_velocities, h_velocities, numBodies * sizeof(float2), cudaMemcpyHostToDevice);

    float dt = timeStep;
    dim3 block(BLOCK_SIZE);
    dim3 grid((numBodies + BLOCK_SIZE - 1) / BLOCK_SIZE);

    // Main simulation loop.
    while (true) {
        clock_t begin = clock();

        // Launch the kernel on the device.
        updateBodies<<<grid, block>>>(numBodies, dt, G, d_mass, d_positions, d_velocities);
        cudaDeviceSynchronize();

        // Copy updated positions back to host to send via WebSocket.
        cudaMemcpy(h_positions, d_positions, numBodies * sizeof(float2), cudaMemcpyDeviceToHost);

        // Send updated positions via WebSocket.
        for (int i = 0; i < numBodies; i++) {
            sendSocket(i, h_positions[i].x, h_positions[i].y);
            usleep(1);
        }

        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        fprintf(fout, "%f \n", time_spent);
        fflush(fout);
    }

    // Cleanup (never reached in this infinite loop).
    cleanupWebSocket();
    cudaFree(d_mass);
    cudaFree(d_positions);
    cudaFree(d_velocities);
    free(h_mass);
    free(h_positions);
    free(h_velocities);
    fclose(fout);
    return 0;
}


// nvcc main.cu ./websocket_client.c -o main -lwebsockets -lm 
// Run command