# SEAL-practice

## How to Use
- There are five practices for using SEAL library.
- In each practice directory (`basic`, `encoder`, `levels`, `ntt`, `performance`), there is `build` directory.
- In `build`, do `cmake .. -DCMAKE_PREFIX_PATH=~/path-of-your-SEAL-install -DCMAKE_BUILD_TYPE=RelWithDebInfo` (add `CMAKE_BUILD_TYPE` option, if you want to debug)
  - And `make`
