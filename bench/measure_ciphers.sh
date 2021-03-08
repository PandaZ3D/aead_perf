#!/bin/bash

# info: measure both AES and ChaCha20 ciphers
# run from within bench directory

# setup python virtual environment
# mkvirtualenv --python=$(which python3) pybench && \
    # python -m pip install -r requirements.txt

# use python environment
# workon pybench
source pybench/bin/activate

# one time set-up for generating data
python bench_setup.py

deactivate

# run bench for AES

# run bench for ChaCha

# analyze results for statistical significance

# measure memory usage

# save all measurements

# plot graphs
