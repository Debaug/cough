#pragma once

#include "diagnostics/report.h"
#include "source/source.h"

typedef struct CrashingReporter {
    Reporter base;
    SourceText source;
    Severity severity;
} CrashingReporter;

CrashingReporter crashing_reporter_new(SourceText source);
