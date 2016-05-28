#
# File : app/Makefile
#
#  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006

# Path to the top directory of the wlan distribution
PATH_TO_TOP = ../..

# Determine how we should copy things to the install directory
ABSPATH := $(filter /%, $(INSTALLDIR))
RELPATH := $(filter-out /%, $(INSTALLDIR))
INSTALLPATH := $(ABSPATH)
ifeq ($(strip $(INSTALLPATH)),)
INSTALLPATH := $(PATH_TO_TOP)/$(RELPATH)
endif

# Override CFLAGS for application sources, remove __ kernel namespace defines
CFLAGS := $(filter-out -D__%, $(CFLAGS))


# Add the wlan driver directory to the preprocessor include path
CFLAGS +=  -I$(PATH_TO_TOP)/wlan


#
# List of application executables to create
#
TARGETS := wlanconfig

#
# Make target rules
#

# All rule compiles list of TARGETS using builtin program target from src rule
all : $(TARGETS)

# Install rule; INSTALLPATH calculated above from passed INSTALLDIR variable
$(INSTALLPATH)/% : ./%
	@cp -f $< $@

# Map build and install invocation to the install rule for each TARGET file
build install: $(addprefix $(INSTALLPATH)/, $(TARGETS))

clean:
	@rm -f $(TARGETS)

distclean: clean
	@rm -f *~ core
