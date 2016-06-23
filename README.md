

# Installation

First, get the files.

``` 
git clone https://github.com/HerveFrezza-Buet/elec
``` 


The library consists of header files only. So you can put them somewhere your compiler can access them, and it is ok. Nevertheless, a cleaner install can be done as follows with cmake.

``` 
cd <your_path_here>/elec
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
sudo make install
``` 


# Documentation

Read examples in the suggested order. Doxygen pages are accessible from the elec/html/index.html file.



