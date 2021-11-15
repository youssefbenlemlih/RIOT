PKG_NAME=iondb
PKG_URL=https://github.com/iondbproject/iondb
PKG_VERSION=7e4a291fbfa9643365bf754d50946c5554288f5f
PKG_LICENSE=BSD-3-Clause License

include $(RIOTBASE)/pkg/pkg.mk

IONDB_MODULES         = iondb-dictionary-bpptree \
						iondb-dictionary-linearhash \
						iondb-dictionary-openaddressfilehash \
						iondb-dictionary-openaddresshash \
						iondb-dictionary-skiplist

IONDB_USEMODULE       = $(filter $(IONDB_MODULES),$(USEMODULE))



.PHONY: $(IONDB_MODULES)

all: iondb

iondb: $(IONDB_USEMODULE)
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src -f $(CURDIR)/Makefile.iondb
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/file -f $(CURDIR)/Makefile.iondb-file
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/dictionary -f $(CURDIR)/Makefile.iondb-dictionary
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/dictionary/flat_file -f $(CURDIR)/Makefile.iondb-dictionary-flatfile

iondb-dictionary-bpptree:
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/dictionary/bpp_tree -f $(CURDIR)/Makefile.iondb-dictionary-bpptree

iondb-dictionary-linearhash:
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/dictionary/linear_hash -f $(CURDIR)/Makefile.iondb-dictionary-linearhash

iondb-dictionary-openaddressfilehash:
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/dictionary/open_address_file_hash -f $(CURDIR)/Makefile.iondb-dictionary-openaddressfilehash

iondb-dictionary-openaddresshash:
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/dictionary/open_address_hash -f $(CURDIR)/Makefile.iondb-dictionary-openaddresshash

iondb-dictionary-skiplist:
	"$(MAKE)" -C $(PKG_SOURCE_DIR)/src/dictionary/skip_list -f $(CURDIR)/Makefile.iondb-dictionary-skiplist