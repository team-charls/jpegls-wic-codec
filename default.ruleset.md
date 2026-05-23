<!--
  SPDX-FileCopyrightText: © 2020 Team CharLS
  SPDX-License-Identifier: BSD-3-Clause
-->

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

- C26436: The type X with a virtual function needs either public virtual or protected non-virtual destructor (c.35).
**Rationale**: Using winRT as a module

- C26432: f you define or delete any default operation in the type define or delete them all (c.21).
**Rationale**: Using winRT as a module

- C26466: Don't use static_cast downcasts. A cast from a polymorphic type should use dynamic_cast (type.2).
**Rationale**: Using winRT as a module

- C26493: Don't use C-style casts (type.4).
**Rationale**: Using winRT as a module

- C26473: Don't cast between pointer types where the source type and the target type are the same (type.1).
**Rationale**: Using winRT as a module

- C26496: The variable 'hr' does not change after construction, mark it as const (con.4).
**Rationale**: Using winRT as a module

- C26497: You can attempt to make '' constexpr 
**Rationale**: Using winRT as a module

- C26409: Avoid calling new and delete explicitly, use std::make_unique<T> instead (r.11).
**Rationale**: Using winRT as a module

- C26447: The function is declared 'noexcept' but calls function 'AddRef()' which may throw exceptions (f.6).
**Rationale**: Using winRT as a module

 - C26465: Don't use const_cast to cast away const or volatile. const_cast is not required; constness or volatility is not being removed by this conversion (type.3).
 **Rationale**: Using winRT as a module