#pragma once

#include <stdbool.h>

#include "tokens/token.h"
#include "diagnostic/diagnostic.h"

typedef struct Scanner {
    const char* text;
    TextPos text_pos;
    Reporter* reporter;
} Scanner;

Scanner new_scanner(const char* text, Reporter* reporter);
char peek_scanner(Scanner scanner);
void step_scanner(Scanner* scanner);
TextView scanner_text_from(Scanner scanner, TextPos start);
Result scan_punctuation(Scanner* scanner, Token* dst);
TokenArrayBuf scan(Scanner* scanner);
