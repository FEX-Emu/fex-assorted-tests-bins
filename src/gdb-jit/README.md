# GDB JIT Interface test

build with
```
gcc fexreader.cpp -shared -o fexreader.so
```

then in gdb type

```
jit-reader-load PATH/TO/fexreader.so
```

Then use https://github.com/FEX-Emu/FEX/tree/skmp/gdb-jit-integration

Requires: x86_64-linux-gnu-objdump on the host


