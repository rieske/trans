all: trans

OBJS = trans.o argv_parser.o $(SCANNER) $(PARSER) $(SYNTAX) $(CODE)

SCANNER = scanner.o state.o
PARSER = parser.o parsing_table.o grammar.o item.o rule.o set_of_items.o action.o 
SYNTAX = symbol_table.o symbol_entry.o node.o terminal_node.o nonterminal_node.o syntax_tree.o carrier_node.o \
		 dir_decl_node.o decl_node.o ptr_node.o decls_node.o var_decl_node.o term_node.o postfix_expr_node.o \
		 u_expr_node.o cast_expr_node.o factor_node.o add_expr_node.o s_expr_node.o ml_expr_node.o \
		 eq_expr_node.o and_expr_node.o xor_expr_node.o or_expr_node.o log_and_expr_node.o log_expr_node.o \
		 a_expr_node.o expr_node.o func_decl_node.o a_expressions_node.o param_list_node.o param_decl_node.o \
		 jmp_stmt_node.o io_stmt_node.o loop_hdr_node.o unmatched_node.o matched_node.o block_node.o
CODE = quadruple.o code_generator.o register.o

CC = g++
CL = g++
DEBUG = -g
CFLAGS = -c -Wall $(DEBUG)
LFLAGS = -Wall $(DEBUG)

.PHONY: clean

trans: $(OBJS) 
	$(CL) $(LFLAGS) -o trans $(OBJS)

trans.o: trans.cpp scanner.h token.h
	$(CC) $(CFLAGS) trans.cpp 

argv_parser.o: argv_parser.cpp argv_parser.h scanner.h parser.h
	$(CC) $(CFLAGS) argv_parser.cpp

scanner.o: scanner.cpp scanner.h state.h token.h
	$(CC) $(CFLAGS) scanner.cpp

state.o: state.cpp state.h
	$(CC) $(CFLAGS) state.cpp

parser.o: parser.cpp parser.h parsing_table.h scanner.h node.h
	$(CC) $(CFLAGS) parser.cpp

parsing_table.o: parsing_table.cpp parsing_table.h
	$(CC) $(CFLAGS) parsing_table.cpp

grammar.o: grammar.cpp grammar.h
	$(CC) $(CFLAGS) grammar.cpp

item.o: item.cpp item.h
	$(CC) $(CFLAGS) item.cpp

rule.o: rule.cpp rule.h
	$(CC) $(CFLAGS) rule.cpp

set_of_items.o: set_of_items.cpp set_of_items.h
	$(CC) $(CFLAGS) set_of_items.cpp

action.o: action.cpp action.h
	$(CC) $(CFLAGS) action.cpp

node.o: node.cpp node.h
	$(CC) $(CFLAGS) node.cpp

nonterminal_node.o: nonterminal_node.cpp nonterminal_node.h node.h
	$(CC) $(CFLAGS) nonterminal_node.cpp

terminal_node.o: terminal_node.cpp terminal_node.h node.h
	$(CC) $(CFLAGS) terminal_node.cpp

carrier_node.o: carrier_node.cpp carrier_node.h nonterminal_node.h
	$(CC) $(CFLAGS) carrier_node.cpp

dir_decl_node.o: dir_decl_node.cpp dir_decl_node.h nonterminal_node.h
	$(CC) $(CFLAGS) dir_decl_node.cpp

decl_node.o: decl_node.cpp decl_node.h dir_decl_node.h
	$(CC) $(CFLAGS) decl_node.cpp

ptr_node.o: ptr_node.cpp ptr_node.h nonterminal_node.h
	$(CC) $(CFLAGS) ptr_node.cpp

decls_node.o: decls_node.cpp decls_node.h nonterminal_node.h
	$(CC) $(CFLAGS) decls_node.cpp

var_decl_node.o: var_decl_node.cpp var_decl_node.h nonterminal_node.h carrier_node.h decls_node.h
	$(CC) $(CFLAGS) var_decl_node.cpp

term_node.o: term_node.cpp term_node.h expr_node.h
	$(CC) $(CFLAGS) term_node.cpp

postfix_expr_node.o: postfix_expr_node.cpp postfix_expr_node.h expr_node.h
	$(CC) $(CFLAGS) postfix_expr_node.cpp

u_expr_node.o: u_expr_node.cpp u_expr_node.h expr_node.h
	$(CC) $(CFLAGS) u_expr_node.cpp

cast_expr_node.o: cast_expr_node.cpp cast_expr_node.h expr_node.h
	$(CC) $(CFLAGS) cast_expr_node.cpp

factor_node.o: factor_node.cpp factor_node.h expr_node.h
	$(CC) $(CFLAGS) factor_node.cpp

add_expr_node.o: add_expr_node.cpp add_expr_node.h expr_node.h
	$(CC) $(CFLAGS) add_expr_node.cpp

s_expr_node.o: s_expr_node.cpp s_expr_node.h expr_node.h
	$(CC) $(CFLAGS) s_expr_node.cpp

ml_expr_node.o: ml_expr_node.cpp ml_expr_node.h expr_node.h
	$(CC) $(CFLAGS) ml_expr_node.cpp

eq_expr_node.o: eq_expr_node.cpp eq_expr_node.h expr_node.h
	$(CC) $(CFLAGS) eq_expr_node.cpp

and_expr_node.o: and_expr_node.cpp and_expr_node.h expr_node.h
	$(CC) $(CFLAGS) and_expr_node.cpp

xor_expr_node.o: xor_expr_node.cpp xor_expr_node.h expr_node.h
	$(CC) $(CFLAGS) xor_expr_node.cpp

or_expr_node.o: or_expr_node.cpp or_expr_node.h expr_node.h
	$(CC) $(CFLAGS) or_expr_node.cpp

log_and_expr_node.o: log_and_expr_node.cpp log_and_expr_node.h expr_node.h
	$(CC) $(CFLAGS) log_and_expr_node.cpp

log_expr_node.o: log_expr_node.cpp log_expr_node.h expr_node.h
	$(CC) $(CFLAGS) log_expr_node.cpp

a_expr_node.o: a_expr_node.cpp a_expr_node.h expr_node.h
	$(CC) $(CFLAGS) a_expr_node.cpp

expr_node.o: expr_node.cpp expr_node.h nonterminal_node.h
	$(CC) $(CFLAGS) expr_node.cpp

symbol_table.o: symbol_table.cpp symbol_table.h
	$(CC) $(CFLAGS) symbol_table.cpp

symbol_entry.o: symbol_entry.cpp symbol_entry.h
	$(CC) $(CFLAGS) symbol_entry.cpp

syntax_tree.o: syntax_tree.cpp syntax_tree.h node.h
	$(CC) $(CFLAGS) syntax_tree.cpp

quadruple.o: quadruple.cpp quadruple.h
	$(CC) $(CFLAGS) quadruple.cpp

func_decl_node.o: func_decl_node.cpp func_decl_node.h
	$(CC) $(CFLAGS) func_decl_node.cpp

a_expressions_node.o: a_expressions_node.cpp a_expressions_node.h
	$(CC) $(CFLAGS) a_expressions_node.cpp

param_list_node.o: param_list_node.cpp param_list_node.h
	$(CC) $(CFLAGS) param_list_node.cpp

param_decl_node.o: param_decl_node.cpp param_decl_node.h
	$(CC) $(CFLAGS) param_decl_node.cpp

jmp_stmt_node.o: jmp_stmt_node.cpp jmp_stmt_node.h
	$(CC) $(CFLAGS) jmp_stmt_node.cpp

io_stmt_node.o: io_stmt_node.cpp io_stmt_node.h
	$(CC) $(CFLAGS) io_stmt_node.cpp

loop_hdr_node.o: loop_hdr_node.cpp loop_hdr_node.h
	$(CC) $(CFLAGS) loop_hdr_node.cpp

unmatched_node.o: unmatched_node.cpp unmatched_node.h
	$(CC) $(CFLAGS) unmatched_node.cpp

matched_node.o: matched_node.cpp matched_node.h
	$(CC) $(CFLAGS) matched_node.cpp

block_node.o: block_node.cpp block_node.h
	$(CC) $(CFLAGS) block_node.cpp

code_generator.o: code_generator.cpp code_generator.h quadruple.h
	$(CC) $(CFLAGS) code_generator.cpp

register.o: register.cpp register.h
	$(CC) $(CFLAGS) register.cpp

clean:
	rm *.o trans

run:
	./trans
