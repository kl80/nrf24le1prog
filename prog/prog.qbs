import qbs

// export DYLD_LIBRARY_PATH=/usr/local/Cellar/libusb-compat/0.1.5/lib:/usr/local/Cellar/libusb/1.0.20/lib
CppApplication {
    name: "nrf24le1prog"

    consoleApplication: true
    files: ["*.c", "*.h"]

    cpp.cLanguageVersion: "c11"
    cpp.includePaths: ["/usr/local/Cellar/libusb-compat/0.1.5/include"]
    cpp.libraryPaths: ["/usr/local/Cellar/libusb-compat/0.1.5/lib"]
    cpp.staticLibraries: ["usb"]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
