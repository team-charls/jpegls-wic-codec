# Comments on disabled Microsoft C++ Code Analysis Rules

This document contains the rationales why Microsoft
C++ warnings are disabled in the file default.ruleset
It is not possible to add this info to the .ruleset file itself as edit actions
with the VS GUI would cause the comments to get lost.
Most of these rules\warning are based on the C++ Core Guidelines.

## Warnings

- C26429: Use a not_null to indicate that "null" is not a valid value  
**Rationale**: Prefast attributes (\_In_, etc.) are better than gsl::not_null.

- C26446: Prefer to use gsl::at() instead of unchecked subscript operator.  
**Rationale**: gsl:at() cannot be used as gsl project is by design not included. MSVC STL in debug mode already checks access.

- C26459: You called an STL function 'std::transform' with a raw pointer parameter. Consider wrapping your range in a
gsl::span and pass as a span iterator (stl.1)  
**Rationale**: gsl\std:span() cannot be used.

- C26472: Don't use static_cast for arithmetic conversions  
**Rationale**: can only be solved with gsl::narrow_cast

- C26474: No implicit cast  
**Rationale**: false warnings for GetProcAddress

- C26481: Do not pass an array as a single pointer.  
**Rationale**: Many false warnings at locations when std::span cannot be used.

- C26482:Only index into arrays using constant expressions.  
**Rationale**: static analysis can verify access.

- C26485: Do not pass an array as a single pointer  
**Rationale**: false warnings when passing wchar_t arrays.

- C26490: Don't use reinterpret_cast  
**Rationale**: required to work with win32 API, manual verification required.

- C26494: Variable 'x' is uninitialized. Always initialize an object  
**Rationale**: many false warnings due to output parameters. Other analyzers are better
as they check if the variable is used before initialized.

- C26821:  Consider using gsl::span instead of std::span to guarantee runtime bounds safety (gsl.view).  
**Rationale**: see C26481.
