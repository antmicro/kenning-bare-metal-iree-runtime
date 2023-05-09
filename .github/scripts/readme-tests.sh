#!/bin/bash

set -e -o pipefail

bash <(echo -e 'set -e\n' && tuttest --language bash README.md | grep -v '^\$')
