CC = clang
CXXFLAGS = -std=c++23 -g -Og -DYYDEBUG=1
FLEX = flex
BISON = bison
BISONFLAGS = --locations -k
INCLUDES = -Ilib -I$(PARSER_DIR)
PARSER_DIR = ./joosc_build
LEXER_OUT = $(PARSER_DIR)/joos1w_lexer.cpp
PARSER_OUT = $(PARSER_DIR)/joos1w_parser.cpp
LEXER_HEADER = $(PARSER_DIR)/joos1w.lexer.do.not.use.h
PARSER_HEADER = $(PARSER_DIR)/joos1w.parser.tab.h

# Joosc specific sources and their dependencies
JOOSC_SRCS = \
  lib/ast/Decl.cc lib/ast/DeclContext.cc lib/ast/Expr.cc \
  lib/ast/Stmt.cc lib/ast/Modifiers.cc lib/semantic/Semantic.cc \
  lib/grammar/Joos1W.cc lib/parsetree/ParseTree.cc \
  lib/parsetree/ParseTreeVisitor.cc lib/parsetree/VisitClassInterface.cc \
  lib/parsetree/VisitExpression.cc lib/parsetree/VisitLeaf.cc \
  lib/parsetree/VisitStatement.cc lib/utils/FragmentGenerator.cc \
  lib/utils/DotPrinter.cc tools/joosc/main.cc {LEXER_OUT} {PARSER_OUT} \

JOOSC_OBJS = $(JOOSC_SRCS:%.cc=$(PARSER_DIR)/%.o)

all: joosc

$(JOOSC_OBJS): $(LEXER_OUT) $(PARSER_OUT)

$(PARSER_DIR)/%.o: %.cc
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(PARSER_OUT): lib/grammar/joos1w_parser.y
	mkdir -p $(PARSER_DIR)
	$(BISON) $(BISONFLAGS) --output=$(PARSER_OUT) --defines=$(PARSER_HEADER) $<

$(LEXER_OUT): lib/grammar/joos1w_lexer.l
	mkdir -p $(PARSER_DIR)
	$(FLEX) --outfile=$(LEXER_OUT) --header-file=$(LEXER_HEADER) $<

joosc: $(JOOSC_OBJS)
	$(CC) $(CXXFLAGS) $(INCLUDES) -o ./$@ $^

clean:
	rm -rf $(PARSER_DIR)

.PHONY: all clean joosc
