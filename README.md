# Cough

A toy language

## Values in Cough

Values in Cough are either *mutable* or *immutable*. Immutable values might be shared by multiple variables, but mutable values can only be owned by one.

Arguments are always passed by value. In particular, shared values are "shallow-copied". This is not an issue, because the garbage collector is responsible for cleaning up nontrivial values.

## System requirements

- The C `float` type must be the IEEE-754 `binary32` type.
- The C `double` type must be the IEEE-754 `binary64` type.
