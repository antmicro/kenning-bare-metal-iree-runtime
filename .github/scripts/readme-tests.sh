#!/bin/bash

set -e -o pipefail

export DEBIAN_FRONTEND=noninteractive

bash -x <(echo -e 'set -e\n' && tuttest --language bash README.md | grep -v '^\$')
