On Mac:

mkdir -p build
gcc-15 src/main.c -Iinclude -fopenmp -lm -o build/image_filter_serial
gcc-15 src/main_parallel.c -Iinclude -fopenmp -lm -o build/image_filter_parallel
OMPI_CC=gcc-15 mpicc src/main_distributed.c -Iinclude -fopenmp -lm -o build/mpi_filter
gcc-15 src/app.c -Iinclude -fopenmp -lm -o build/app_runner
cd build
./app_runner

On Windows:

