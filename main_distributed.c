#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <mpi.h>  // Requires MPI installation (e.g., OpenMPI, MPICH)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*******************************************************************************
 * UTILITY FUNCTIONS
 ******************************************************************************/

static inline unsigned char clamp255(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (unsigned char)v;
}

/*******************************************************************************
 * BUILD GAUSSIAN KERNEL
 ******************************************************************************/
double* build_gaussian(int ksize, double sigma) {
    double *k = (double*)malloc(ksize * ksize * sizeof(double));
    int half = ksize / 2;
    double sum = 0.0;

    for (int y = -half; y <= half; y++) {
        for (int x = -half; x <= half; x++) {
            double v = exp(-(x*x + y*y) / (2.0 * sigma * sigma));
            k[(y + half) * ksize + (x + half)] = v;
            sum += v;
        }
    }
    
    // Normalize
    for (int i = 0; i < ksize * ksize; i++) {
        k[i] /= sum;
    }

    return k;
}

/*******************************************************************************
 * LOCAL CONVOLUTION (operates on extended buffer with halos)
 * 
 * extended: buffer containing [halo_top rows] + [local_rows] + [halo_bot rows]
 * local_out: output buffer for local_rows only
 * w: image width
 * local_rows: number of rows this rank is responsible for
 * channels: number of color channels (3 for RGB)
 * kernel: convolution kernel
 * ksize: kernel size
 * halo: number of halo rows on each side
 * global_y_start: starting row index in global image
 * global_h: total image height
 ******************************************************************************/
void convolve_rgb_local(unsigned char *extended, unsigned char *local_out,
                        int w, int local_rows, int channels,
                        double *kernel, int ksize, int halo,
                        int global_y_start, int global_h)
{
    int half = ksize / 2;
    int extended_rows = local_rows + 2 * halo;

    for (int y = 0; y < local_rows; y++) {
        int global_y = global_y_start + y;  // Global row index
        
        for (int x = 0; x < w; x++) {
            for (int c = 0; c < channels; c++) {
                double acc = 0.0;

                for (int ky = -half; ky <= half; ky++) {
                    for (int kx = -half; kx <= half; kx++) {
                        // Global coordinates
                        int gx = x + kx;
                        int gy = global_y + ky;

                        // Clamp to image boundaries (global)
                        if (gx < 0) gx = 0;
                        if (gx >= w) gx = w - 1;
                        if (gy < 0) gy = 0;
                        if (gy >= global_h) gy = global_h - 1;

                        // Convert global y to extended buffer index
                        // extended buffer starts at global row (global_y_start - halo)
                        int ext_y = gy - (global_y_start - halo);
                        
                        // Clamp to extended buffer bounds
                        if (ext_y < 0) ext_y = 0;
                        if (ext_y >= extended_rows) ext_y = extended_rows - 1;

                        int idx = (ext_y * w + gx) * channels + c;
                        int kidx = (ky + half) * ksize + (kx + half);

                        acc += extended[idx] * kernel[kidx];
                    }
                }

                int out_index = (y * w + x) * channels + c;
                local_out[out_index] = clamp255((int)acc);
            }
        }
    }
}

/*******************************************************************************
 * LOCAL SOBEL FILTER
 ******************************************************************************/
void sobel_local(unsigned char *extended, unsigned char *local_out,
                 int w, int local_rows, int ch, int halo,
                 int global_y_start, int global_h)
{
    int extended_rows = local_rows + 2 * halo;
    
    // Convert extended region to grayscale
    unsigned char *gray = (unsigned char*)malloc(w * extended_rows);
    
    for (int i = 0; i < w * extended_rows; i++) {
        int r = extended[i * ch + 0];
        int g = extended[i * ch + 1];
        int b = extended[i * ch + 2];
        gray[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
    }

    int gx[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int gy[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

    for (int y = 0; y < local_rows; y++) {
        int global_y = global_y_start + y;
        
        for (int x = 0; x < w; x++) {
            double sx = 0.0, sy = 0.0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    // Global coordinates
                    int gxx = x + kx;
                    int gyy = global_y + ky;

                    // Clamp to image boundaries
                    if (gxx < 0) gxx = 0;
                    if (gxx >= w) gxx = w - 1;
                    if (gyy < 0) gyy = 0;
                    if (gyy >= global_h) gyy = global_h - 1;

                    // Convert to extended buffer index
                    int ext_y = gyy - (global_y_start - halo);
                    if (ext_y < 0) ext_y = 0;
                    if (ext_y >= extended_rows) ext_y = extended_rows - 1;

                    int val = gray[ext_y * w + gxx];
                    int kidx = (ky + 1) * 3 + (kx + 1);

                    sx += val * gx[kidx];
                    sy += val * gy[kidx];
                }
            }

            int mag = (int)sqrt(sx * sx + sy * sy);
            if (mag > 255) mag = 255;

            // Write to all channels
            for (int c = 0; c < ch; c++) {
                local_out[(y * w + x) * ch + c] = (unsigned char)mag;
            }
        }
    }

    free(gray);
}

/*******************************************************************************
 * MAIN FUNCTION
 ******************************************************************************/
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) {
        if (rank == 0) {
            printf("Usage: %s input.png output.png [sobel|gaussian|laplacian|sharpen] [params]\n", argv[0]);
            printf("  gaussian requires: ksize sigma\n");
        }
        MPI_Finalize();
        return 1;
    }

    char *infile = argv[1];
    char *outfile = argv[2];
    char *mode = argv[3];

    int w = 0, h = 0, ch = 3;
    unsigned char *img = NULL;
    unsigned char *out = NULL;
    double *kernel = NULL;
    int ksize = 3;
    double sigma = 1.0;

    double start_time = MPI_Wtime();

    /***************************************************************************
     * STEP 1: Parse parameters and determine kernel size
     ***************************************************************************/
    if (strcmp(mode, "gaussian") == 0) {
        if (argc < 6) {
            if (rank == 0) printf("Usage: gaussian ksize sigma\n");
            MPI_Finalize();
            return 1;
        }
        ksize = atoi(argv[4]);
        sigma = atof(argv[5]);
    }

    int halo = ksize / 2;  // Number of rows needed from neighbors

    /***************************************************************************
     * STEP 2: Root loads the image
     ***************************************************************************/
    if (rank == 0) {
        img = stbi_load(infile, &w, &h, &ch, 3);
        if (!img) {
            printf("Error loading image: %s\n", infile);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        ch = 3;  // Force RGB
        out = (unsigned char*)malloc(w * h * ch);
        
        printf("Image loaded: %d x %d, %d channels\n", w, h, ch);
        printf("Using %d MPI processes\n", size);
    }

    /***************************************************************************
     * STEP 3: Broadcast image dimensions and parameters
     ***************************************************************************/
    MPI_Bcast(&w, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&h, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&ch, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&ksize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sigma, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    halo = ksize / 2;

    /***************************************************************************
     * STEP 4: Build kernel on all processes
     ***************************************************************************/
    if (strcmp(mode, "gaussian") == 0) {
        kernel = build_gaussian(ksize, sigma);
    } 
    else if (strcmp(mode, "laplacian") == 0) {
        kernel = (double*)malloc(9 * sizeof(double));
        double lap[9] = {0, 1, 0, 1, -4, 1, 0, 1, 0};
        memcpy(kernel, lap, 9 * sizeof(double));
    } 
    else if (strcmp(mode, "sharpen") == 0) {
        kernel = (double*)malloc(9 * sizeof(double));
        double sh[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
        memcpy(kernel, sh, 9 * sizeof(double));
    }

    /***************************************************************************
     * STEP 5: Calculate row distribution across processes
     ***************************************************************************/
    int *row_counts = (int*)malloc(size * sizeof(int));
    int *row_starts = (int*)malloc(size * sizeof(int));

    int base_rows = h / size;
    int extra = h % size;

    int offset = 0;
    for (int i = 0; i < size; i++) {
        row_counts[i] = base_rows + (i < extra ? 1 : 0);
        row_starts[i] = offset;
        offset += row_counts[i];
    }

    int local_rows = row_counts[rank];
    int my_start = row_starts[rank];

    // Prepare scatter/gather parameters
    int *sendcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        sendcounts[i] = row_counts[i] * w * ch;
        displs[i] = row_starts[i] * w * ch;
    }

    /***************************************************************************
     * STEP 6: Scatter image data to all processes
     ***************************************************************************/
    unsigned char *local_data = (unsigned char*)malloc(local_rows * w * ch);

    MPI_Scatterv(img, sendcounts, displs, MPI_UNSIGNED_CHAR,
                 local_data, local_rows * w * ch, MPI_UNSIGNED_CHAR,
                 0, MPI_COMM_WORLD);

    /***************************************************************************
     * STEP 7: Create extended buffer with halo regions
     ***************************************************************************/
    int extended_rows = local_rows + 2 * halo;
    unsigned char *extended = (unsigned char*)malloc(extended_rows * w * ch);

    // Copy local data to center of extended buffer
    memcpy(extended + halo * w * ch, local_data, local_rows * w * ch);

    /***************************************************************************
     * STEP 8: Halo exchange with neighboring processes
     ***************************************************************************/
    MPI_Request reqs[4];
    int nreqs = 0;

    // Calculate actual halo sizes to send/receive
    int send_top = (local_rows >= halo) ? halo : local_rows;
    int send_bot = (local_rows >= halo) ? halo : local_rows;

    // Exchange with previous rank (rank - 1)
    if (rank > 0) {
        // Send my first 'send_top' rows to previous rank
        MPI_Isend(local_data, send_top * w * ch, MPI_UNSIGNED_CHAR,
                  rank - 1, 0, MPI_COMM_WORLD, &reqs[nreqs++]);
        
        // Receive their last rows into my top halo
        MPI_Irecv(extended, halo * w * ch, MPI_UNSIGNED_CHAR,
                  rank - 1, 1, MPI_COMM_WORLD, &reqs[nreqs++]);
    }

    // Exchange with next rank (rank + 1)
    if (rank < size - 1) {
        // Send my last 'send_bot' rows to next rank
        MPI_Isend(local_data + (local_rows - send_bot) * w * ch,
                  send_bot * w * ch, MPI_UNSIGNED_CHAR,
                  rank + 1, 1, MPI_COMM_WORLD, &reqs[nreqs++]);
        
        // Receive their first rows into my bottom halo
        MPI_Irecv(extended + (halo + local_rows) * w * ch,
                  halo * w * ch, MPI_UNSIGNED_CHAR,
                  rank + 1, 0, MPI_COMM_WORLD, &reqs[nreqs++]);
    }
    
    MPI_Waitall(nreqs, reqs, MPI_STATUSES_IGNORE);
    /***************************************************************************
     * STEP 9: Handle boundary conditions (replicate edge rows)
     ***************************************************************************/
    // For rank 0: Fill top halo by replicating first row of local data
    if (rank == 0) {
        for (int i = 0; i < halo; i++) {
            memcpy(extended + i * w * ch,
                   local_data,  // First row of local data
                   w * ch);
        }
    }

    // For last rank: Fill bottom halo by replicating last row of local data
    if (rank == size - 1) {
        for (int i = 0; i < halo; i++) {
            memcpy(extended + (halo + local_rows + i) * w * ch,
                   local_data + (local_rows - 1) * w * ch,  // Last row
                   w * ch);
        }
    }

    /***************************************************************************
     * STEP 10: Apply the filter
     ***************************************************************************/
    unsigned char *local_out = (unsigned char*)malloc(local_rows * w * ch);

    if (strcmp(mode, "sobel") == 0) {
        sobel_local(extended, local_out, w, local_rows, ch, halo, my_start, h);
    } 
    else {
        convolve_rgb_local(extended, local_out, w, local_rows, ch,
                           kernel, ksize, halo, my_start, h);
    }

    /***************************************************************************
     * STEP 11: Gather results back to root
     ***************************************************************************/
    MPI_Gatherv(local_out, local_rows * w * ch, MPI_UNSIGNED_CHAR,
                out, sendcounts, displs, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    /***************************************************************************
     * STEP 12: Root writes output and reports timing
     ***************************************************************************/
    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Filter: %s\n", mode);
        printf("Execution time: %.6f seconds\n", end_time - start_time);
        
        stbi_write_png(outfile, w, h, ch, out, w * ch);
        printf("Output written to: %s\n", outfile);
        
        free(img);
        free(out);
    }

    /***************************************************************************
     * STEP 13: Cleanup
     ***************************************************************************/
    free(local_data);
    free(extended);
    free(local_out);
    free(row_counts);
    free(row_starts);
    free(sendcounts);
    free(displs);
    if (kernel) free(kernel);

    MPI_Finalize();
    return 0;
}