#!/bin/bash
# Regenerate resources/configuration/parsing_table from the product grammar.
# -g alone is not enough: the driver still needs a source file to run the pipeline.
set -euo pipefail
mkdir -p logs
tmp=$(mktemp --suffix=.c)
trap 'rm -f "$tmp"' EXIT
echo 'int main(void){return 0;}' >"$tmp"
build/trans -gresources/configuration/grammar.bnf "$tmp" --no-preprocess
cp logs/parsing_table resources/configuration/parsing_table
echo "Updated resources/configuration/parsing_table"
