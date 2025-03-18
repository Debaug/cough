import sys, json

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
        return indent + f"kind = {"TOKEN_" + char_tree['']};\n"
    
    string = indent + "switch (peek_scanner(*scanner)) {\n"

    for ch, subtree in char_tree.items():
        if ch == "":
            continue
        string += indent + f"case '{ch}':\n"
        string += indent + "    step_scanner(scanner);\n"
        string += character_switch(subtree, indent + "    ")
        string += indent + "    break;\n"

    if "" in char_tree:
        string += indent + f"default: kind = {"TOKEN_" + char_tree['']}; break;\n"
    else:
        string += indent + f"default: error = true; error_char = peek_scanner(*scanner); break;\n"

    string += indent + "}\n"
    return string

def main():
    punct = json.load(open(sys.argv[1]))

    out = sys.stdout
    if len(sys.argv) > 2:
        out = open(sys.argv[2], "w")

    char_tree = character_tree(punct)

    string = '''\
#include <stdio.h>
#include <stdbool.h>

#include "tokens/scanner.h"
#include "alloc/array.h"

Result scan_punctuation(Scanner* scanner, Token* dst) {
    bool error = false;
    char error_char;
    TokenKind kind;
    TextPos start = scanner->text_pos;
'''

    string += character_switch(char_tree, indent = "    ")

    string += """
    if (error) {
        Error error = {
            .kind = ERROR_UNEXPECTED_CHARACTER,
            .source = scanner_text_from(*scanner, start),
            .message = format("unexpected character `%c` in punctuation token", error_char)
        };
        report(scanner->reporter, error);
        return ERROR;
    } else {
        *dst = (Token){ .kind = kind, .text = scanner_text_from(*scanner, start) };
        return SUCCESS;
    }
}"""

    print(string, file = out)

if __name__ == "__main__":
    main()
