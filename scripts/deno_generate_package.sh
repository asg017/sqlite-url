#!/bin/bash

set -euo pipefail

export PACKAGE_NAME="sqlite-url"
export EXTENSION_NAME="url0"
export VERSION=$(cat VERSION)

envsubst < deno/deno.json.tmpl > deno/deno.json
echo "✅ generated deno/deno.json"

envsubst < deno/README.md.tmpl > deno/README.md
echo "✅ generated deno/README.md"
