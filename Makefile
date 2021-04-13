# ---------------------------------------------------------------
# https://www.rapidtables.com/code/linux/gcc/gcc-l.html <- how to link libs

CC := gcc

C_FLAGS :=

I_FLAGS :=
I_FLAGS += -Idoc

L_FLAGS :=

EXE:= main.exe

SOURCES := doc/doc.c base64/base64.c doc/doc_json.c doc/doc_xml.c doc/doc_ini.c doc/doc_print.c
TEST_SOURCE := test_ini.c
HEADERS := doc/doc.h doc/doc_struct.h doc/doc_json.h doc/doc_xml.h doc/doc_ini.h doc/doc_print.h

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
OBJS_BUILD := $(addprefix $(BUILD_DIR), $(OBJS))
TEST_OBJ := $(TEST_SOURCE:.c=.o)

# ---------------------------------------------------------------

.PHONY : build

build : C_FLAGS += -g
build : $(TEST_OBJ)
build : $(OBJS) main

release : C_FLAGS += -O2
release : $(OBJS) dist


%.o : %.c
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $(addprefix $(BUILD_DIR), $@)


dist : $(OBJS_BUILD)
	$(ARCHIVER) $(DIST_DIR)$(LIB_NAME) $^
	cp $(HEADERS) $(DIST_DIR)


main: 
	$(CC) $(OBJS_BUILD) $(BUILD_DIR)$(TEST_OBJ) -o $(EXE)


# pack : 
# 	$(TAR) $(DIST_NAME) 


clear : 
	@rm -f $(BUILD_DIR)/*
	@rm -f $(DIST_DIR)$(LIB_NAME)
