# 🖼️ Image Filter Program

> **High-Resolution Real-Time Image Filtering and Convolution**  
> PNG → PNG conversion with Serial, Parallel (OpenMP), and Distributed (MPI) execution

---

## 📋 Quick Start

### Download STB Headers
```bash
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
```

---

## 🔨 Compilation

### 1. Serial Version
```bash
gcc main.c -o image_filter_serial -fopenmp -lm
```

### 2. Parallel Version (OpenMP)
**macOS:**
```bash
gcc-15 main_parallel.c -o image_filter_parallel -fopenmp -lm
```

**Linux/Windows:**
```bash
gcc main_parallel.c -o image_filter_parallel -fopenmp -lm
```

### 3. Distributed Version (MPI)

**Install MPI:**
```bash
# macOS
brew install open-mpi

# Ubuntu/Debian
sudo apt-get install libopenmpi-dev openmpi-bin

# CentOS/RHEL
sudo yum install openmpi openmpi-devel
```

**Setup & Compile:**
```bash
# Create MPI directory
mkdir mpi
cd mpi

# Download STB headers
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Copy your main_distributed.c here
cp ../main_distributed.c .

# Compile
mpicc main_distributed.c -o mpi_filter -lm
```

---

## 🚀 How to Run

### Command Formats

**Serial:**
```bash
./image_filter_serial <input.png> <output.png> <filter> [parameters]
```

**Parallel (OpenMP):**
```bash
./image_filter_parallel <input.png> <output.png> <threads> <filter> [parameters]
```

**Distributed (MPI):**
```bash
mpirun -np <processes> ./mpi_filter <input.png> <output.png> <filter> [parameters]
```

---

## 🎨 Filter Commands

### 1️⃣ Sobel Edge Detection

**Serial:**
```bash
./image_filter_serial input.png output_sobel.png sobel
```

**Parallel (4 threads):**
```bash
./image_filter_parallel input.png output_sobel.png 4 sobel
```

**MPI (4 processes):**
```bash
mpirun -np 4 ./mpi_filter input.png output_sobel.png sobel
```
**Windows**
```cmd
mpiexec -n 4 main_distributed.exe input.png output_sobel.png sobel
---

---

### 2️⃣ Gaussian Blur

**Parameters:**
- `ksize` - Kernel size (odd number: 3, 5, 7, 9, ...)
- `sigma` - Blur strength (0.8, 1.0, 1.5, 2.0, ...)

**Serial:**
```bash
./image_filter_serial input.png output_gaussian.png gaussian 5 1.0
```

**Parallel (4 threads):**
```bash
./image_filter_parallel input.png output_gaussian.png 4 gaussian 5 1.0
```

**MPI (4 processes):**
```bash
mpirun -np 4 ./mpi_filter input.png output_gaussian.png gaussian 5 1.0
```
**Windows**
```cmd
mpiexec -n 4 main_distributed.exe input.png output_gaussian.png gaussian 5 1.0
```

---

### 3️⃣ Laplacian Edge Enhancement

**Serial:**
```bash
./image_filter_serial input.png output_laplacian.png laplacian
```

**Parallel (4 threads):**
```bash
./image_filter_parallel input.png output_laplacian.png 4 laplacian
```

**MPI (4 processes):**
```bash
mpirun -np 4 ./mpi_filter input.png output_laplacian.png laplacian
```
**Windows**
```cmd
mpiexec -n 4 main_distributed.exe input.png output_laplacian.png laplacian
```

---

### 4️⃣ Sharpen Filter

**Serial:**
```bash
./image_filter_serial input.png output_sharpen.png sharpen
```

**Parallel (4 threads):**
```bash
./image_filter_parallel input.png output_sharpen.png 4 sharpen
```

**MPI (4 processes):**
```bash
mpirun -np 4 ./mpi_filter input.png output_sharpen.png sharpen
```
**Windows**
```cmd
mpiexec -n 4 main_distributed.exe input.png output_sharpen.png sharpen
---


## ⚡ Performance Benchmarks

### Serial

| Filter | Time |
|--------|------|
| Sobel | 0.016043s |
| Gaussian | 0.055719s |
| Laplacian | 0.053983s |
| Sharpen | 0.054240s |

### Parallel (4 threads)

| Filter | Time | Speedup |
|--------|------|---------|
| Sobel | 0.002268s | 7.07x |
| Gaussian | 0.010814s | 5.15x |
| Laplacian | 0.004109s | 13.14x |
| Sharpen | 0.004169s | 13.01x |

---
