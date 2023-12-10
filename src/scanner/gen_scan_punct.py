import sys

def punct():
    return {
        "(": "TOKEN_LEFT_PAREN",
        ")": "TOKEN_RIGHT_PAREN",
        "[": "TOKEN_LEFT_BRACKET",
        "]": "TOKEN_RIGHT_BRACKET",
        "{": "TOKEN_LEFT_BRACE",
        "}": "TOKEN_RIGHT_BRACE",

        "::": "TOKEN_COLON_COLON",
        ":=": "TOKEN_COLON_EQUAL",
        "=": "TOKEN_EQUAL",

        ":": "TOKEN_COLON",
        ";": "TOKEN_SEMICOLON",
        ",": "TOKEN_COMMA",
        "->": "TOKEN_ARROW",

        "+": "TOKEN_PLUS",
        "-": "TOKEN_MINUS",
        "*": "TOKEN_STAR",
        "/": "TOKEN_SLASH",
        "%": "TOKEN_PERCENT",

        "==": "TOKEN_EQUAL_EQUAL",
        "!=": "TOKEN_BANG_EQUAL",
        "<": "TOKEN_LESS",
        "<=": "TOKEN_LESS_EQUAL",
        ">": "TOKEN_GREATER",
        ">=": "TOKEN_GREATER_EQUAL",

        "!": "TOKEN_BANG",
    }

def character_tree(punct):
    character_tree = {}
    for pat, token in punct.items():
        node = character_tree
        for ch in pat:
            if ch not in node:
                node[ch] = {}
            node = node[ch]
        node[""] = token
    return character_tree

def character_switch(char_tree, indent = ""):
    if  list(char_tree.keys()) == [""]:
        return indent + f"type = {char_tree['']};\n"
    
    string = indent + "switch (peek_scanner(*scanner)) {\n"

    for ch, subtree in char_tree.items():
        if ch == "":
            continue
        string += indent + f"case '{ch}':\n"
        string += indent + "    step_scanner(scanner);\n"
        string += character_switch(subtree, indent + "    ")
        string += indent + "    break;\n"

    if "" in char_tree:
        string += indent + f"default: type = {char_tree['']}; break;\n"
    else:
        string += indent + f"default: error = true; break;"

    string += indent + "}\n"
    return string

def main():
    out = sys.stdout
    if len(sys.argv) > 1:
        out = open(sys.argv[1], "w")

    char_tree = character_tree(punct())

    string = '''#include <stdio.h>
#include <stdbool.h>

#include "scanner/scanner.h"

scan_result_t scan_punctuation(scanner_t* scanner) {
    bool error = false;
    token_type_t type;
    text_pos_t start = scanner->current_pos;
'''

    string += character_switch(char_tree, indent = "    ")

    string += """
    if (error) {
        scan_error_t error = {
            .kind = SCAN_UNEXPECTED_CHARACTER,
            .pos = scanner->current_pos,
        };
        return (scan_result_t){ .success = false, .error = error };
    } else {
        text_span_t span = { .start = start, .end = scanner->current_pos };
        token_t token = { .type = type, .span = span };
        return (scan_result_t){ .success = true, .token = token };
    }
}"""

    print(string, file = out)

if __name__ == "__main__":
    main()
