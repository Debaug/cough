# Cough

A toy language

## Values in Cough

Values in Cough are either *mutable* or *immutable*. Immutable values might be shared by multiple variables, but mutable values can only be owned by one.

Arguments are always passed by value. In particular, shared values are "shallow-copied". This is not an issue, because the garbage collector is responsible for cleaning up nontrivial values.
