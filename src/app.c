#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep_ms(x) Sleep(x)
    #define EXE_SUFFIX ".exe"
    #define EXE_PREFIX ".\\"
#else
    #include <unistd.h>
    #include <time.h>
    #define sleep_ms(x) usleep((x) * 1000)
    #define EXE_SUFFIX ""
    #define EXE_PREFIX "./"
#endif

#define NUM_FILTERS 4
#define NUM_IMAGES 2

const char *filters[NUM_FILTERS] = {
    "sobel",
    "gaussian",
    "laplacian",
    "sharpen"
};

const char *images[NUM_IMAGES] = {
    "../input_images/input1.png",
    "../input_images/input2_8k.png"
};

/* ============================================================
 * WALL-CLOCK TIMER (FIXED CORE FUNCTION)
 * ============================================================ */
#ifdef _WIN32
double now_seconds() {
    static LARGE_INTEGER freq;
    static int initialized = 0;
    LARGE_INTEGER counter;

    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }

    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
}
#else
double now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
#endif

/* ============================================================
 * COMMAND EXECUTION
 * ============================================================ */
void run_command(const char *cmd) {
    printf("Running: %s\n", cmd);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
}

/* ============================================================
 * TIMED EXECUTION (FIXED CORE FUNCTION)
 * ============================================================ */
double time_command(const char *cmd) {
    double start = now_seconds();
    run_command(cmd);
    double end = now_seconds();
    return end - start;
}

/* ============================================================
 * MAIN BENCHMARK DRIVER
 * ============================================================ */
int main(void) {

    /* Create output directories */
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

    FILE *report = fopen("../performance_report.txt", "w");
    if (!report) {
        perror("fopen");
        return 1;
    }

    fprintf(report, "Image Filtering Performance Report\n\n");

    for (int i = 0; i < NUM_IMAGES; i++) {

        const char *img = images[i];
        const char *img_tag = (i == 0) ? "normal" : "8k";

        printf("\nProcessing Image: %s\n", img);
        fprintf(report, "Image: %s\n", img);

        for (int f = 0; f < NUM_FILTERS; f++) {

            const char *filter = filters[f];
            int is_gaussian = (strcmp(filter, "gaussian") == 0);

            /* ---------------- SERIAL ---------------- */
            char serial_cmd[512];
            snprintf(
                serial_cmd, sizeof(serial_cmd),
                "%simage_filter_serial%s %s ../output/serial/output_%s_%s.png %s %s",
                EXE_PREFIX, EXE_SUFFIX,
                img, filter, img_tag,
                filter,
                is_gaussian ? "5 1.0" : ""
            );

            double t_serial = time_command(serial_cmd);
            fprintf(report, "Serial - %s: %.6f s\n", filter, t_serial);

            /* ---------------- OPENMP ---------------- */
            char parallel_cmd[512];
            snprintf(
                parallel_cmd, sizeof(parallel_cmd),
                "%simage_filter_parallel%s %s ../output/parallel/output_%s_%s.png 4 %s %s",
                EXE_PREFIX, EXE_SUFFIX,
                img, filter, img_tag,
                filter,
                is_gaussian ? "5 1.0" : ""
            );

            double t_parallel = time_command(parallel_cmd);
            fprintf(report, "Parallel - %s: %.6f s\n", filter, t_parallel);

            /* ---------------- MPI ---------------- */
            char mpi_cmd[512];

#ifdef _WIN32
            snprintf(
                mpi_cmd, sizeof(mpi_cmd),
                "mpiexec -n 4 mpi_filter%s %s ../output/distributed/output_%s_%s.png %s %s",
                EXE_SUFFIX,
                img, filter, img_tag,
                filter,
                is_gaussian ? "5 1.0" : ""
            );
#else
            snprintf(
                mpi_cmd, sizeof(mpi_cmd),
                "mpirun -np 4 ./mpi_filter%s %s ../output/distributed/output_%s_%s.png %s %s",
                EXE_SUFFIX,
                img, filter, img_tag,
                filter,
                is_gaussian ? "5 1.0" : ""
            );
#endif

            double t_distributed = time_command(mpi_cmd);
            fprintf(report, "Distributed - %s: %.6f s\n", filter, t_distributed);

            fprintf(report, "\n");
        }

        fprintf(report, "-------------------------------------\n\n");
    }

    fclose(report);

    printf("\nBenchmark complete. See ../performance_report.txt for results.\n");
    return 0;
}
