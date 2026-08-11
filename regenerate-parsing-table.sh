#!/bin/bash
set -euo pipefail
mkdir -p logs
build/trans -gresources/configuration/grammar.bnf
