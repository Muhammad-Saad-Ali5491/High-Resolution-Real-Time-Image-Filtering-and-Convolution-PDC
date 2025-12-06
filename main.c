#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static inline unsigned char clamp255(int v) {
    if(v < 0) return 0;
    if(v > 255) return 255;
    return (unsigned char)v;
}

// Laplacian+Sharpen filter
void convolve_rgb(unsigned char *in, unsigned char *out,
                  int w, int h, int channels,
                  double *kernel, int ksize)
{
    int half = ksize / 2;

    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            for(int c = 0; c < channels; c++) {

                double acc = 0.0;

                for(int ky = -half; ky <= half; ky++) {
                    for(int kx = -half; kx <= half; kx++) {

                        int xx = x + kx;
                        int yy = y + ky;

                        if(xx < 0) xx = 0;
                        if(xx >= w) xx = w-1;
                        if(yy < 0) yy = 0;
                        if(yy >= h) yy = h-1;

                        int idx = (yy * w + xx) * channels + c;
                        int kidx = (ky + half)*ksize + (kx + half);

                        acc += in[idx] * kernel[kidx];
                    }
                }

                int out_index = (y * w + x) * channels + c;
                out[out_index] = clamp255((int)acc);
            }
        }
    }
}

// Used for Sobel
unsigned char* to_grayscale(unsigned char *img, int w, int h, int ch) {
    unsigned char *g = malloc(w * h);
    for(int i=0;i<w*h;i++) {
        int r = img[i*ch + 0];
        int g1 = img[i*ch + 1];
        int b = img[i*ch + 2];

        g[i] = (unsigned char)(0.299*r + 0.587*g1 + 0.114*b);
    }
    return g;
}

// Sobel filter
void sobel(unsigned char *img, unsigned char *out, int w, int h, int ch)
{
    unsigned char *g = to_grayscale(img, w, h, ch);

    int gx[9] = {-1,0,1,-2,0,2,-1,0,1};
    int gy[9] = {-1,-2,-1,0,0,0,1,2,1};

    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            double sx=0, sy=0;

            for(int ky=-1; ky<=1; ky++) {
                for(int kx=-1; kx<=1; kx++) {
                    int xx = x + kx;
                    int yy = y + ky;

                    if(xx<0) xx=0;
                    if(xx>=w) xx=w-1;
                    if(yy<0) yy=0;
                    if(yy>=h) yy=h-1;

                    int val = g[yy*w + xx];
                    int idx = (ky+1)*3 + (kx+1);

                    sx += val * gx[idx];
                    sy += val * gy[idx];
                }
            }

            int mag = (int)sqrt(sx*sx + sy*sy);
            if(mag > 255) mag = 255;

            // write as grayscale PNG (all channels equal)
            for(int c=0;c<ch;c++)
                out[(y*w + x)*ch + c] = (unsigned char)mag;
        }
    }

    free(g);
}

// Gaussian filter
double* build_gaussian(int ksize, double sigma) {
    double *k = malloc(ksize * ksize * sizeof(double));
    int half = ksize/2;
    double sum = 0.0;

    for(int y=-half;y<=half;y++){
        for(int x=-half;x<=half;x++){
            double v = exp(-(x*x + y*y)/(2*sigma*sigma));
            k[(y+half)*ksize + (x+half)] = v;
            sum += v;
        }
    }
    // Normalize
    for(int i=0;i<ksize*ksize;i++) k[i] /= sum;

    return k;
}

int main(int argc, char **argv)
{
    if(argc < 4) {
        printf("Usage: %s input.png output.png [sobel|gaussian|laplacian|sharpen] [params]\n", argv[0]);
        return 1;
    }

    char *infile = argv[1];
    char *outfile = argv[2];
    char *mode = argv[3];

    int w, h, ch;

    unsigned char *img = stbi_load(infile, &w, &h, &ch, 3);
    if(!img) {
        printf("Error loading image.\n");
        return 1;
    }

    ch = 3; // force RGB
    unsigned char *out = malloc(w*h*ch);

    if(strcmp(mode,"sobel")==0) {
        sobel(img, out, w, h, ch);
    }
    else if(strcmp(mode,"gaussian")==0) {
        if(argc < 6) {
            printf("Usage: gaussian ksize sigma\n");
            return 1;
        }
        int ksize = atoi(argv[4]);
        double sigma = atof(argv[5]);

        double *g = build_gaussian(ksize, sigma);
        convolve_rgb(img, out, w, h, ch, g, ksize);
        free(g);
    }
    else if(strcmp(mode,"laplacian")==0) {
        double lap[9] = {0,1,0,
                         1,-4,1,
                         0,1,0};
        convolve_rgb(img, out, w, h, ch, lap, 3);
    }
    else if(strcmp(mode,"sharpen")==0) {
        double sh[9] = {0,-1,0,
                        -1,5,-1,
                        0,-1,0};
        convolve_rgb(img, out, w, h, ch, sh, 3);
    }
    else {
        printf("Unknown mode.\n");
        return 1;
    }

    // Write PNG
    stbi_write_png(outfile, w, h, ch, out, w*ch);

    free(img);
    free(out);

    return 0;
}
