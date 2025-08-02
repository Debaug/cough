#pragma once

#include <stdbool.h>

#include "diagnostics/diagnostics.h"
#include "compiler/tokens/token.h"

typedef struct Scanner {
    const char* text;
    TextPos text_pos;
    Reporter* reporter;
} Scanner;

Scanner new_scanner(const char* text, Reporter* reporter);
char peek_scanner(Scanner scanner);
void step_scanner(Scanner* scanner);
void step_scanner_by_same_line(Scanner* scanner, usize steps);
TextView scanner_text_from(Scanner scanner, TextPos start);
TokenArrayBuf scan(Scanner* scanner);
