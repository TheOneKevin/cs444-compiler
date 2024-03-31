#!/bin/bash

doxygen doxygen.in
rm -rf ~/public_html/cs444/
cp -r build/doxygen-docs/html ~/public_html/cs444
chmod -R a+r ~/public_html/cs444
