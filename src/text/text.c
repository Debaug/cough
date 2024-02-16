#include "text.h"

text_pos_t text_pos_next(text_pos_t pos, const char* text) {
    if (text[0] == '\n') {
        return (text_pos_t){ 
            .line = pos.line + 1,
            .column = 0,
        };
    } else {
        return (text_pos_t){
            .line = pos.line,
            .column = pos.column + 1,
        };
    }
}
