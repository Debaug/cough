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
        return indent + f"type = {"TOKEN_" + char_tree['']};\n"
    
    string = indent + "switch (peek_scanner(*scanner)) {\n"

    for ch, subtree in char_tree.items():
        if ch == "":
            continue
        string += indent + f"case '{ch}':\n"
        string += indent + "    step_scanner(scanner);\n"
        string += character_switch(subtree, indent + "    ")
        string += indent + "    break;\n"

    if "" in char_tree:
        string += indent + f"default: type = {"TOKEN_" + char_tree['']}; break;\n"
    else:
        string += indent + f"default: error = true; break;"

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

#include "scanner/scanner.h"

scan_result_t scan_punctuation(scanner_t* scanner, token_t* dst) {
    bool error = false;
    token_type_t type;
    text_pos_t start = scanner->text_pos;
    const char* text = scanner->text;
'''

    string += character_switch(char_tree, indent = "    ")

    string += """
    if (error) {
        return SCAN_UNEXPECTED_CHARACTER;
    } else {
        text_view_t view = {
            .ptr = text,
            .len = scanner->text - text,
            .start = start,
            .end = scanner->text_pos,
        };
        *dst = (token_t){ .type = type, .text = view };
        return SCAN_SUCCESS;
    }
}"""

    print(string, file = out)

if __name__ == "__main__":
    main()
