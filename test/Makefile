include Makevars

SRC=test_main.cc test_structure.cc test_idioms.cc test_xml_structure.cc test_input.cc test_iterators.cc test_predicates.cc
OBJ=$(subst .cc,.o,$(SRC))
COV_OBJ=$(subst .cc,.cov.o,$(SRC))

DEBUG_OPTS=-gdwarf-2 -gstrict-dwarf

COV_OPTS=-g -fprofile-arcs -ftest-coverage
COV_OPTS=$(DEBUG_OPTS) --coverage

INFO_FILE=test-coverage.info

all:  test

dump:
	@echo $(SRC)
	@echo $(OBJ)
	@echo $(COV_OBJ)

include .depend

.depend: $(SRC) Makefile
	$C --depend $(SRC) | sed 's/^\([^ ]*\)[.]o/\1.o \1.cov.o/' > $@

test: .test-OK

.test-OK: xml_test-ndebug
	./$< && touch $@ || rm -f $@

test-coverage: xml_test
	lcov --zerocounters --directory .
	./$< | tail -3
	lcov --gcov-tool $(COV) --no-checksum --directory . --capture --output-file $(INFO_FILE)
	lcov --remove $(INFO_FILE) "/usr*" -o $(INFO_FILE)
	lcov --remove $(INFO_FILE) "catch.hpp" -o $(INFO_FILE)
	genhtml --highlight --legend --output-directory TestCodeCoverage $(INFO_FILE)

xml_test-ndebug: $(OBJ)
	$(LD) $(OBJ) $(LIBS) -o $@

$(OBJ):%.o: %.cc
	$C -c $(DEBUG_OPTS) $< -o $@ -DNDEBUG -g

$(COV_OBJ):%.cov.o: %.cc
	$C -c $< -o $@ $(COV_OPTS)

xml_test: $(COV_OBJ)
	$(C) $(COV_OBJ) $(COV_OPTS) $(LIBS) -o $@

#$(COV_OBJ):%.cov.o: %.cc
#	$C -c $< -o $@ $(COV_OPTS)

clean:
	rm -f .depend xml_test xml_test-ndebug *.o
