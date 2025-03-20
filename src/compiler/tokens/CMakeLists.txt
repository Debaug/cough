set(GEN_SCAN_EXACT_CHARS ${CMAKE_CURRENT_SOURCE_DIR}/gen_scan_exact_chars.py)
set(PUNCT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/punct.json)
add_custom_command(
    OUTPUT scan_exact_chars.c
    COMMAND ${Python_EXECUTABLE} ${GEN_SCAN_EXACT_CHARS} ${PUNCT_FILE} > scan_exact_chars.c
    DEPENDS ${GEN_SCAN_EXACT_CHARS} ${PUNCT_FILE}
    VERBATIM
)
add_custom_target(scan_exact_chars DEPENDS scan_exact_chars.c)

target_sources(cough PRIVATE
    token.h
    scanner.h scanner.c
    ${CMAKE_CURRENT_BINARY_DIR}/scan_exact_chars.c
)
add_dependencies(cough scan_exact_chars)
