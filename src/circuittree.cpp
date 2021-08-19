#include "circuittree.h"

std::string gatetypenames[] = {"INPUT", "OUTPUT", "DFF", "AND", "OR", "NAND", "NOR", "NOT", "FANOUT", "BUFFER"};

inline bool issep (char ch)
{
    return (ISWHITESPACE(ch) || ch == '=' || ch == '(' || ch == ')' || ch == '#' || ch == ',');
}

std::vector<std::string> tokenize (std::string input)
{
    std::vector<std::string> output;
    std::string ctok = "";
    input += " ";
    for (int i = 0; i < input.size(); i++)
    {
        if (issep(input[i]))
        {
            if (ctok.size() > 0)
            {
                output.push_back(ctok);
                ctok = "";
            }
            if (input[i] == '#')
                break;
        }
        else
            ctok += input[i];
    }
    return output;
}

void circuit_t::addline (std::string line)
{
    gate_t* gate = new gate_t;
    std::vector<std::string> token = tokenize(line);
    if (token.size() > 0)
    {
        if (token.size() == 1)
            std::cerr << "Corrupted line: " << line << std::endl;
        else if (token.size() == 2)   // IO
        {
            if (token[0] == "INPUT")
            {
                gate->type = GATE_INPUT;
                this->primaryinput.push_back(gate);
            }
            else if (token[0] == "OUTPUT")
            {
                gate->type = GATE_OUTPUT;
                this->primaryoutput.push_back(gate);
            }
            else
                std::cerr << "Unknown IO type: " << token[0] << std::endl;
            gate->name = token[1];
        }
        else if (token.size() > 2)
        {
            gate->name = token[0];
            if (token[1] == "AND")
                gate->type = GATE_AND;
            else if (token[1] == "OR")
                gate->type = GATE_OR;
            else if (token[1] == "NAND")
                gate->type = GATE_NAND;
            else if (token[1] == "NOR")
                gate->type = GATE_NOR;
            else if (token[1] == "NOT")
                gate->type = GATE_NOT;
            else if (token[1] == "DFF")
                gate->type = GATE_DFF;
            else
                std::cerr << "Unknown gate type: " << token[1] << std::endl;
            for (int i = 2; i < token.size(); i++)
            {
                gate->inputname.push_back(token[i]);
            }
        }
        gate_t* original = this->nodes[gate->name];
        if (original != nullptr) // duplicate name: either a PO or a gate that leads to a PO
        {
            if (gate->type == GATE_OUTPUT)
            {
                assert (original->type != GATE_OUTPUT);
                gate->name = "PO_" + gate->name;
                gate->inputname.push_back(original->name);
                reportx("Untested code!");
            }
            else
            {
                assert (gate->type != GATE_OUTPUT);
                original->name = "PO_" + original->name;
                this->nodes[original->name] = original;
                original->inputname.push_back(gate->name);
            }
        }
        this->nodes[gate->name] = gate;
        this->gatelist.push_back(gate);
    }
}

void circuit_t::postprocessnetlist()
{
    /**
    for (int i = 0; i < this->gatelist.size(); i++)
        std::cerr << this->gatelist[i]->name << '~' << (this->nodes[this->gatelist[i]->name]==nullptr) << ' ';
    std::cerr << std::endl;
    /**/
    for (int i = 0; i < this->gatelist.size(); i++)
    {
        gate_t* cgate = this->gatelist[i];
        for (int i = 0; i < cgate->inputname.size(); i++)
        {
            assert (this->nodes[cgate->inputname[i]] != nullptr);
            gate_t* cinput = this->nodes[cgate->inputname[i]];
            cgate->input.push_back(cinput);
            cinput->output.push_back(cgate);
        }
    }
}

void circuit_t::dumpdot (const char* filename, bool usenetlist, bool showdepth)
{
    std::ofstream fout (filename);
    std::map<gate_t*,bool> visited;
    std::queue<gate_t*> gates;

    fout << "digraph \"dump\" {\n";

    for (int i = 0; i < this->primaryinput.size(); i++)
    {
        gates.push(primaryinput[i]);
        visited[primaryinput[i]] = true;
    }
    
    while (gates.size())
    {
        gate_t* cgate = gates.front();
        gates.pop();
        fout << "   " << cgate->name << "[label=\"" << cgate->name << "\\n(";
        fout << gatetypenames[cgate->type] << ")\"];\n";
        for (int i = 0; i < cgate->output.size(); i++)
        {
            //fout << "   " << cgate->output[i]->name << " -> " << cgate->name << ";\n";
            fout << "   " << cgate->name << " -> " << cgate->output[i]->name;
            if (usenetlist)
            {
                fout << " [label=\"[" << this->netlist[net(cgate, cgate->output[i])].cc0;
                fout << ", " << this->netlist[net(cgate, cgate->output[i])].cc1 << "]";
                if (showdepth)
                    fout << "\\n" << cgate->depth;
                fout << "\"]";
            }
            fout << ";\n";
            if (visited[cgate->output[i]] == false)
            {
                visited[cgate->output[i]] = true;
                gates.push(cgate->output[i]);
            }
        }
    }

    fout << "}\n";
    fout.close();
}

std::string generatename (std::string base, int number)
{
    std::stringstream ss;
    ss << base << "_" << number;
    return ss.str();
}

void circuit_t::insertfanouts()
{
    for (int i = 0; i < this->gatelist.size(); i++)
        if (this->gatelist[i]->type != GATE_FANOUT)
        {
            gate_t* gate = this->gatelist[i];
            if (gate->output.size() > 1)
            {
                gate_t* fanout = new gate_t;
                fanout->type = GATE_FANOUT;
                fanout->name = "FO_" + gate->name;
                fanout->input.push_back(gate);
                for (int i = 0; i < gate->output.size(); i++)
                {
                    gate_t* target = gate->output[i];
                    fanout->output.push_back(target);
                    for (int i = 0; i < target->input.size(); i++)
                        if (target->input[i] == gate)
                        {
                            target->input[i] = fanout;
                            break;
                        }
                    this->nodes[fanout->name] = fanout;
                    this->gatelist.push_back(fanout);
                }
                gate->output.clear();
                gate->output.push_back(fanout);
            }
        }
}

void circuit_t::unfold()
{
    std::vector<gate_t*> dfflist;
    for (int i = 0; i < this->gatelist.size(); i++)
        if (this->gatelist[i]->type == GATE_DFF)
            dfflist.push_back(this->gatelist[i]);
    unfold(dfflist);
}

void circuit_t::unfold (std::vector<gate_t*> dfflist)
{
    for (int i = 0; i < dfflist.size(); i++)
    {
        gate_t* pi = new gate_t;
        gate_t* po = new gate_t;
        gate_t* predecessor = dfflist[i]->input[0];
        gate_t* successor = dfflist[i]->output[0];
        pi->name = "PI_" + dfflist[i]->name;
        po->name = "PO_" + dfflist[i]->name;
        pi->type = GATE_INPUT;
        po->type = GATE_OUTPUT;
        pi->output.push_back(successor);
        po->input.push_back(predecessor);
        for (int j = 0; j < successor->input.size(); j++)
            if (successor->input[j] == dfflist[i])
            {
                successor->input[j] = pi;
                break;
            }
        for (int j = 0; j < predecessor->output.size(); j++)
            if (predecessor->output[j] == dfflist[i])
            {
                predecessor->output[j] = po;
                break;
            }
        this->nodes[po->name] = po;
        this->gatelist.push_back(po);
        this->nodes[pi->name] = pi;
        this->gatelist.push_back(pi);
        this->primaryinput.push_back(pi);
        this->primaryoutput.push_back(po);
    }
    for (int i = 0; i < dfflist.size(); i++)
    {
        int pos;
        for (pos = 0; pos < this->gatelist.size(); pos++)
            if (this->gatelist[pos] == dfflist[i])
                break;
        assert (pos < this->gatelist.size());
        this->gatelist.erase(this->gatelist.begin() + pos);
        dfflist[i]->inputname.clear();
        dfflist[i]->input.clear();
        dfflist[i]->output.clear();
        delete dfflist[i];
    }
    dfflist.clear();
}

void circuit_t::calculatecontrollability()
{
    std::queue<gate_t*> list;
    std::map<gate_t*,int> markcount;
    for (int i = 0; i < this->primaryinput.size(); i++)
    {
        edge_t edge = std::make_pair(this->primaryinput[i], this->primaryinput[i]->output[0]);
        gate_t* child = this->primaryinput[i]->output[0];
        this->netlist[edge].cc0 = 1;
        this->netlist[edge].cc1 = 1;
        markcount[child]++;
        if (markcount[child] == child->input.size())
            // Otherwise, it would be later added to the list through the longest branch
            list.push(child);
        this->primaryinput[i]->depth = 0;
        this->primaryinput[i]->scoapschoseninputrank = 1;
        this->primaryinput[i]->scoapschoseninputdepth = 0;
    }
    while (list.size())
    {
        gate_t* gate = list.front();
        list.pop();
        gate->depth = 0;
        for (int i = 0; i < gate->input.size(); i++)
            gate->depth = max(gate->input[i]->depth + 1, gate->depth);
        int cc0 = 0, cc1 = 0;
        scoap_t rank1 = {0, 0, 0}, rankchosen = {0, 0, 0};
        switch (gate->type)
        {
            case GATE_NAND:
            case GATE_AND: 
                cc0 = MAXVAL;
                for (int i = 0; i < gate->input.size(); i++)
                {
                    if (cc0 > this->netlist[net(gate->input[i], gate)].cc0)
                    {
                        rankchosen = this->netlist[net(gate->input[i], gate)];
                        cc0 = rankchosen.cc0;
                        gate->scoapschoseninputdepth = gate->input[i]->depth;
                    }
                    cc1 += this->netlist[net(gate->input[i], gate)].cc1;
                }
                cc0++;
                cc1++;
                break;
            case GATE_NOR: 
            case GATE_OR: 
                cc1 = MAXVAL;
                for (int i = 0; i < gate->input.size(); i++)
                {
                    if (cc1 > this->netlist[net(gate->input[i], gate)].cc1)
                    {
                        rankchosen = this->netlist[net(gate->input[i], gate)];
                        cc1 = rankchosen.cc1;
                        gate->scoapschoseninputdepth = gate->input[i]->depth;
                    }
                    cc0 += this->netlist[net(gate->input[i], gate)].cc0;
                }
                cc0++;
                cc1++;
                break;
            case GATE_BUFFER: 
            case GATE_NOT: 
                cc0 = this->netlist[net(gate->input[0], gate)].cc0 + 1;
                cc1 = this->netlist[net(gate->input[0], gate)].cc1 + 1;
                break;
            case GATE_FANOUT: 
                cc0 = this->netlist[net(gate->input[0], gate)].cc0;
                cc1 = this->netlist[net(gate->input[0], gate)].cc1;
                break;
            case GATE_OUTPUT: 
                // Avoid emitting meaningless errors
                break;
            default: 
                std::cerr << "ERROR in gate " << gate->name << ": ";
                std::cerr << "Unexpected gate type or invalid for controllability calculation: " << gatetypenames[gate->type] << std::endl;
        }
        if (gate->type == GATE_NAND || gate->type == GATE_NOR || gate->type == GATE_NOT)
        {
            int buf = cc0;
            cc0 = cc1;
            cc1 = buf;
        }
        gate->scoapschoseninputrank = 1;
        gate->scoapchoseninputerror0 = 0;
        gate->scoapchoseninputerror1 = 0;
        gate->scoapchosenrankval0 = 0;
        gate->scoapchosenrankval1 = 0;
        gate->scoapchosenrank10 = 0;
        gate->scoapchosenrank11 = 0;
        if (gate->type == GATE_AND || gate->type == GATE_NAND || gate->type == GATE_OR || gate->type == GATE_NOR)
        {
            int mindepth = MAXVAL;
            for (int i = 0; i < gate->input.size(); i++)
            {
                if (gate->input[i]->depth < gate->scoapschoseninputdepth)
                    gate->scoapschoseninputrank++;
                if (mindepth > gate->input[i]->depth)
                {
                    mindepth = gate->input[i]->depth;
                    rank1 = this->netlist[net(gate->input[i], gate)];
                }
            }
            if (rank1.cc0 >= 1 && mindepth < MAXVAL)
                gate->scoapchoseninputerror0 = double(rankchosen.cc0 - rank1.cc0) / double(rankchosen.cc0);
                //gate->scoapchoseninputerror0 = rankchosen.cc0 - rank1.cc0;
            if (rank1.cc1 >= 1 && mindepth < MAXVAL)
                gate->scoapchoseninputerror1 = double(rankchosen.cc1 - rank1.cc1) / double(rankchosen.cc1);
                //gate->scoapchoseninputerror1 = rankchosen.cc1 - rank1.cc1;
            gate->scoapchosenrankval0 = rankchosen.cc0;
            gate->scoapchosenrankval1 = rankchosen.cc1;
            gate->scoapchosenrank10 = rank1.cc0;
            gate->scoapchosenrank11 = rank1.cc1;
        }
        for (int i = 0; i < gate->output.size(); i++)
        {
            gate_t* output = gate->output[i];
            markcount[output]++;
            netlist[net(gate, output)].cc0 = cc0;
            netlist[net(gate, output)].cc1 = cc1;
            if (markcount[output] == output->input.size())
                list.push(output);
        }
    }
}

void circuit_t::bypassdffs()
{
    for (int i = 0; i < this->gatelist.size(); i++)
        if (this->gatelist[i]->type == GATE_DFF)
            this->gatelist[i]->type = GATE_BUFFER;
}

void circuit_t::troubleshoot()
{
    std::map<gate_t*,bool> marked;
    for (int i = 0; i < this->primaryinput.size(); i++)
        marked[this->primaryinput[i]] = true;
    for (int i = 0; i < this->gatelist.size(); i++)
        if (this->gatelist[i]->type == GATE_INPUT && marked[this->gatelist[i]] == false)
            std::cerr << "Troubleshoot: " << this->gatelist[i]->name << " is an input, but not listed in circuit_t input list." << std::endl;
}

void circuit_t::calculateobservability()
{
    // Unimplemented
    /**
    std::queue<gate_t*> list;
    std::map<gate_t*,int> markcount;
    for (int i = 0; i < this->primaryoutput.size(); i++)
    {
        gate_t* target = this->primaryoutput[i]->input[0];
        netlist[net(target, this->primaryoutput[i])].cb = 1;
        markcount[target]++;
        if (markcount[target] == 1)
            list.push(target);
    }
    while (list.size())
    {
        gate_t* gate = list.front();
        list.pop();
        int cb = 0;
    }
    /**/
}