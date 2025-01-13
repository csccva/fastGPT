# Set compiler and flags
FC=ftn
FFLAGS="-fopenmp -O3"

# Clean up previous builds
rm -f *.o libfastgpt.a gpt2 chat test_basic_input test_more_inputs test_chat

# Compile Fortran source files
$FC $FFLAGS -c linalg_f.f90 -o linalg_f.o
$FC $FFLAGS -c tokenizer.f90 -o tokenizer.o
$FC $FFLAGS -c gpt2.f90 -o gpt2.o
$FC $FFLAGS -c omp.f90 -o omp.o
$FC $FFLAGS -c driver.f90 -o driver.o

# Create static library
ar rcs libfastgpt.a linalg_f.o tokenizer.o gpt2.o omp.o driver.o

# Link the executables
$FC $FFLAGS main.f90 gpt2.o -L. -lfastgpt -o gpt2
$FC $FFLAGS chat.f90 driver.o -L. -lfastgpt -o chat
$FC $FFLAGS tests/test_basic_input.f90 linalg_f.o tokenizer.o driver.o -L. -lfastgpt -o test_basic_input
$FC $FFLAGS tests/test_more_inputs.f90 linalg_f.o tokenizer.o driver.o -L. -lfastgpt -o test_more_inputs
$FC $FFLAGS tests/test_chat.f90 linalg_f.o tokenizer.o driver.o gpt2.o -L. -lfastgpt -o test_chat
