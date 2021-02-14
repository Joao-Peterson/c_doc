# ---------------------------------------------------------------
# https://www.rapidtables.com/code/linux/gcc/gcc-l.html <- how to link libs

CC := gcc

C_FLAGS :=

I_FLAGS :=

L_FLAGS :=

SOURCES := doc/doc.c doc/doc_json.c base64/base64.c
HEADERS := doc/doc.h doc/doc_json.h

VERSION := 1.0

LIB_NAME := libdoc.a
DIST_NAME := C_doc_Win_x86_64_$(VERSION).tar.gz

DIST_DIR := dist/
BUILD_DIR := build/

ARCHIVER := ar -rcs

TAR := tar -c -v -z -f

# ---------------------------------------------------------------

# rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) \
#   $(filter $(subst *,%,$2),$d))

OBJS := $(SOURCES:.c=.o)
OBJS_BUILD := $(addprefix $(BUILD_DIR), $(notdir $(SOURCES:.c=.o)))

# ---------------------------------------------------------------

.PHONY : build

build : C_FLAGS += -g
build : $(OBJS)

release : C_FLAGS += -O3
release : $(OBJS) dist pack

%.o : %.c
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $(addprefix $(BUILD_DIR), $(notdir $@))


dist : $(OBJS_BUILD)
	$(ARCHIVER) $(DIST_DIR)$(LIB_NAME) $< 
	cp $(HEADERS) $(DIST_DIR)

# pack : 
# 	$(TAR) $(DIST_NAME) 

clear : 
	@rm -f $(OBJS_BUILD)
	@rm -f $(DIST_DIR)$(LIB_NAME)