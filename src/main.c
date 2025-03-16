#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "text/text.h"
#include "diagnostic/diagnostic.h"
#include "tokens/scanner.h"
#include "ast/ast.h"
#include "vm/vm.h"

int test_parse(int argc, const char* argv[]) {
    if (argc >= 3) {
        print_error(
            "too many arguments\n\n"
            "\tUSAGE:\n"
            "\t\tcough <FILE>\t parse file\n"
            "\t\tcough\t\t parse from standard input"
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
    default_reporter_t reporter = new_default_reporter(NULL);

    byteword_t instructions[] = {
        [0] =
        OP_SYSCALL, SYS_SAY_HI,
        OP_SYSCALL, SYS_SAY_BYE,
        OP_LOAD_IMM, 0x11, 0x22,
        OP_SYSCALL, SYS_EXIT,
    };

    printf("===== ASSEMBLY =====\n");
    for (size_t i = 0; i * sizeof(byteword_t) < sizeof(instructions); i++) {
        printf("%02x ", instructions[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }
    printf("\n\n");

    section_buf_t instruction_buf = new_array_buf();
    array_buf_extend(
        &instruction_buf,
        &instructions,
        sizeof(instructions) / sizeof(byteword_t),
        byteword_t
    );

    bytecode_t bytecode = {
        .instructions = instruction_buf,
        .rodata = new_array_buf(),
    };
    vm_t vm = new_vm(bytecode, &reporter.reporter);

    printf("== PROGRAM OUTPUT ==\n");
    run_vm(&vm);

    printf(
        "\n=== PROGRAM EXIT ===\nexit code: %" PRId64 "/ 0x%" PRIx64 "\n",
        vm.exit_code, vm.exit_code
    );
    return 0;
}

int main(int argc, const char* argv[]) {
#if 0
    return test_parse(argc, argv);
#else
    return test_run(argc, argv);
#endif
}
