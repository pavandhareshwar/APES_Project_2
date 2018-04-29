CC = cc
CFLAGS = -std=c99 -Wall -Werror -g -O0

#
# build third-party libraries
#
SUBDIRS = 3rd-party

ifdef SUBDIRS
.PHONY : $(SUBDIRS)
$(SUBDIRS) : 
	@if [ -d $@ ]; then \
		$(MAKE) --no-print-directory --directory=$@ \
			CC=$(CC) CFLAGS="$(CFLAGS)" $(MAKECMDGOALS); \
	fi	
endif


THIRD_PARTY_DIR = 3rd-party
CMOCKA_INCLUDE_DIR = $(THIRD_PARTY_DIR)/build-Debug/include
CMOCKA_LIBRARY = $(THIRD_PARTY_DIR)/build-Debug/lib/libcmocka.a

cmocka : $(SUBDIRS)

CFLAGS += -I $(CMOCKA_INCLUDE_DIR)

#
# quick and dirty build rules to demonstrate testing, not good build
# system practices.
#

#
# library
#
LIB_SRCS = \
	application.c 

LIB_OBJS = $(LIB_SRCS:%.c=%.o)

LIB_HEADERS = $(LIB_SRCS:%.c=%.h)

LIB = libutils.a



#
# tests
#
TEST_APPLICATION_SRCS = \
	test_application.c

TEST_APPLICATION_OBJS = $(TEST_APPLICATION_SRCS:%.c=%.o)

TEST_APPLICATION_EXE = test-application.out


#
# Generic rule to generate objects
#
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

test : $(TEST_APPLICATION_EXE)
	./$(TEST_APPLICATION_EXE)

$(TEST_APPLICATION_EXE) : $(TEST_APPLICATION_SRCS) $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(CMOCKA_LIBRARY) -lm


.PHONY : clean
clean : FORCE
	-rm -rf *~ *.o $(LIB) $(TEST_APPLICATION_EXE) *.dSYM/

.PHONY : clobber
clobber : $(SUBDIRS) clean


FORCE :
