#ifndef __CIRCUITTREE_H__
#define __CIRCUITTREE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <cassert>
#include <sstream>
#include <utility>
#include <climits>

#define ISWHITESPACE(x) (x<=32)
#define reportx(x) std::cerr<<__FILE__<<":"<<__LINE__<<": "<<x<<std::endl
#define report() std::cerr<<__FILE__<<":"<<__LINE__<<std::endl
#define min(a,b) (a<b?a:b)
#define max(a,b) (a<b?b:a)
#define net(a,b) std::make_pair(a,b)
#define MAXVAL INT_MAX

extern std::string gatetypenames[];

enum gatetype
{
    GATE_INPUT = 0, GATE_OUTPUT = 1, GATE_DFF = 2, GATE_AND = 3, GATE_OR = 4, GATE_NAND = 5, GATE_NOR = 6, GATE_NOT = 7, 
    GATE_FANOUT = 8, GATE_BUFFER = 9
};

struct gate_t
{
    std::string name;
    gatetype type;
    int depth, scoapschoseninputdepth, scoapschoseninputrank;
    double scoapchoseninputerror0, scoapchoseninputerror1;
    int scoapchosenrankval0, scoapchosenrank10, scoapchosenrankval1, scoapchosenrank11;
    std::vector<gate_t*> input;
    std::vector<gate_t*> output;
    std::vector<std::string> inputname;
};

struct scoap_t
{
    int cc0, cc1, cb;
};

typedef std::pair<gate_t*,gate_t*> edge_t;

struct circuit_t
{
    std::vector<gate_t*> primaryinput, primaryoutput;
    std::vector<gate_t*> gatelist;
    std::map<std::string,gate_t*> nodes;
    std::map<edge_t,scoap_t> netlist;
    void addline (std::string line);
    void postprocessnetlist();
    void dumpdot (const char* filename, bool usenetlist = false, bool showdepth = false);
    void insertfanouts();
    void unfold();
    void unfold (std::vector<gate_t*> dfflist);
    void bypassdffs();
    void calculatecontrollability();
    void calculateobservability();
    void troubleshoot();
};

#endif
