CXX=g++-4.8
CXXARGS=--std=gnu++0x -Wall

INC=-I../include -I/usr/include/libxml++-2.6 -I/usr/include/glibmm-2.4

C=$(CXX) $(CXXARGS) $(INC)

TARGETS=sandbox sandbox-prof sandbox-opt sandbox-opt-prof

#all: $(TARGETS)
all: test

include .depend

.depend: sandbox.cc xml_sandbox.cc
	$C --depend sandbox.cc | sed 's/^sandbox\.o:/$(TARGETS):/' > $@
	$C --depend xml_sandbox.cc | sed 's/^xml_sandbox\.o:/xml_sandbox:/' >> $@

test: xml_test-ndebug
	./$<

xml_test-ndebug: xml_test.cc
	$C $< -o $@ -DNDEBUG -ggdb -lexpat

xml_test: xml_test.cc
	$C $< -o $@ -ggdb -lexpat

sandbox: sandbox.cc 
	$C $< -o $@ -ggdb

xml_sandbox: xml_sandbox.cc 
	$C $< -o $@ -lexpat -ggdb

sandbox-prof: sandbox.cc 
	$C $< -o $@ -pg -ggdb

sandbox-opt: sandbox.cc 
	$C $< -o $@ -O3 -DNDEBUG

sandbox-opt-prof: sandbox.cc 
	$C $< -o $@ -pg -O3 -DNDEBUG

clean:
	rm -f sandbox-* sandbox .depend xml_sandbox
