#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "circuittree.h"

using namespace std;

int main (int argc, char* argv[])
{
    if (argc < 3)
    {
        cerr << "Usage: bescoap <input.bench> <output.dot>" << endl;
        cerr << "Note: To avoid naming collisions, avoid gate names with `PI_`, `PO_` and `FO_` prefixes, ";
        cerr << "as well as `_#` suffixes (# denotes any cardinal number)." << endl;
    }
    else
    {
        circuit_t cut;
        ifstream fin (argv[1]);
        string line;
        while (!fin.eof())
        {
            getline(fin, line);
            cut.addline(line);
        }
        fin.close();
        cut.postprocessnetlist();
        cerr << "Input parsed." << endl;
        cut.insertfanouts();

        vector<gate_t*> dffs;
        for (int i = 4; i < argc; i++)
        {
            string name(argv[i]);
            if (cut.nodes[name] == nullptr)
                cerr << "ERROR: Gate `" << name << "` not found." << endl;
            else if (cut.nodes[name]->type != GATE_DFF)
                cerr << "ERROR: " << name << " is not of type DFF." << endl;
            else
                dffs.push_back(cut.nodes[name]);
        }

        if (dffs.size() > 0)
            cut.unfold(dffs);
        else
            cut.unfold();
        
        cut.calculatecontrollability();
        cut.dumpdot(argv[2], true);
        cerr << "Output dumped." << endl;
    }
    return 0;
}