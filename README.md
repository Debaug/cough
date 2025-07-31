# Cough

A toy functional programming language.

## Values

All values are immutable. 'Plain old data' values (e.g. integers, floats) live on the stack whereas *resources* (values that need cleanup, such as dynamic arrays and files) live on the heap and are garbage-collected. As such, all values can be shallow-copied, and this is what happens when they are passed to functions.

## Types

There are five primitive types in Cough:

- Unit
- Never
- Bool
- Int
- Float

There are two kinds of composite types:

- Structs, i.e. product types
- Variants, i.e. sum types

Both consist of a set of statically typed fields. A struct value consists of all of its fields whereas a variant value is exactly one at a time.

Furthermore, functions are first-class citizens. As such, every function signature corresponds to a unique function type.

### Composite Type Example

```cough
Gender := variant {
    male: Unit,
    female: Unit,
}

Person := struct {
    name: String,
    gender: Gender,
}

tom_gender: Gender = Gender { male := Unit {} };
tom: Person = Person { name := "Tom", gender := tom_gender };
```

## Functions

Functions in Cough are pure, i.e. they are functions in the mathematical sense: they do not mutate anything nor produce any side effect. For example:

```cough
mul_add := fn(a: Int, b: Int, c: Int): Int {
    a * b + c
}

d: Int = mul_add(2, 3, 4); // 10
```

### Function Binding

Function binding syntax allows to ergonomically transform a function into a new one that fixes an argument. For example:

```cough
double_add := fn mul_add($, 2, $); // function binding

e: Int = double_add(3, 5); // mul_add(3, 2, 5) i.e. 11
```

### Piping

The `|>` pipe operator also allows to succinctly chain function calls. It works by passing its expression to the left as the first argument of the function call to its right. For example:

```cough
f: Int = mul_add(2, 3, 4) |> double_add(5); // desugars to double_add(mul_add(2, 3, 4), 5)
```

## Traits

Traits encapsulate common behavior. They can play the role of interfaces in other languages. They may be abstract and have type parameters. For example:

```cough
Eat := trait(A, F) {
    eat: fn(animal: A, food: F): A;
}

Monkey := // some type
Banana := // some type

impl Eat(Monkey, Banana) {
    eat := fn(monkey: Monkey, banana: Banana): Monkey {
        // body
    }
} 
```

The `for`/`with` syntax allows one to require that a given trait be implemented in generics:

```cough
// very temporary syntax
for(A, F) with Eat(A, F) {
    feed_all := fn(animals: [A], food: F): [A] {
        // body
    }
}
```

## For Loops & Iterators

Here is the built-in `Iterator` trait:

```cough
Iterator := trait()
```

## System requirements

- The C `float` type must be the IEEE-754 `binary32` type.
- The C `double` type must be the IEEE-754 `binary64` type.
