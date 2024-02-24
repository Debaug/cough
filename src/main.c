#include <stdio.h>
#include <stdbool.h>

#include "text/text.h"
#include "diagnostic/diagnostic.h"
#include "scanner/scanner.h"
#include "ast/parser.h"
#include "ast/program.h"
#include "compiler/compiler.h"
#include "vm/vm.h"

int test_parse(int argc, const char* argv[]) {
    if (argc >= 3) {
        report_error(
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
        report_errno();
        return EXIT_FAILURE;
    }

    report_system_error("a system error, value: %d", 420);
    report_error("a regular error, value: %d", 69);

    printf("==== TOKENS ====\n");

    scanner_t scanner = new_scanner((const char*)source.text.ptr);
    array_buf_t tokens = scan(&scanner);
    for (token_t* token = tokens.ptr; token->type != TOKEN_EOF; token++) {
        printf("%zu:%zu .. %zu:%zu: [%d] '%.*s'\n",
            token->text.start.line + 1,
            token->text.start.column + 1,
            token->text.end.line + 1,
            token->text.end.column + 1,
            token->type,
            (int)token->text.len,
            token->text.ptr
        );
    }

    printf("\n====== AST ======\n");

    parser_t parser = new_parser((const token_t*)tokens.ptr);
    parse_program_result_t program = parse_program(&parser);

    if (!program.is_ok) {
        fprintf(stderr, "error: failed to parse program\n");
    }

    ast_debugger_t debugger = new_ast_debugger();
    debug_program(program.ok, &debugger);

    destroy_array_buf(source.text);

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

    array_buf_t instruction_buf = new_array_buf();
    array_buf_push(&instruction_buf, &instructions, sizeof(instructions));
    bytecode_t bytecode = {
        .rodata = new_array_buf(),
        .instructions = instruction_buf,
    };
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
