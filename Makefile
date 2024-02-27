CC = clang++-16
CXXFLAGS = -std=c++2b -g -Og -DYYDEBUG=1
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
  lib/parsetree/VisitStatement.cc tools/genfrags/FragmentGenerator.cc \
  lib/utils/DotPrinter.cc lib/semantic/NameResolver.cc lib/semantic/HierarchyChecker.cc \
  lib/passes/CompilerPasses.cc lib/utils/PassManager.cc tools/joosc/main.cc \
  ${LEXER_OUT} ${PARSER_OUT}

all: joosc

joosc:
	mkdir -p $(PARSER_DIR)
	$(FLEX) --outfile=$(LEXER_OUT) --header-file=$(LEXER_HEADER) lib/grammar/joos1w_lexer.l
	$(BISON) $(BISONFLAGS) --output=$(PARSER_OUT) --defines=$(PARSER_HEADER) lib/grammar/joos1w_parser.y
	$(CC) $(CXXFLAGS) $(INCLUDES) $(JOOSC_SRCS) -o $@

clean:
	rm -rf $(PARSER_DIR) && rm joosc

.PHONY: all clean joosc
