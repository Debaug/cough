#include <stdio.h>
#include <stdbool.h>

#include "text/text.h"
#include "diagnostic/diagnostic.h"
#include "tokens/scanner.h"
#include "ast/ast.h"
#include "compiler/compiler.h"
#include "vm/vm.h"

int test_parse(int argc, const char* argv[]) {
    if (argc >= 3) {
        print_error(
            "too many arguments\n\n"
            "\tUSAGE:\n"
            "\t\tcough <FILE>\t scan file\n"
            "\t\tcough\t\t scan from standard input"
        );
        return -1;
    }

    const char* path = (argc == 2) ? argv[1] : NULL;
    source_t source;
    if (load_source_file(path, &source) != SUCCESS) {
        print_errno();
        return EXIT_FAILURE;
    }

    print_system_error("a system error, value: %d\n", 420);
    print_error("a regular error, value: %d\n", 69);

    default_reporter_t reporter = new_default_reporter(&source);

    printf("==== TOKENS ====\n");

    scanner_t scanner = new_scanner(source.text.data, (reporter_t*)&reporter);
    token_array_buf_t tokens = scan(&scanner);
    for (size_t i = 0; tokens.data[i].type != TOKEN_EOF; i++) {
        token_t token = tokens.data[i];
        printf("%zu. \t%zu:%zu .. %zu:%zu: [%d] '%.*s'\n",
            i,
            token.text.start.line + 1,
            token.text.start.column + 1,
            token.text.end.line + 1,
            token.text.end.column + 1,
            token.type,
            (int)token.text.len,
            token.text.data
        );
    }

    printf("\n====== AST ======\n");

    parser_t parser = new_parser(tokens.data, &reporter.reporter);
    ast_t ast;
    if (parse(&parser, &ast) != SUCCESS) {
        eprintf("failed to parse program");
        return EXIT_FAILURE;
    }

    // analyzer_t analyzer = new_analyzer();
    // if (analyze_unordered_symbols(&analyzer, &ast.program) != ANALYZE_SUCCESS) {
    //     report_error("failed to analyze unordered symbols");
    //     return EXIT_FAILURE;
    // }
    // if (analyze_expressions(&analyzer, &ast.program) != ANALYZE_SUCCESS) {
    //     report_error("failed to analyze expressions");
    //     return EXIT_FAILURE;
    // }

    ast_debugger_t debugger = new_ast_debugger();
    debug_ast(ast, &debugger);

    free_ast(ast);
    free_array_buf(source.text);

    return EXIT_SUCCESS;
}

int test_run(int argc, const char* argv[]) {
    uint32_t instructions[] = {
        [0] =
        OP_CALL, 10,
        OP_SYSCALL, SYS_EXIT, 0,

        [10] =
        OP_ENTER, 2,
        OP_SYSCALL, SYS_SAY_BYE,
        OP_CALL, 30,
        OP_SCALAR, 20, 0,
        OP_ADD_INT,
        OP_RETURN,

        [30] =
        OP_ENTER, 5,
        OP_SYSCALL, SYS_SAY_HI,
        OP_SCALAR, 1, 0,
        OP_SCALAR, 2, 0,
        OP_ADD_INT,
        OP_RETURN,
    };

    printf("===== ASSEMBLY =====\n");
    for (size_t i = 0; i * sizeof(uint32_t) < sizeof(instructions); i++) {
        printf("%02x ", instructions[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }
    printf("\n\n");

    instruction_buf_t instruction_buf = new_array_buf(uint32_t);
    array_buf_extend(
        &instruction_buf,
        &instructions,
        sizeof(instructions) / sizeof(uint32_t),
        uint32_t
    );

    bytecode_t bytecode = { .instructions = instruction_buf };
    vm_t vm = new_vm(bytecode);

    printf("== PROGRAM OUTPUT ==\n");
    run_vm(vm);

    printf("variable stack index: %zu\n", vm.variable_frame_index);
    return 0;
}

int main(int argc, const char* argv[]) {
#if 1
    return test_parse(argc, argv);
#else
    return test_run(argc, argv);
#endif
}
