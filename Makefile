INSTALL_DIR=/usr/local/include/x2c

all: .test

.test:
	cd test && make

clean:
	cd test && make clean

install: test
	mkdir -p $(INSTALL_DIR)
	cp -av include/*.h $(INSTALL_DIR)
