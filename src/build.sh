gcc smc.cpp -g -o ../smc -DEXECSTACK -z execstack
gcc smc.cpp -g -o ../smc-static -DEXECSTACK -DOMAGIC -static -Wl,--omagic -z execstack # to test the elf loader
gcc smc-2.cpp -g -o ../smc-2
gcc smc-mt.cpp -g -o ../smc-mt -lpthread
gcc smc-mt-2.cpp -g -o ../smc-mt-2 -lpthread
gcc smc-shared.cpp -g -o ../smc-shared -lrt
gcc smc-shared-2.cpp -g -o ../smc-shared-2 -lrt
gcc shmid.cpp -g -o ../shmid
gcc segfault.cpp -g -o ../segfault
gcc mremap.cpp -g -o ../mremap
gcc smc-fd.cpp -g -o ../smc-fd
gcc smc-signals.cpp -g -o ../smc-signals -lpthread
gcc signals-nested.cpp -g -o ../signals-nested -lrt
gcc signals-nested-2.cpp -g -o ../signals-nested-2 -lpthread
gcc smc-shmatdt.cpp -g -o ../smc-shmatdt
gcc prot-growsdown.cpp -g -o ../prot-growsdown
gcc memfd.cpp -g -o ../memfd
