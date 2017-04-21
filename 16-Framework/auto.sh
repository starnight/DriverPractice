cd Framework
make clean; make; make install

cd ../UseFramework
make clean; make; make install; echo

cat /proc/kallsyms | grep example; echo
ls -l /dev/useexample*

cd ../test-application
make; make test; make clean

cd ../UseFramework
make uninstall; make clean

cd ../Framework
make uninstall; make clean

dmesg | tail -n 50
