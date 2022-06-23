SUBDIRS := $(wildcard */.)

all: $(SUBDIRS)
$(SUBDIRS):
	echo "make arg is" $(MAKECMDGOALS)
	$(MAKE) -C $@ $(MAKECMDGOALS)
	#$(MAKE) -C $@

.PHONY: all $(SUBDIRS)

SUBCLEAN = $(addsuffix .clean,$(SUBDIRS))

clean: $(SUBCLEAN)

$(SUBCLEAN): %.clean:
	$(MAKE) -C $* clean
