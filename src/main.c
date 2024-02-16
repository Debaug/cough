#include <stdio.h>
#include <stdbool.h>

#include "text/text.h"
#include "scanner/scanner.h"
#include "ast/parser.h"
#include "ast/program.h"
#include "compiler/compiler.h"
#include "vm/vm.h"

int test_parse(int argc, const char* argv[]) {
    if (argc >= 3) {
        fprintf(stderr, "error: too many arguments\n");
        fprintf(stderr, "USAGE:\n");
        fprintf(stderr, "    cough <FILE>\t-- scan file\n");
        fprintf(stderr, "    cough\t\t-- scan from standard input\n");
        return -1;
    }

    FILE* file;
    bool from_file;
    if (argc == 2) {
        file = fopen(argv[1], "r");
        from_file = true;
    } else {
        file = stdin;
        from_file = false;
    };

    read_file_result_t text = read_file(file);
    if (text.error) {
        fprintf(stderr, "error: failed to read file (error code %d)\n",
            text.error);
        if (from_file) {
            fclose(file);
        }
        return -2;
    }

    printf("==== TOKENS ====\n");

    scanner_t scanner = new_scanner((const char*)text.text.ptr);
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

    destroy_array_buf(text.text);
    if (from_file) {
        fclose(file);
    }
    return 0;
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
