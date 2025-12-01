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
gcc main.c -o image_filter -lm
````

This generates the executable:

```
image_filter
```

---

## 3. Running the Program

General format:

```
./image_filter input.png output.png filter_name [parameters]
```

---

## 4. Filter Usage Examples

### (1) Sobel Edge Detection

**Command:**

```bash
./image_filter input.png output_sobel.png sobel
```

**Output:**
Grayscale edge-detected PNG.

---

### (2) Gaussian Blur

**Command:**

```bash
./image_filter input.png output_blur.png gaussian <ksize> <sigma>
```

**Example:**

```bash
./image_filter input.png blurred.png gaussian 5 1.0
```

**Parameters**

* `ksize` → odd integer (3, 5, 7, …)
* `sigma` → blur strength (0.8, 1.0, 1.5, …)

---

### (3) Laplacian Edge Filter

**Command:**

```bash
./image_filter input.png output_lap.png laplacian
```

---

### (4) Sharpen Filter

**Command:**

```bash
./image_filter input.png output_sharp.png sharpen
```
