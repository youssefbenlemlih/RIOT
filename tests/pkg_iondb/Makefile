include ../Makefile.tests_common

BOARD ?= native

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

else # For native usage
	FATFS_IMAGE_FILE_SIZE_MIB ?= 128

	USEMODULE += mtd_native

	#overwrite default mtd_native-config to use fat image as flash device
	MTD_NATIVE_FILENAME    ?= \"./bin/riot_fatfs_disk.img\"
	MTD_PAGE_SIZE   ?= 512
	MTD_SECTOR_SIZE ?= 512
	MTD_SECTOR_NUM  ?= \(\(\(FATFS_IMAGE_FILE_SIZE_MIB\)*1024*1024\)/MTD_SECTOR_SIZE\)
	CFLAGS += -DMTD_NATIVE_FILENAME=$(MTD_NATIVE_FILENAME)
	CFLAGS += -DMTD_PAGE_SIZE=$(MTD_PAGE_SIZE)
	CFLAGS += -DMTD_SECTOR_SIZE=$(MTD_SECTOR_SIZE)
	CFLAGS += -DFATFS_IMAGE_FILE_SIZE_MIB=$(FATFS_IMAGE_FILE_SIZE_MIB)
	CFLAGS += -DMTD_SECTOR_NUM=$(MTD_SECTOR_NUM)
endif

include $(RIOTBASE)/Makefile.include

image:
	@tar -xjf riot_fatfs_disk.tar.gz -C ./bin/

#this generates a compressed fat image file that can be used by the fat driver on native
compressed-image:
	@./create_fat_image_file.sh $(FATFS_IMAGE_FILE_SIZE_MIB)