#include "diagnostics/report.h"
#include "source/source.h"
#include "tokens/token.h"

// *dst will be replaced without being freed
// *dst may be uninitialized
bool tokenize(String source, Reporter* reporter, TokenStream* dst);
