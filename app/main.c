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

int main(int argc, const char* argv[]) {
    return test_parse(argc, argv);
}
