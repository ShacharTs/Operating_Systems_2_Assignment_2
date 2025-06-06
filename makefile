SUBDIRS = Level_1 Level_2 Level_3 Level_4 level_5 level_6

.PHONY: all clean

all:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean || true; \
	done
