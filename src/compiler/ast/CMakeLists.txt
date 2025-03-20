target_sources(cough PRIVATE
    parser.h parser.c
    analyzer.h analyzer.c
    debug.h debug.c

    storage.h storage.c
    ast.h ast.c
    type.h type.c
    program.h program.c
    function.h function.c
    expression.h expression_parse.c expression_analyze.c expression_debug.c
)
