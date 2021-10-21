# ---------------------------------------------------------------
# https://www.rapidtables.com/code/linux/gcc/gcc-l.html <- how to link libs
# 
# Commands:
# 	build 		: build lib objects and test file for testing 
# 	release 	: build lib objects, archive and organize the lib files for use in the 'dist/' folder
# 	dist 		: dist just organizes the lib files for use in the 'dist/' folder
# 	clear 		: clear compiled executables
# 	clearall 	: clear compiled objects and lib files in 'build/' and 'dist/' folders as well as executables

CC := gcc

C_FLAGS :=

I_FLAGS :=
I_FLAGS += -Idoc

L_FLAGS :=

EXE:= main.exe

TEST_SOURCE := test.c

SOURCES := doc/doc.c doc/base64.c doc/doc_json.c doc/doc_xml.c doc/doc_ini.c 
SOURCES += doc/doc_csv.c doc/doc_print.c doc/parse_utils.c

HEADERS := doc/doc.h doc/doc_json.h doc/doc_xml.h doc/doc_ini.h 
HEADERS += doc/doc_csv.h doc/doc_print.h doc/parse_utils.h doc/base64.h

LIB_NAME := libdoc.a

DIST_DIR := dist/
BUILD_DIR := build/

ARCHIVER := ar -rcs

TAR := tar -c -v -z -f

# ---------------------------------------------------------------

# rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) \
#   $(filter $(subst *,%,$2),$d))

OBJS := $(SOURCES:.c=.o)
OBJS_BUILD := $(addprefix $(BUILD_DIR), $(OBJS))
TEST_OBJ := $(BUILD_DIR)$(TEST_SOURCE:.c=.o)

# MAKEFLAGS += --jobs=$(shell nproc)
# MAKEFLAGS += --output-sync=target

# ---------------------------------------------------------------

.PHONY : build

build : C_FLAGS += -g
build : $(HEADERS)
build : $(TEST_OBJ)
build : $(OBJS_BUILD) $(EXE)

release : C_FLAGS += -O2
release : $(HEADERS)
release : clearall $(OBJS_BUILD) dist

$(BUILD_DIR)%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $@


dist : $(OBJS_BUILD)
	@mkdir -p $(DIST_DIR)
	$(ARCHIVER) $(DIST_DIR)$(LIB_NAME) $^
	cp $(HEADERS) $(DIST_DIR)


$(EXE): $(OBJS_BUILD) $(TEST_OBJ)
	$(CC) $^ -o $@

clear : 
	rm -f $(EXE)

clearall : clear
	rm -f -r $(BUILD_DIR)*
	rm -f -r $(DIST_DIR)*
