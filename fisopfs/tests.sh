#!/bin/bash

# Dame tus permisos.
# chmod +x tests.sh

gcc tests.c entry.c fs.c utils.c -g -o tests && ./tests && rm tests
