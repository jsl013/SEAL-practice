# SEAL-practice

## How to Use
- There are five practices for using SEAL library.
- In each practice directory (`basic`, `encoder`, `levels`, `ntt`, `performance`), there is `build` directory.
- At first time after you clone the repo, you have to delete the build directory and mkdir build again (`rm -rf *` in `build`)
- In `build`, do `cmake .. -DCMAKE_PREFIX_PATH=~/path-of-your-SEAL-install -DCMAKE_BUILD_TYPE=RelWithDebInfo` (add `CMAKE_BUILD_TYPE` option, if you want to debug)
  - And `make`
