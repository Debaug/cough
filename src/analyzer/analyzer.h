#include "ast/program.h"

typedef enum type_storage {
    TYPE_STORAGE_INLINE,
    TYPE_STORAGE_ALLOCATED,
} type_storage_t;

typedef struct type_info {
    size_t size_in_words; // word = 8 bytes
    type_storage_t storage;
} type_info_t;

typedef struct variable {
    identifier_t identifier;
    type_t type;
} variable_t;

typedef struct scope {
    array_buf_t /* named_type_t */ declared_types;
    array_buf_t /* variable_t */ variables;
} scope_t;

typedef struct analyzer {
    array_buf_t /* type_info */ types;
    array_buf_t /* scope_t */ scopes;
} analyzer_t;

analyzer_t new_analyzer(void);
void analyze(program_t program, analyzer_t* analyzer);
void destroy_analyzer(analyzer_t analyzer);
