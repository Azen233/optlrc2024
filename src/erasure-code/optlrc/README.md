# Building Ceph with OptLRC(in-progress work)

## Prerequisite
Image: Ubuntu 22.04.2 LTS(Jammy)

Filesystem space: ~70G 
(manual for allocating more space on a node of cloudlab:https://docs.cloudlab.us/cloudlab-manual.html#%28part._storage-example-local-dataset%29)

## Build and Install Instructions
Use "sudo" if permission denied in any of the following steps.

```bash
git clone https://github.com/Azen233/optlrc2024.git
cd optlrc2024
git submodule update --init --recursive --progress
git clean -fdx
git submodule foreach git clean -fdx
./install-deps.sh
sudo apt install python3-routes
sudo apt-get install doxygen
sudo apt-get install graphviz
```
(optional:)

```bash
sudo add-apt-repository ppa:deadsnakes/ppa -y && sudo apt-get update && sudo apt-get install python3.9 python3.9-venv python3.9-dev -y && sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 
```

```bash
./do_cmake.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo
```
Ignore the int-floating conversion test(https://github.com/ceph/ceph/commit/658ecaec8ebade868799f2535bd195853f07f7f9)

```bash
cd build
ninja
sudo ninja install
```
Setting up a test Ceph cluster, refer to the README: https://github.com/ceph/ceph

## Reference
https://github.com/olekol33/optlrc2018/tree/master/src/erasure-code/optlrc
   
