# IMAGE FILTER PROGRAM  
### (PNG → PNG Serial Version)

This program reads an input PNG image, applies a selected filter, and saves the output as a new PNG file.

---

## Supported Filters
- **sobel**
- **gaussian**
- **laplacian**
- **sharpen**

---

## 1. Requirements
- `main.c`  
- `stb_image.h`  
- `stb_image_write.h`  
- GCC compiler  

Download **stb** headers from:  
https://github.com/nothings/stb

---

## 2. Compilation

Run the following command:

```bash
For Serial:
gcc main.c -o image_filter_serial -lm

For Parallel:
gcc-15 main_parallel.c -o image_filter_parallel -fopenmp -lm
````

This generates the executable:

```
image_filter
```

---

## 3. Filter Usage Examples

### (1) Sobel Edge Detection

**Command:**

```bash
./image_filter_serial input.png output_sobel_serial.png sobel
./image_filter_parallel input.png output_sobel_parallel.png sobel
```

**Output:**
Grayscale edge-detected PNG.

---

### (2) Gaussian Blur

**Command:**

```bash
./image_filter_serial input.png output_gaussian_serial.png gaussian 5 1.0
./image_filter_parallel input.png output_gaussian_parallel.png gaussian 5 1.0
```

**Parameters**

* `ksize` → odd integer (3, 5, 7, …)
* `sigma` → blur strength (0.8, 1.0, 1.5, …)

---

### (3) Laplacian Edge Filter

**Command:**

```bash
./image_filter_serial input.png output_laplacian_serial.png laplacian
./image_filter_parallel input.png output_laplacian_parallel.png gaussian 5 1.0
```

---

### (4) Sharpen Filter

**Command:**

```bash
./image_filter_serial input.png output_sharpen_serial.png sharpen
./image_filter_parallel input.png output_sharpen_parallel.png gaussian 5 1.0
```