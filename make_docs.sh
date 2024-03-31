#!/bin/bash

rm -rf build/html docs/_static/doxygen-html
doxygen doxygen.in
source .venv/bin/activate
sphinx-build -b html docs build/html
deactivate
rm -rf ~/public_html/cs444/
cp -r build/html ~/public_html/cs444
chmod -R a+r ~/public_html/cs444
