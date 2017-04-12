cd Export
make clean; make; make install
cd ../UseExport
make clean; make; make test; make clean
cd ../Export
make uninstall; make clean
