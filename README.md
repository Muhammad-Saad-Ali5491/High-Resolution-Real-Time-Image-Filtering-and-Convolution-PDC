On Mac:

mkdir -p build
gcc-15 src/main.c -Iinclude -fopenmp -lm -o build/image_filter_serial
gcc-15 src/main_parallel.c -Iinclude -fopenmp -lm -o build/image_filter_parallel
OMPI_CC=gcc-15 mpicc src/main_distributed.c -Iinclude -fopenmp -lm -o build/mpi_filter
gcc-15 src/app.c -Iinclude -fopenmp -lm -o build/app_runner
cd build
./app_runner

On Windows:
gcc src/app.c -o build/app_runner.exe
gcc src/main.c -o build/image_filter_serial -fopenmp -lm
gcc src/main_parallel.c -o build/image_filter_parallel -fopenmp -lm

to create mpi_filter in wondows open file the in vscode goto terminal->run build task after that rename the file 
main_distributed.exe => mpi_filter.exe 
