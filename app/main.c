#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "text/text.h"
#include "compiler/diagnostics.h"
#include "compiler/tokens/scanner.h"
#include "compiler/ast/ast.h"
#include "vm/vm.h"
#include "vm/diagnostics.h"

int test_parse(int argc, const char* argv[]) {
    if (argc >= 3) {
        print_error(
            "too many arguments\n\n"
            "\tUSAGE:\n"
            "\t\tcough <FILE>\t parse file\n"
            "\t\tcough\t\t parse from standard input"
        );
        return EXIT_FAILURE;
    }

    const char* path = (argc == 2) ? argv[1] : NULL;
    Source source;
    if (load_source_file(path, &source) != SUCCESS) {
        print_errno();
        return EXIT_FAILURE;
    }

    print_system_error("a system error, value: %d\n", 420);
    print_error("a regular error, value: %d\n", 69);

    CompilerReporter reporter = new_compiler_reporter(&source);

    printf("==== TOKENS ====\n");

    Scanner scanner = new_scanner(source.text.data, (Reporter*)&reporter);
    TokenArrayBuf tokens = scan(&scanner);
    for (usize i = 0; tokens.data[i].kind != TOKEN_EOF; i++) {
        Token token = tokens.data[i];
        printf("%zu. \t%zu:%zu .. %zu:%zu: [%d] '%.*s'\n",
            i,
            token.text.start.line + 1,
            token.text.start.column + 1,
            token.text.end.line + 1,
            token.text.end.column + 1,
            token.kind,
            (int)token.text.len,
            token.text.data
        );
    }

    printf("\n====== AST ======\n");

    Parser parser = new_parser(tokens.data, (Reporter*)&reporter);
    Ast ast;
    if (parse(&parser, &ast) != SUCCESS) {
        eprintf("failed to parse program");
        return EXIT_FAILURE;
    }

    // Analyzer analyzer = new_analyzer();
    // if (analyze_unordered_symbols(&analyzer, &ast.program) != ANALYZE_SUCCESS) {
    //     report_error("failed to analyze unordered symbols");
    //     return EXIT_FAILURE;
    // }
    // if (analyze_expressions(&analyzer, &ast.program) != ANALYZE_SUCCESS) {
    //     report_error("failed to analyze expressions");
    //     return EXIT_FAILURE;
    // }

    AstDebugger debugger = new_ast_debugger();
    debug_ast(ast, &debugger);

    free_ast(ast);
    free_array_buf(source.text);

    return EXIT_SUCCESS;
}

int test_run(int argc, const char* argv[]) {
    DefaultVmSystem vm_system = new_default_vm_system();

    RuntimeReporter reporter = new_runtime_reporter();

    Byteword instructions[] = {
        [0] =
            OP_FRM, 0,
            OP_CAS, [4] = 64, 0, 0, [8] =
            OP_SYS, SYS_EXIT, 0,
        
        [64] =
            OP_RES, 1,
            OP_SCA, 0, [68] = 21, 0, 0, 0, [72] =
            OP_SYS, SYS_HI,
            OP_FRM, 1,
            OP_ARG, 0,
            OP_CAS, [80] = 128, 0, 0, 0, [84] =
            OP_SYS, SYS_DBG, 1,
            OP_SCA, 0, [92] = 0xdead, 0, 0, 0, [96] =
            OP_RET, 0, 1,

        [128] =
            OP_RES, 1,
            OP_MOV, 1, 0,
            OP_ADU, 0, 0, 1,
            OP_RET, 0, 1,
    };

    printf("===== ASSEMBLY =====\n");
    for (usize i = 0; i * sizeof(Byteword) < sizeof(instructions); i++) {
        printf("%04x ", instructions[i]);
        if (i % 8 == 7) {
            printf("\n");
        }
    }
    printf("\n\n");

    SectionBuf instruction_buf = new_array_buf();
    array_buf_extend(
        &instruction_buf,
        &instructions,
        sizeof(instructions) / sizeof(Byteword)
    );

    Bytecode bytecode = {
        .instructions = instruction_buf,
        .rodata = new_array_buf(),
    };
    Vm vm = new_vm((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);

    printf("== PROGRAM OUTPUT ==\n");
    run_vm(&vm);
    return 0;
}

int main(int argc, const char* argv[]) {
#if 0
    return test_parse(argc, argv);
#else
    return test_run(argc, argv);
#endif
}
