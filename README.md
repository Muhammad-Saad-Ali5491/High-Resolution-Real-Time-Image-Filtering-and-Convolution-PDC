# 🖼️ Image Filter Program

> **High-Resolution Real-Time Image Filtering and Convolution**  
> PNG → PNG conversion with multiple filter options

---

## 📋 Overview

This program reads an input PNG image, applies a selected filter, and saves the output as a new PNG file. It supports both **serial** and **parallel** execution modes for optimal performance.

---

## ✨ Supported Filters

| Filter | Description |
|--------|-------------|
| **sobel** | Edge detection filter |
| **gaussian** | Blur filter with configurable kernel size and sigma |
| **laplacian** | Edge enhancement filter |
| **sharpen** | Image sharpening filter |

---

## 📦 Requirements
- `main_parallel.c`
- `main.c`
- `stb_image.h`
- `stb_image_write.h`
- GCC compiler

### Download STB Headers

Get the **stb** headers from:  
🔗 [https://github.com/nothings/stb](https://github.com/nothings/stb)

---

## 🔨 Compilation

### Serial Version
```bash
gcc main.c -o image_filter_serial -lm
```

### Parallel Version
```bash
gcc-15 main_parallel.c -o image_filter_parallel -fopenmp -lm
```

## 🚀 Filter Usage 

### 1️⃣ Sobel Edge Detection

Applies edge detection to create a grayscale edge-detected PNG.

**Command:**
```bash
# Serial
./image_filter_serial input.png output_sobel_serial.png sobel

# Parallel
./image_filter_parallel input.png output_sobel_parallel.png sobel
```

---

### 2️⃣ Gaussian Blur

Applies blur effect with configurable parameters.

**Command:**
```bash
# Serial
./image_filter_serial input.png output_gaussian_serial.png gaussian 5 1.0

# Parallel
./image_filter_parallel input.png output_gaussian_parallel.png gaussian 5 1.0
```

**Parameters:**
- `ksize` → Kernel size (odd integer: 3, 5, 7, …)
- `sigma` → Blur strength (e.g., 0.8, 1.0, 1.5, …)

---

### 3️⃣ Laplacian Edge Filter

Applies Laplacian edge enhancement.

**Command:**
```bash
# Serial
./image_filter_serial input.png output_laplacian_serial.png laplacian

# Parallel
./image_filter_parallel input.png output_laplacian_parallel.png laplacian
```

---

### 4️⃣ Sharpen Filter

Enhances image sharpness.

**Command:**
```bash
# Serial
./image_filter_serial input.png output_sharpen_serial.png sharpen

# Parallel
./image_filter_parallel input.png output_sharpen_parallel.png sharpen
```

---

## ⚡ Performance Benchmarks


| Filter | Serial Execution Time |
|--------|----------------|
| Sobel | 0.016043s |
| Gaussian | 0.055719s |
| Laplacian | 0.053983s |
| Sharpen | 0.054240s |

---

## 📝 General Usage Pattern

```bash
./image_filter_[serial|parallel] <input.png> <output.png> <filter> [parameters]
```

**Arguments:**
- `input.png` - Source image file
- `output.png` - Destination image file
- `filter` - Filter type (sobel, gaussian, laplacian, sharpen)
- `[parameters]` - Optional filter-specific parameters (e.g., ksize and sigma for gaussian)

---

## 👤 Author

**saffisardar and saadali**

---
