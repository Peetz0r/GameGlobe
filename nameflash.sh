#!/bin/bash

echo "#define NAAM \"$@\"" > src/naam.h

ano clean
ano build
ano upload
