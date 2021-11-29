# name of your application
APPLICATION = iondb_test

# If no BOARD is found in the environment, use this default:
BOARD ?= native


# Blacklisting msp430-based boards, as file syscalls are not supported
FEATURES_BLACKLIST += arch_msp430

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

#### Iondb
USEMODULE += fatfs_vfs
FEATURES_OPTIONAL += periph_rtc

USEPKG += iondb
USEMODULE += iondb-dictionary-bpptree
USEMODULE += iondb-dictionary-linearhash
USEMODULE += iondb-dictionary-openaddressfilehash
USEMODULE += iondb-dictionary-openaddresshash
USEMODULE += iondb-dictionary-skiplist

# For real hardware and spi sd_card usage
ifneq ($(BOARD), native)
	USEMODULE += sdcard_spi
	USEMODULE += fatfs_diskio_mtd

	# for actual hardware use mtd_sdcard as storage device
	USEMODULE += mtd_sdcard

	ifeq ($(BOARD), esp32-wroom-32)
	CFLAGS += -DSDCARD_SPI_PARAM_SPI=SPI_DEV\(1\)
	CFLAGS += -DSDCARD_SPI_PARAM_CS=GPIO_PIN\(2,15\)
	CFLAGS += -DSDCARD_SPI_PARAM_CLK=GPIO_PIN\(2,14\)
	CFLAGS += -DSDCARD_SPI_PARAM_MOSI=GPIO_PIN\(2,13\)
	CFLAGS += -DSDCARD_SPI_PARAM_MISO=GPIO_PIN\(2,12\)
	endif

endif

include $(RIOTBASE)/Makefile.include
