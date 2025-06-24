# olcGimmeHead
A simple guided text parser to combine multifile C++ projects into a single header include

# Example
`example.h`
```cpp
#pragma once

//! START DECLARATION
class MyClass 
{
public:
	MyClass();
}
//! END DECLARATION

```

`example.cpp`
```cpp
#include "example.h"

//! START IMPLEMENTATION
MyClass::MyClass()
{
	// Construction
}
//! END IMPLEMENTATION

```

`template.h`
```cpp
#pragma once

//!GRAB example.h DECLARATION

#if defined(APP_IMPL)
//!GRAB example.cpp IMPLEMENTATION
#undef APP_IMPL
#endif
```

usage:
```
>gimme-head template.h output.h
```

yields:
`output.h`
```cpp
#pragma once

class MyClass 
{
public:
	MyClass();
}

#if defined(APP_IMPL)
MyClass::MyClass()
{
	// Construction
}
#undef APP_IMPL
#endif
```


