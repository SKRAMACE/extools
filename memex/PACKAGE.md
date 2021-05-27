 CREATE A DEB PACKAGE
    mkdir -p /tmp/installdir
    make clean
    make
    DESTDIR=/tmp/installdir NOLDCONFIG=y make install
    fpm -s dir -t deb -n lmemex -v 1.0.0 --after-install=package/ldconfig.sh --after-remove=package/ldconfig.sh -C /tmp/installdir usr
