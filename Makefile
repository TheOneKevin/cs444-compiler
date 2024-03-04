CC = clang++-16
CXXFLAGS = -std=c++2b -O3
FLEX = flex
BISON = bison
BISONFLAGS = --locations -k
BUILD_DIR = ./build1

INCLUDES = -Iinclude -I$(BUILD_DIR)
LEXER_HEADER = $(BUILD_DIR)/joos1w.lexer.do.not.use.h
LEXER_OBJ = $(BUILD_DIR)/joos1w_lexer.o
PARSER_HEADER = $(BUILD_DIR)/joos1w.parser.tab.h
PARSER_OBJ = $(BUILD_DIR)/joos1w_parser.o

# Recursive glob .cc files under lib
JOOSC_SRCS = $(shell find lib -name '*.cc') tools/joosc/main.cc
JOOSC_OBJS = $(patsubst %.cc,%.o,$(JOOSC_SRCS))
JOOSC_OBJS := $(addprefix $(BUILD_DIR)/,$(JOOSC_OBJS))

all: joosc

joosc: $(JOOSC_OBJS) $(LEXER_OBJ) $(PARSER_OBJ)
	@echo "Linking joosc"
	@$(CC) $(CXXFLAGS) -o $@ $^

$(LEXER_OBJ): lib/grammar/joos1w_lexer.l
	@echo "Building lexer"
	@mkdir -p $(BUILD_DIR)
	@$(FLEX) --outfile=lexer.cpp --header-file=$(LEXER_HEADER) $<
	@$(CC) $(CXXFLAGS) $(INCLUDES) -c -o $@ lexer.cpp

$(PARSER_OBJ): lib/grammar/joos1w_parser.y
	@echo "Building parser"
	@mkdir -p $(BUILD_DIR)
	@$(BISON) $(BISONFLAGS) --output=parser.cpp --defines=$(PARSER_HEADER) $<
	@$(CC) $(CXXFLAGS) $(INCLUDES) -c -o $@ parser.cpp

generated_sources: $(LEXER_OBJ) $(PARSER_OBJ)

$(BUILD_DIR)/%.o: %.cc generated_sources
	@echo "Building $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	@rm -rf $(BUILD_DIR) && rm -f joosc

.PHONY: all clean
