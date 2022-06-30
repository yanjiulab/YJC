# ev

## X86
```
./configure
make 
make install
```

## ARM
```
./configure --prefix=${PWD}/lib/ev_install --host=arm-none-linux CC=arm-none-linux-gnueabi-gcc
make 
make install
```