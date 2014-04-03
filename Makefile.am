AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = trans
trans_SOURCES = src/trans.cpp \
				src/scanner/scanner.h src/scanner/scanner.cpp \
				src/scanner/state.h src/scanner/state.cpp src/scanner/token.h \
				src/parser/parser.h src/parser/parser.cpp src/parser/action.h src/parser/action.cpp \
				src/parser/item.h src/parser/item.cpp src/parser/rule.h src/parser/rule.cpp \
				src/parser/node.h src/parser/node.cpp src/parser/set_of_items.h src/parser/set_of_items.cpp \
				src/parser/grammar.h src/parser/grammar.cpp src/parser/parsing_table.h src/parser/parsing_table.cpp \
				src/parser/nonterminal_node.h src/parser/nonterminal_node.cpp \
				src/parser/terminal_node.h src/parser/terminal_node.cpp \
				    src/decl_node.h src/log_and_expr_node.h src/symbol_table.h src/add_expr_node.h \
                src/decls_node.h src/log_expr_node.h src/postfix_expr_node.h src/syntax_tree.h src/a_expressions_node.h src/dir_decl_node.h \
                src/loop_hdr_node.h src/ptr_node.h  src/a_expr_node.h src/eq_expr_node.h src/matched_node.h src/quadruple.h \
                src/term_node.h src/and_expr_node.h src/expr_node.h src/ml_expr_node.h src/register.h src/argv_parser.h src/factor_node.h \
                src/u_expr_node.h src/block_node.h src/func_decl_node.h src/unmatched_node.h \
                src/carrier_node.h src/or_expr_node.h src/var_decl_node.h src/cast_expr_node.h src/io_stmt_node.h \
                src/param_decl_node.h src/s_expr_node.h src/xor_expr_node.h src/code_generator.h src/param_list_node.h  \
                src/config.h src/jmp_stmt_node.h src/symbol_entry.h \
                              src/eq_expr_node.cpp                       \
                src/add_expr_node.cpp       src/expr_node.cpp             src/s_expr_node.cpp \
                src/a_expressions_node.cpp  src/factor_node.cpp        src/or_expr_node.cpp        \
                src/a_expr_node.cpp         src/func_decl_node.cpp     src/param_decl_node.cpp    src/symbol_entry.cpp \
                src/and_expr_node.cpp                   src/param_list_node.cpp    src/symbol_table.cpp \
                src/argv_parser.cpp         src/io_stmt_node.cpp                   src/syntax_tree.cpp \
                src/block_node.cpp                                \
                src/carrier_node.cpp        src/jmp_stmt_node.cpp      src/postfix_expr_node.cpp  src/term_node.cpp \
                src/cast_expr_node.cpp      src/log_and_expr_node.cpp  src/ptr_node.cpp            \
                src/code_generator.cpp      src/log_expr_node.cpp      src/quadruple.cpp          src/u_expr_node.cpp \
                src/decl_node.cpp           src/loop_hdr_node.cpp      src/register.cpp           src/unmatched_node.cpp \
                src/decls_node.cpp          src/matched_node.cpp                     src/var_decl_node.cpp \
                src/dir_decl_node.cpp       src/ml_expr_node.cpp       src/xor_expr_node.cpp 


dist_noinst_SCRIPTS = autogen.sh
