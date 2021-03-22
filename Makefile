# ---------------------------------------------------------------
# https://www.rapidtables.com/code/linux/gcc/gcc-l.html <- how to link libs

CC := gcc

C_FLAGS :=

I_FLAGS :=
I_FLAGS += -Idoc

L_FLAGS :=

EXE:= main.exe

SOURCES := doc/doc.c doc/doc_json.c base64/base64.c
TEST_SOURCE := test.c
HEADERS := doc/doc.h doc/doc_json.h doc/doc_struct.h

VERSION := 1.4

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
TEST_OBJ := $(TEST_SOURCE:.c=.o)

# ---------------------------------------------------------------

.PHONY : build

build : C_FLAGS += -g
build : $(TEST_OBJ)
build : $(OBJS) main

release : C_FLAGS += -O2
release : $(OBJS) dist


%.o : %.c
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $(addprefix $(BUILD_DIR), $(notdir $@))


dist : $(OBJS_BUILD)
	$(ARCHIVER) $(DIST_DIR)$(LIB_NAME) $^
	cp $(HEADERS) $(DIST_DIR)


main: 
	$(CC) $(OBJS_BUILD) $(BUILD_DIR)/$(TEST_OBJ) -o $(EXE)


# pack : 
# 	$(TAR) $(DIST_NAME) 


clear : 
	@rm -f $(OBJS_BUILD)
	@rm -f $(BUILD_DIR)/$(TEST_OBJ)
	@rm -f $(DIST_DIR)$(LIB_NAME)
