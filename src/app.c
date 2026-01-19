#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#define EXE_SUFFIX ".exe"  // For Windows executables
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#define EXE_SUFFIX ""  // No suffix on Mac/Linux
#endif

#define NUM_FILTERS 4
#define NUM_IMAGES 2

const char *filters[NUM_FILTERS] = {"sobel", "gaussian", "laplacian", "sharpen"};
const char *images[NUM_IMAGES] = {"../input_images/input1.png", "../input_images/input2_8k.png"};

void run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
}

double time_command(const char *cmd) {
    clock_t start = clock();
    run_command(cmd);
    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

int main() {
    // Create output directories (platform-specific)
#ifdef _WIN32
    system("mkdir ..\\output\\serial >nul 2>nul");
    system("mkdir ..\\output\\parallel >nul 2>nul");
    system("mkdir ..\\output\\distributed >nul 2>nul");
#else
    system("mkdir -p ../output/serial");
    system("mkdir -p ../output/parallel");
    system("mkdir -p ../output/distributed");
#endif

    printf("=== Image Filtering Benchmark ===\n");

    FILE *report = fopen("../performance_report.txt", "w");  // In root folder
    if (!report) {
        perror("fopen");
        return 1;
    }

    fprintf(report, "Image Filtering Performance Report\n\n");

    for (int i = 0; i < NUM_IMAGES; i++) {
        const char *img = images[i];
        printf("\nProcessing Image: %s\n", img);
        fprintf(report, "Image: %s\n", img);

        for (int f = 0; f < NUM_FILTERS; f++) {
            const char *filter = filters[f];

            // Serial
            char serial_cmd[512];
            sprintf(serial_cmd, ".\\image_filter_serial%s %s ../output/serial/output_%s_%s.png %s %s",
                    EXE_SUFFIX, img, filter, (i == 0) ? "normal" : "8k", filter,
                    (strcmp(filter, "gaussian") == 0) ? "5 1.0" : "");
            double t_serial = time_command(serial_cmd);
            fprintf(report, "Serial - %s: %.4f s\n", filter, t_serial);

            // Parallel (4 threads)
            char parallel_cmd[512];
            sprintf(parallel_cmd, ".\\image_filter_parallel%s %s ../output/parallel/output_%s_%s.png 4 %s %s",
                    EXE_SUFFIX, img, filter, (i == 0) ? "normal" : "8k", filter,
                    (strcmp(filter, "gaussian") == 0) ? "5 1.0" : "");
            double t_parallel = time_command(parallel_cmd);
            fprintf(report, "Parallel - %s: %.4f s\n", filter, t_parallel);

            // Distributed (4 processes; platform-specific MPI command)
            char mpi_cmd[512];
#ifdef _WIN32
            // MS-MPI on Windows uses mpiexec -n
            sprintf(mpi_cmd, "mpiexec -n 4 mpi_filter%s %s ../output/distributed/output_%s_%s.png %s %s",
                    EXE_SUFFIX, img, filter, (i == 0) ? "normal" : "8k", filter,
                    (strcmp(filter, "gaussian") == 0) ? "5 1.0" : "");
#else
            // OpenMPI on Mac/Linux uses mpirun -np
            sprintf(mpi_cmd, "mpirun -np 4 ./mpi_filter%s %s ../output/distributed/output_%s_%s.png %s %s",
                    EXE_SUFFIX, img, filter, (i == 0) ? "normal" : "8k", filter,
                    (strcmp(filter, "gaussian") == 0) ? "5 1.0" : "");
#endif
            double t_distributed = time_command(mpi_cmd);
            fprintf(report, "Distributed - %s: %.4f s\n", filter, t_distributed);

            fprintf(report, "\n");
        }

        fprintf(report, "-------------------------------------\n\n");
    }

    fclose(report);
    printf("\nBenchmark complete. See ../performance_report.txt (in project root) for results.\n");

    return 0;
}