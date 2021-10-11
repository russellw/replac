replac: *.cc *.h
	g++ -O3 -oreplac -pipe -s -std=c++17 *.cc

debug:
	g++ -DDEBUG -Og -fmax-errors=1 -g -oreplac -pipe -std=c++17 *.cc

prof:
	g++ -pg -oreplac -pipe -std=c++17 *.cc

clean:
	rm replac
	rm replac-$(version).tgz

install:
	mv replac /usr/local/bin

uninstall:
	rm /usr/local/bin/replac
