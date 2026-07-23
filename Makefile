# Thin wrappers around CMake so day-to-day work stays short:
#   make            # build
#   make test       # build + run tests
#   make configure  # first-time / reconfigure Debug build

BUILD_DIR  ?= build
BUILD_TYPE ?= Debug
# Parallelism for the inner build (outer `make -j` only parallelizes this Makefile).
JOBS       ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Extra args for ctest, e.g. make test ARGS='-R parser -V'
ARGS       ?=

.PHONY: all configure build test coverage coverage-report clean scrub help

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	ln -sfn $(BUILD_DIR)/compile_commands.json compile_commands.json

# Configure automatically if the build tree is missing.
$(BUILD_DIR)/CMakeCache.txt:
	@$(MAKE) configure

build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) -j$(JOBS)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure -j$(JOBS) $(ARGS)

# lcov only — assumes tests already ran serially (CI: make test JOBS=1).
# Parallel ctest races gcov counters when functionalTest is sharded.
coverage-report: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) --target coverage-report

coverage: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) --target coverage -j$(JOBS)

clean: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) --target clean

scrub: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) --target scrub

help:
	@echo "Targets:"
	@echo "  make [all]            Build (configures $(BUILD_DIR) on first run)"
	@echo "  make test             Build and run tests"
	@echo "  make configure        Configure $(BUILD_DIR) (BUILD_TYPE=$(BUILD_TYPE))"
	@echo "  make coverage         Run tests and produce $(BUILD_DIR)/coverage/lcov.info"
	@echo "  make coverage-report  lcov only (after tests; used by CI)"
	@echo "  make clean            Clean build outputs"
	@echo "  make scrub            Clean build outputs and gcov artifacts"
	@echo ""
	@echo "Variables: BUILD_DIR=$(BUILD_DIR) BUILD_TYPE=$(BUILD_TYPE) JOBS=$(JOBS)"
	@echo "  make test ARGS='-R parser -V'   # filter / verbose ctest"
	@echo "  make test ARGS='-L functional'  # functional shards only"
	@echo "  make test JOBS=1               # serial ctest"
	@echo "  make BUILD_TYPE=Release configure"
	@echo "  cmake -S . -B build -DFUNCTIONAL_TEST_SHARDS=16  # more functional shards"
