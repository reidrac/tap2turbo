tap2turbo
=========

This is a simple tool to generate a *TZX* tape file from any of the tape formats
supported by `libspectrum`, changing any block into a turbo block setting
arbitrary timing values.

I'm not very familiar with `libspectrum` but the resulting files seem to be OK
when analysed with Tapir, so I think the result is reliable.


Required to build
-----------------

 - gcc
 - libspectrum (and its development files)
 - make

Once the requirements are met, just run `make`.


Example of use
--------------

For example, to change blocks 4 and 5 (first block is 0) of test.tap to
turbo blocks with the timings of *Biturbo 1*, we can run:

    tap2turbo -t 2165:8350:714:714:486:833 -o test.tzx test.tap 4 5

Run the tool with `-h` flag for help.


Author
------

Juan J. Martinez <jjm@usebox.net>

This is free software under the GPL (the same licence of `libspectrum`), and
comes with no warranty of any kind. Use it at your own risk.

