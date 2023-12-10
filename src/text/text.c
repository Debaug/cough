#include "text.h"

text_pos_t text_pos_next(text_pos_t pos, const char* text) {
    if (text[pos.index] == '\n') {
        return (text_pos_t){ 
            .line = pos.line + 1,
            .column = 0,
            .index = pos.index + 1
        };
    } else {
        return (text_pos_t){
            .line = pos.line,
            .column = pos.column + 1,
            .index = pos.index + 1
        };
    }
}
