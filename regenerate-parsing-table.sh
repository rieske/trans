#!/bin/bash
set -euo pipefail
mkdir -p logs
build/trans --grammar=resources/configuration/grammar.bnf
