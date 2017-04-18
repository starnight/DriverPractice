cd Framework
make clean; make; make install
cd ../UseFramework
make clean; make; make test; make clean
cd ../Framework
make uninstall; make clean
dmesg | tail -n 40
