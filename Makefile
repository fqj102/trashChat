SUBDIRS := $(wildcard */.)

.PHONY : all $(SUBDIRS)
all : $(SUBDIRS)

$(SUBDIRS) :
	-$(MAKE) -C $@ clean all
clean:
	rm -f *.o
	rm -f *.so
