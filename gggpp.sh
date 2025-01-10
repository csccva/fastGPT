# Set compiler and flags
FC=gfortran
FFLAGS="-fopenmp -pg"  # Added -pg for profiling

# Clean up previous builds
rm -f *.o libfastgpt.a gpt2 chat test_basic_input test_more_inputs test_chat

# Compile Fortran source files
$FC $FFLAGS -c linalg_f.f90 -o ./linalg_f.f90.o
$FC $FFLAGS -c tokenizer.f90 -o ./tokenizer.f90.o
$FC $FFLAGS -c gpt2.f90 -o ./gpt2.f90.o
$FC $FFLAGS -c omp.f90 -o ./omp.f90.o
$FC $FFLAGS -c driver.f90 -o ./driver.f90.o

# Create static library
ar rcs libfastgpt.a ./linalg_f.f90.o ./tokenizer.f90.o ./gpt2.f90.o ./omp.f90.o ./driver.f90.o

# Link the executables with profiling enabled
$FC $FFLAGS main.f90 ./gpt2.f90.o -L. -lfastgpt -o gpt2
$FC $FFLAGS chat.f90 ./driver.f90.o -L. -lfastgpt -o chat
$FC $FFLAGS tests/test_basic_input.f90 ./linalg_f.f90.o ./tokenizer.f90.o ./driver.f90.o -L. -lfastgpt -o test_basic_input
$FC $FFLAGS tests/test_more_inputs.f90 ./linalg_f.f90.o ./tokenizer.f90.o ./driver.f90.o -L. -lfastgpt -o test_more_inputs
$FC $FFLAGS tests/test_chat.f90 ./linalg_f.f90.o ./tokenizer.f90.o ./driver.f90.o ./gpt2.f90.o -L. -lfastgpt -o test_chat

# Run the program to generate profiling data
srun  --time=00:15:00 --partition=test --account=project_2001659  --ntasks=1 -c 1 ./gpt2  # Or any other executable you'd like to profile

# After running the program, analyze with gprof
srun  --time=00:15:00 --partition=test --account=project_2001659  --ntasks=1 -c 1 gprof ./gpt2 gmon.out > analysis.txt  # Replace with relevant executable if necessary
