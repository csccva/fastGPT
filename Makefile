# Set default target system
ifeq ($(COMP),)
    COMP = lumi_cray
endif

# Select compiler based on target system
ifeq ($(COMP),gnu)
    FC = gfortran
	FFLAGS = -fopenmp -O2
endif
ifeq ($(COMP),lumi_cray)
    FC = ftn 
	FFLAGS =-O2  -fopenmp  
endif
ifeq ($(COMP),lumi_amd_new)
    FC = amdflang-new 
	FFLAGS = -O2  -fopenmp -fopenmp-targets=amdgcn-amd-amdhsa --offload-arch=gfx90a
endif



# Sources and objects
SRCS = linalg_f.f90 tokenizer.f90 gpt2.f90 omp.f90 driver.f90
OBJS = $(SRCS:.f90=.o)

# Static library
LIB = libfastgpt.a

# Executables and their dependencies
EXECUTABLES = gpt2 chat test_basic_input test_more_inputs test_chat

# Default target
all: $(EXECUTABLES)

# Compile source files
%.o: %.f90
	$(FC) $(FFLAGS) -c $< -o $@

# Static library
$(LIB): $(OBJS)
	ar rcs $@ $^

# Executables
gpt2: main.f90 $(LIB) gpt2.o
	$(FC) $(FFLAGS) $< gpt2.o -L. -lfastgpt -o $@

chat: chat.f90 $(LIB) driver.o
	$(FC) $(FFLAGS) $< driver.o -L. -lfastgpt -o $@

test_basic_input: tests/test_basic_input.f90 $(LIB) linalg_f.o tokenizer.o driver.o
	$(FC) $(FFLAGS) $< linalg_f.o tokenizer.o driver.o -L. -lfastgpt -o $@

test_more_inputs: tests/test_more_inputs.f90 $(LIB) linalg_f.o tokenizer.o driver.o
	$(FC) $(FFLAGS) $< linalg_f.o tokenizer.o driver.o -L. -lfastgpt -o $@

test_chat: tests/test_chat.f90 $(LIB) linalg_f.o tokenizer.o driver.o gpt2.o
	$(FC) $(FFLAGS) $< linalg_f.o tokenizer.o driver.o gpt2.o -L. -lfastgpt -o $@

# Clean target
clean:
	rm -f *.o $(LIB) $(EXECUTABLES) *.mod

.PHONY: all clean
