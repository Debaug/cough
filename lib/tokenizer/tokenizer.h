#include "diagnostics/report.h"
#include "source/source.h"
#include "tokens/token.h"

bool tokenize(String source, Reporter* reporter, ArrayBuf(Token)* dst);
