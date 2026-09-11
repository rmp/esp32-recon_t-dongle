# ==============================================================================
#  T-Dongle-S3 Diagnostic & Pentest Tool  -  developer Makefile
#
#  Thin, self-documenting wrapper around PlatformIO. Run `make` (or `make help`)
#  to list targets.
#
#  Common flows:
#    make build            compile the firmware
#    make test             static analysis + unit tests (if any)
#    make deploy           build + flash to the connected dongle
#    make deploy-monitor   flash, then open the serial monitor
#
#  Override the environment or port on the command line, e.g.:
#    make deploy PORT=/dev/cu.usbmodem31401
#    make build ENV=tdongle-s3
# ==============================================================================

PIO   ?= pio
ENV   ?= tdongle-s3
BAUD  ?= 115200

# Auto-detect the dongle's serial port on macOS/Linux; override with PORT=...
PORT  ?= $(shell ls /dev/cu.usbmodem* /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -n1)
ifneq ($(strip $(PORT)),)
  UPLOAD_ARGS := --upload-port $(PORT)
  MON_ARGS    := --port $(PORT)
endif

.DEFAULT_GOAL := help

# ---- Help ---------------------------------------------------------------------
.PHONY: help
help: ## Show this help
	@echo "T-Dongle-S3 Diagnostic Tool - make targets:"
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
	  | sort \
	  | awk 'BEGIN{FS=":.*?## "}{printf "  \033[36m%-16s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "Variables: ENV=$(ENV)  BAUD=$(BAUD)  PORT=$(if $(PORT),$(PORT),<auto>)"

# ---- Build --------------------------------------------------------------------
.PHONY: build
build: ## Compile the firmware
	$(PIO) run -e $(ENV)

.PHONY: rebuild
rebuild: clean build ## Clean then compile

# ---- Test / quality -----------------------------------------------------------
.PHONY: check
check: ## Static analysis (cppcheck via PlatformIO)
	$(PIO) check -e $(ENV) --skip-packages --fail-on-defect high \
	  --src-filters="+<src/>" \
	  --flags="--suppress=cppcheckError"

.PHONY: test
test: check ## Run static analysis and unit tests (tests optional, under test/)
	@echo ">> Running unit tests (skipped if none under test/) ..."
	-$(PIO) test -e $(ENV) $(UPLOAD_ARGS)

# ---- Deploy / flash -----------------------------------------------------------
.PHONY: deploy upload flash
deploy: ## Build and flash to the connected dongle
	$(PIO) run -e $(ENV) -t upload $(UPLOAD_ARGS)
upload: deploy   ## Alias for deploy
flash:  deploy   ## Alias for deploy

.PHONY: deploy-monitor
deploy-monitor: deploy monitor ## Flash, then open the serial monitor

# ---- Serial / device ----------------------------------------------------------
.PHONY: monitor
monitor: ## Open the USB-CDC serial monitor (Ctrl-C to exit)
	$(PIO) device monitor -b $(BAUD) $(MON_ARGS)

.PHONY: devices
devices: ## List connected serial devices
	$(PIO) device list

.PHONY: erase
erase: ## Erase the entire flash (device must be connected)
	$(PIO) run -e $(ENV) -t erase $(UPLOAD_ARGS)

# ---- Housekeeping -------------------------------------------------------------
.PHONY: clean
clean: ## Remove build artifacts for this environment
	$(PIO) run -e $(ENV) -t clean

.PHONY: distclean
distclean: ## Remove the entire .pio build directory
	rm -rf .pio
