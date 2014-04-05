AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = trans
trans_MAIN = src/trans.cpp
trans_MODULES =	src/scanner/scanner.h src/scanner/scanner.cpp \
				src/scanner/state.h src/scanner/state.cpp src/scanner/token.h \
				src/parser/parser.h src/parser/parser.cpp src/parser/action.h src/parser/action.cpp \
				src/parser/item.h src/parser/item.cpp src/parser/rule.h src/parser/rule.cpp \
				src/parser/node.h src/parser/node.cpp src/parser/set_of_items.h src/parser/set_of_items.cpp \
				src/parser/grammar.h src/parser/grammar.cpp src/parser/parsing_table.h src/parser/parsing_table.cpp \ src/parser/nonterminal_node.h src/parser/nonterminal_node.cpp \ src/parser/terminal_node.h src/parser/terminal_node.cpp \ src/semantic_analyzer/decl_node.h src/semantic_analyzer/decl_node.cpp \
				src/semantic_analyzer/decls_node.h src/semantic_analyzer/decls_node.cpp \
				src/semantic_analyzer/log_and_expr_node.h src/semantic_analyzer/log_and_expr_node.cpp \
				src/semantic_analyzer/log_expr_node.h src/semantic_analyzer/log_expr_node.cpp \
				src/semantic_analyzer/dir_decl_node.h src/semantic_analyzer/dir_decl_node.cpp \
				src/semantic_analyzer/add_expr_node.h src/semantic_analyzer/add_expr_node.cpp \
				src/semantic_analyzer/postfix_expr_node.h src/semantic_analyzer/postfix_expr_node.cpp \
				src/semantic_analyzer/a_expressions_node.h src/semantic_analyzer/a_expressions_node.cpp \
				src/semantic_analyzer/loop_hdr_node.h src/semantic_analyzer/loop_hdr_node.cpp \
				src/semantic_analyzer/ptr_node.h src/semantic_analyzer/ptr_node.cpp \
				src/semantic_analyzer/a_expr_node.h src/semantic_analyzer/a_expr_node.cpp \
				src/semantic_analyzer/eq_expr_node.h src/semantic_analyzer/eq_expr_node.cpp \
				src/semantic_analyzer/matched_node.h src/semantic_analyzer/matched_node.cpp \
				src/semantic_analyzer/term_node.h src/semantic_analyzer/term_node.cpp \
				src/semantic_analyzer/and_expr_node.h src/semantic_analyzer/and_expr_node.cpp \
				src/semantic_analyzer/expr_node.h src/semantic_analyzer/expr_node.cpp \
				src/semantic_analyzer/ml_expr_node.h src/semantic_analyzer/ml_expr_node.cpp \
				src/semantic_analyzer/factor_node.h src/semantic_analyzer/factor_node.cpp \
				src/semantic_analyzer/u_expr_node.h src/semantic_analyzer/u_expr_node.cpp \
				src/semantic_analyzer/block_node.h src/semantic_analyzer/block_node.cpp \
				src/semantic_analyzer/func_decl_node.h src/semantic_analyzer/func_decl_node.cpp \
				src/semantic_analyzer/unmatched_node.h src/semantic_analyzer/unmatched_node.cpp \
				src/semantic_analyzer/carrier_node.h src/semantic_analyzer/carrier_node.cpp \
				src/semantic_analyzer/or_expr_node.h src/semantic_analyzer/or_expr_node.cpp \
				src/semantic_analyzer/var_decl_node.h src/semantic_analyzer/var_decl_node.cpp \
				src/semantic_analyzer/cast_expr_node.h src/semantic_analyzer/cast_expr_node.cpp \
				src/semantic_analyzer/io_stmt_node.h src/semantic_analyzer/io_stmt_node.cpp \
				src/semantic_analyzer/param_decl_node.h src/semantic_analyzer/param_decl_node.cpp \
				src/semantic_analyzer/s_expr_node.h src/semantic_analyzer/s_expr_node.cpp \
				src/semantic_analyzer/xor_expr_node.h src/semantic_analyzer/xor_expr_node.cpp \
				src/semantic_analyzer/param_list_node.h src/semantic_analyzer/param_list_node.cpp \
				src/semantic_analyzer/jmp_stmt_node.h src/semantic_analyzer/jmp_stmt_node.cpp \
				src/semantic_analyzer/syntax_tree.h src/semantic_analyzer/syntax_tree.cpp \
				src/code_generator/symbol_table.h src/code_generator/symbol_table.cpp \
				src/code_generator/symbol_entry.h src/code_generator/symbol_entry.cpp \
				src/code_generator/quadruple.h src/code_generator/quadruple.cpp \
                src/code_generator/register.h src/code_generator/register.cpp \
                src/code_generator/code_generator.h src/code_generator/code_generator.cpp \
                src/util/argv_parser.h  src/util/argv_parser.cpp 

noinst_LTLIBRARIES = libtrans.la
libtrans_la_SOURCES = $(trans_MODULES)
trans_SOURCES = $(trans_MAIN)
trans_LDADD = libtrans.la

dist_noinst_SCRIPTS = autogen.sh


AM_DEFAULT_SOURCE_EXT = .cpp

noinst_PROGRAMS = gtest
check_PROGRAMS = gtest
TESTS = gtest

gtest_SOURCES = test/gtest.cpp test/dummy_test.cpp test/dummy_test2.cpp test/util/argv_parser_test.cpp
gtest_LDADD = test/lib/gtest/lib/libgtest.a libtrans.la -lpthread
gtest_CXXFLAGS = -Itest/lib/gtest/include -Isrc

