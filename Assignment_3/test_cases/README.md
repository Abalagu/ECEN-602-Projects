# README

## Notes on test cases

### 1.enable typedef on `addrinfo` 

* modified file `/usr/include/features.h` , add `#define _XOPEN_SOURCE 700` to the file, which represents using 7th edition.
* after defining the macro, VS Code won't complain about `incomplete type` on addrinfo.

