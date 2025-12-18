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


# 🚀 OpenMP Parallelization Implementation Guide

> **Step-by-Step Guide to Parallelize Image Filter Program**  
> Transform serial code into high-performance parallel implementation

---

## 📋 Table of Contents

- [Implementation Roadmap](#-implementation-roadmap)
- [Phase 1: Setup](#phase-1-setup-do-first)
- [Phase 2: Parallelize Functions](#phase-2-parallelize-functions-easiest--hardest)
- [Phase 3: Optimization](#phase-3-optimization-optional)
- [Testing Strategy](#-testing-strategy)
- [Common Mistakes](#️-common-mistakes-to-avoid)
- [Implementation Order](#-implementation-order)

---

## 🎯 Implementation Roadmap

### Progress Checklist

- [ ] **Phase 1:** Setup Thread Count Parameter
- [ ] **Phase 2:** Parallelize Functions
  - [ ] Step 2: `to_grayscale()`
  - [ ] Step 3: `sobel()`
  - [ ] Step 4: `build_gaussian()`
  - [ ] Step 5: `convolve_rgb()`
- [ ] **Phase 3:** Optimization with Scheduling

---

## Phase 1: SETUP (Do First)

### **STEP 1: Add Thread Count Parameter**

#### Changes Required:
1. **Modify `argc` check:** `< 4` → `< 5`
2. **Add variable:** `int thread_count` in main
3. **Parse thread count:**
   ```c
   thread_count = strtol(argv[3], NULL, 10)
   ```
4. **Set OpenMP threads:**
   ```c
   omp_set_num_threads(thread_count)
   ```
5. **Shift argv indices:**
   - `mode` → `argv[3]` becomes `argv[4]`
   - Gaussian `ksize` → `argv[4]` becomes `argv[5]`
   - Gaussian `sigma` → `argv[5]` becomes `argv[6]`
6. **Update Gaussian check:** `argc < 6` → `argc < 7`

#### New Command Format:
```bash
./image_filter_parallel input.png output.png <num_threads> <filter> [params]
```

#### Testing:
```bash
# Compile
gcc-15 main_parallel.c -o image_filter_parallel -fopenmp -lm

# Test with different thread counts
./image_filter_parallel input.png out.png 1 sobel
./image_filter_parallel input.png out.png 2 sobel
./image_filter_parallel input.png out.png 4 sobel
./image_filter_parallel input.png out.png 8 sobel
```

**Expected Result:** ✅ All commands work, but no speedup yet (that's normal!)

---

## Phase 2: PARALLELIZE FUNCTIONS (Easiest → Hardest)

### **STEP 2: Parallelize `to_grayscale()` - ⭐ EASIEST**

#### 🎯 Why Easiest?
- ✅ Single loop, no nested loops
- ✅ No race conditions (each thread writes to different index)
- ✅ No reduction needed

#### 📝 What to Add:

**Before this loop:**
```c
for(int i=0; i<w*h; i++)
```

**Add this directive:**
```c
#pragma omp parallel for default(none) shared(img, g, w, h, ch)
```

#### 🧩 Variable Analysis:

| Variable | Type | Reason |
|----------|------|--------|
| `img` | shared | Read-only input |
| `g` | shared | Each thread writes different index |
| `w, h, ch` | shared | Read-only constants |
| `i, r, g1, b` | private | Automatic with `parallel for` |

---

### **STEP 3: Parallelize `sobel()` - ⭐⭐ MEDIUM**

#### 🎯 Why Medium?
- ⚠️ Nested loops (y, x)
- ✅ Need `collapse(2)` to parallelize both
- ✅ Still no race conditions

#### 📝 What to Add:

**Before this loop:**
```c
for(int y=0; y<h; y++) {
    for(int x=0; x<w; x++) {
```

**Add this directive:**
```c
#pragma omp parallel for collapse(2) default(none) shared(g, out, w, h, ch, gx, gy)
```

#### 🧩 Variable Analysis:

| Variable | Type | Reason |
|----------|------|--------|
| `g` | shared | Read-only (from `to_grayscale`) |
| `out` | shared | Each thread writes different index |
| `gx, gy` | shared | Read-only arrays |
| `y, x, sx, sy, ky, kx, xx, yy, val, idx, mag, c` | private | Automatic |

#### ⚠️ Important:
**Don't parallelize inner loops** (`ky`, `kx`, `c`) - they're part of the computation!

---

### **STEP 4: Parallelize `build_gaussian()` - ⭐⭐⭐ TWO PARTS**

#### **Part A: Kernel Generation Loop - NEEDS REDUCTION ⚡**

#### 🎯 Why Needs Reduction?
- ⚠️ Multiple threads adding to same `sum` variable
- ❌ Without reduction → **RACE CONDITION!**

#### 📝 What to Add:

**Before this loop:**
```c
for(int y=-half; y<=half; y++){
    for(int x=-half; x<=half; x++){
```

**Add this directive:**
```c
#pragma omp parallel for collapse(2) reduction(+:sum) default(none) shared(k, ksize, half, sigma)
```

#### 🧩 Variable Analysis:

| Variable | Type | Reason |
|----------|------|--------|
| `sum` | **reduction** | ⚡ All threads accumulate to it |
| `k` | shared | Each thread writes different index |
| `ksize, half, sigma` | shared | Read-only |
| `y, x, v` | private | Automatic |

#### 🔑 Key Concept:
```
reduction(+:sum) means:
1. Each thread gets private copy of sum
2. Each thread accumulates in its copy
3. At end, all copies are combined with + operator
4. Result stored in original sum variable
```

---

#### **Part B: Normalization Loop - SIMPLE**

#### 📝 What to Add:

**Before this loop:**
```c
for(int i=0; i<ksize*ksize; i++)
```

**Add this directive:**
```c
#pragma omp parallel for default(none) shared(k, ksize, sum)
```

---

### **STEP 5: Parallelize `convolve_rgb()` - ⭐⭐⭐⭐ MOST COMPLEX**

#### 🎯 Why Most Complex?
- ⚠️ Triple nested loops (y, x, c)
- ⚠️ Only parallelize y and x (NOT c!)
- ⚠️ Many variables to manage

#### 📝 What to Add:

**Before this loop:**
```c
for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
```

**Add this directive:**
```c
#pragma omp parallel for collapse(2) default(none) shared(in, out, w, h, channels, kernel, ksize, half)
```

#### 🧩 Variable Analysis:

| Variable | Type | Reason |
|----------|------|--------|
| `in, out` | shared | Input/output arrays |
| `kernel` | shared | Read-only filter |
| `w, h, channels, ksize, half` | shared | Read-only constants |
| All other variables | private | Automatic |

#### ⚠️ Critical:
**Don't parallelize the c, ky, kx loops!** Too small, overhead > benefit

---

## Phase 3: OPTIMIZATION (Optional)

### **STEP 6: Add Schedule Clauses**

Try different scheduling strategies and measure performance!

---

#### **Option 1: Static Scheduling (Default)** ⚖️

```c
schedule(static)
// or
schedule(static, chunk_size)
```

**When to use:**
- ✅ All iterations take similar time
- ✅ Predictable workload
- ✅ Lowest overhead

---

#### **Option 2: Dynamic Scheduling** 🔄

```c
schedule(dynamic)
// or
schedule(dynamic, chunk_size)
```

**When to use:**
- ✅ Iterations vary in execution time
- ✅ Unpredictable workload
- ✅ Better load balancing

---

#### **Option 3: Guided Scheduling** 🎯

```c
schedule(guided)
// or
schedule(guided, chunk_size)
```

**When to use:**
- ✅ Start with large chunks, decrease size
- ✅ Balances load and overhead
- ✅ Good compromise

---

#### Where to Add Scheduling:

**In `sobel()` parallel loop:**
```c
#pragma omp parallel for collapse(2) schedule(dynamic) default(none) shared(...)
```

**In `convolve_rgb()` parallel loop:**
```c
#pragma omp parallel for collapse(2) schedule(guided) default(none) shared(...)
```

---

#### 🧪 Experiment Plan:

| Function | Test Schedules |
|----------|----------------|
| `sobel()` | static, dynamic, guided |
| `convolve_rgb()` | static, dynamic, guided |

**Measure which gives best performance for YOUR hardware!**

---

## 📊 Testing Strategy

### After Each Step:

#### 1️⃣ **Compile:**
```bash
gcc-15 main_parallel.c -o image_filter_parallel -fopenmp -lm
```

#### 2️⃣ **Run Baseline (1 thread):**
```bash
./image_filter_parallel input.png out1.png 1 sobel
```

#### 3️⃣ **Run with Multiple Threads:**
```bash
./image_filter_parallel input.png out2.png 2 sobel
./image_filter_parallel input.png out4.png 4 sobel
./image_filter_parallel input.png out8.png 8 sobel
```

#### 4️⃣ **Verify Output:**
- ✅ Check if output images are correct
- ✅ Compare with serial version output

#### 5️⃣ **Compare Execution Times:**
```
1 thread:  X.XXX seconds (baseline)
2 threads: X.XXX seconds (speedup: X.Xx)
4 threads: X.XXX seconds (speedup: X.Xx)
8 threads: X.XXX seconds (speedup: X.Xx)
```

---

## ⚠️ Common Mistakes to Avoid

### 🚫 **Mistake 1: Parallelizing Inner Convolution Loops**
```c
// ❌ WRONG - Don't do this!
for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++) {
        #pragma omp parallel for  // ❌ BAD!
        for(int ky = -half; ky <= half; ky++) {
```
**Why wrong:** Loops too small, overhead > benefit

---

### 🚫 **Mistake 2: Parallelizing Channel Loop**
```c
// ❌ WRONG - Don't do this!
#pragma omp parallel for  // ❌ BAD!
for(int c = 0; c < channels; c++) {
```
**Why wrong:** Only 3 iterations (RGB), overhead >> benefit

---

### 🚫 **Mistake 3: Forgetting Reduction**
```c
// ❌ WRONG - Race condition!
#pragma omp parallel for
for(int y=-half; y<=half; y++){
    for(int x=-half; x<=half; x++){
        sum += v;  // ❌ Multiple threads writing to sum!
    }
}

// ✅ CORRECT
#pragma omp parallel for reduction(+:sum)
```

---

### 🚫 **Mistake 4: Not Using collapse(2)**
```c
// ⚠️ SUBOPTIMAL - Only parallelizing outer loop
#pragma omp parallel for
for(int y=0; y<h; y++) {
    for(int x=0; x<w; x++) {

// ✅ BETTER - Parallelizing both loops
#pragma omp parallel for collapse(2)
for(int y=0; y<h; y++) {
    for(int x=0; x<w; x++) {
```

---

### 🚫 **Mistake 5: Not Using default(none)**
```c
// ⚠️ RISKY - Variable scoping bugs hidden
#pragma omp parallel for

// ✅ SAFE - Forces explicit variable declaration
#pragma omp parallel for default(none) shared(...) 
```

---

## 🎯 Implementation Order

```mermaid
graph LR
    A[Start] --> B[STEP 1: Setup]
    B --> C[Test]
    C --> D[STEP 2: to_grayscale]
    D --> E[Test]
    E --> F[STEP 3: sobel]
    F --> G[Test]
    G --> H[STEP 4: build_gaussian]
    H --> I[Test]
    I --> J[STEP 5: convolve_rgb]
    J --> K[Test]
    K --> L[STEP 6: Optimize]
    L --> M[Done!]
```

### Recommended Flow:

1. ✅ **STEP 1** → Setup Thread Count
2. ✅ **STEP 2** → Parallelize `to_grayscale()` → **Test**
3. ✅ **STEP 3** → Parallelize `sobel()` → **Test**
4. ✅ **STEP 4** → Parallelize `build_gaussian()` → **Test**
5. ✅ **STEP 5** → Parallelize `convolve_rgb()` → **Test**
6. ✅ **STEP 6** → Add Scheduling → **Test & Compare**

---

## 📈 Expected Speedup

### Theoretical Speedup (Amdahl's Law):

| Threads | Ideal Speedup | Realistic Speedup |
|---------|---------------|-------------------|
| 1 | 1.0x | 1.0x |
| 2 | 2.0x | 1.7-1.9x |
| 4 | 4.0x | 3.0-3.5x |
| 8 | 8.0x | 5.0-6.5x |

**Note:** Actual speedup depends on:
- Image size (larger = better parallelization)
- CPU architecture
- Memory bandwidth
- Cache effects

---

## 🎓 Key OpenMP Concepts Recap

### Directives Used:

| Directive | Purpose |
|-----------|---------|
| `#pragma omp parallel for` | Parallelize loop |
| `collapse(N)` | Parallelize N nested loops |
| `reduction(+:var)` | Safe accumulation across threads |
| `default(none)` | Force explicit variable scoping |
| `shared(...)` | Variables shared among threads |
| `schedule(...)` | Control work distribution |

---

## 🏁 Final Checklist

Before considering implementation complete:

- [ ] All 5 functions parallelized
- [ ] Tested with 1, 2, 4, 8 threads
- [ ] Output images verified correct
- [ ] Speedup measurements recorded
- [ ] Schedule optimization attempted
- [ ] No race conditions (check with larger images)
- [ ] Code compiles without warnings

---

## 🎉 Success!

Once all steps are complete, you should see:
- ✅ 3-6x speedup with 8 threads
- ✅ Correct output images
- ✅ Understanding of OpenMP parallelization
- ✅ Production-ready parallel image filter!

---

# 💻 Hardware Specifications

```
Total Cores:        10
├── P-Cores:        4 @ 4.4 GHz
└── E-Cores:        6 @ 2.8 GHz

Cache:              16 MB L2 (shared)
Memory:             16 GB Unified
Bandwidth:          120 GB/s
```

---

## 👤 Author

**saffisardar and saadali**

---