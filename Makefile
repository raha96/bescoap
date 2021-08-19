
default: src/bescoap.cpp circuittree
	g++ build/circuittree.o src/bescoap.cpp -o build/bescoap

circuittree: src/circuittree.cpp src/circuittree.h
	g++ -c src/circuittree.cpp -o build/circuittree.o

csv: src/scoapcsv.cpp circuittree
	g++ build/circuittree.o src/scoapcsv.cpp -o build/scoapcsv

benchalyzer: benchalyzer.cpp circuittree
	g++ build/circuittree.o benchalyzer.cpp -o build/benchalyzer