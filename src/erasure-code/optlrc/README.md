# Building Ceph with OptLRC(in-progress work)

## Build Instructions

1. **Clone the Repository**
```bash
git clone https://github.com/Azen233/optlrc2024.git
cd ceph
git submodule update --init --recursive --progress
./install-deps.sh
sudo apt install python3-routes
./do_cmake.sh
cd build
ninja
```

## Reference
https://github.com/olekol33/optlrc2018/tree/master/src/erasure-code/optlrc
   