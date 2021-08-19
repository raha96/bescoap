# bescoap
Calculate SCOAP testability parameters for a given Bench file

To build using gcc, enter 

`make`

To run, use

`./build/bescoap <bench-file> <dot-file>`

This would generate a graph file using _graphviz dot_ notation. For generating the output graph in PNG format, use 

`dot -Tpng <dot-file> -o <png-file>`

Note that graphviz should be installed for this command to work. In Ubuntu graphviz is available through default repositories: 

`sudo apt install graphviz`

For other operating systems, refer to [its website](https://graphviz.org/download/). For windows, exe installer would probably work best. 

The above two commands have been condensed into a bash file: 

`./build/bescoap-png.sh <bench-file> <png-file>`

This can be used to test the program with the provided default sample file: 

`./build/bescap ./build/bescoap-png.sh samples/s27.bench samples/s27`

Two output files would be generated: _samples/s27.dot_ containing the dot graph description and _samples/27.png_ as the visualization.

TODO: Add windows support, along with the necessary documentation. Add sample graph to this document. 