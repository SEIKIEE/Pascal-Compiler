

#include "genCode.hpp"

string regs[REG_SIZE + 4] = { "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "a0", "a1", "a2", "a3" };
string argregs[4] = { "a0", "a1", "a2", "a3" };
int regValid[REG_SIZE];
vector<vector<string> > line;
vector<string> variable;
vector<int> variable2;
map<string, int> table;
vector<int> tempreg;
vector<string> params;
int argument;
int argument1;
int argument2;

void buildCode(string irPath, string obPath)
{
    /* initialize all registers */
    for (int i = 0; i < REG_SIZE; i++) {
        regValid[i] = 1;
    }
    argument = REG_SIZE;
    argument1 = argument2 = 0;
    loadIrcode(irPath);
    loadVariable();
    assignRegister();
    string obcode;
    for (int i = 0; i < line.size(); i++) {
        vector<string> l = line[i];
        string s = translate(l);
        if (s != "")
            obcode.append(s).append("\n");
    }
    writeCode(obPath, obcode);
}

void writeCode(string path, string obcode)
{
    ofstream f(path);
    f << ".data\n"
      << "_prompt: .asciiz \"Enter an integer:\"\n"
      << "_ret: .asciiz \"\\n\"\n"
      << ".globl main\n"
      << ".text\n"
      << "READ:\n"
      << "    li $v0,4\n"
      << "    la $a0,_prompt\n"
      << "    syscall\n"
      << "    li $v0,5\n"
      << "    syscall\n"
      << "    jr $ra\n"
      << "WRITE:\n"
      << "    li $v0,1\n"
      << "    syscall\n"
      << "    li $v0,4\n"
      << "    la $a0,_ret\n"
      << "    syscall\n"
      << "    move $v0,$0\n"
      << "    jr $ra\n"
      << "WRITELN:\n"
      << "    li $v0,1\n"
      << "    syscall\n"
      << "    li $v0,4\n"
      << "    la $a0,_ret\n"
      << "    syscall\n"
      << "    move $v0,$0\n"
      << "    jr $ra\n";
    f << obcode;
    f.close();
}

void loadIrcode(string path)
{
    fstream f(path);
    string lines;
    vector<string> temp;
    while (getline(f, lines)) {
        temp.clear();
        int i;
        while ((i = lines.find(' ')) != string::npos) {
            string s = lines.substr(0, i);
            if (s.size() != 0)
                temp.push_back(s);
            lines = lines.substr(i + 1);
        }
        if (!lines.empty())
            temp.push_back(lines);
        line.push_back(temp);
    }
    f.close();
}

void loadVariable()
{
    for (int i = 0; i < line.size(); i++) {
        for (int j = 0; j < line[i].size(); j++) {
            if (line[i][j].substr(0, 3) == "var") {
                variable.push_back(line[i][j]);
            }
        }
    }
}

void assignRegister()
{
    for (int i = 0; i < line.size(); i++) {
        if (line[i][0] == "PARAM") {
            params.push_back(line[i][1]);
            continue;
        }
        for (int j = 0; j < line[i].size(); j++) {
            if (line[i][j].substr(0, 4) == "temp" && find(params.begin(), params.end(), line[i][j]) == params.end()) {
                if (table.find(line[i][j]) == table.end()) {
                    for (int t = 0; t < REG_SIZE; t++) {
                        if (regValid[t] == 1) {
                            variable2.push_back(t);
                            table.insert(make_pair(line[i][j], t));
                            regValid[t] = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
}

string getRegister(string s)
{
    vector<string>::iterator it = find(variable.begin(), variable.end(), s);
    if (it != variable.end()) {
        variable.erase(it);
    }
    map<string, int>::iterator it2 = table.find(s);
    if (it2 != table.end()) {
        return "$" + regs[it2->second];
    }
    vector<string> keys;
    for (map<string, int>::iterator it = table.begin(); it != table.end(); it++) {
        keys.push_back(it->first);
    }
    for (int i = 0; i < keys.size(); i++) {
        string key = keys[i];
        if (key.substr(0, 3) == "var" && find(variable.begin(), variable.end(), key) == variable.end()) {
            map<string, int>::iterator it = table.find(key);
            regValid[it->second] = 1;
            table.erase(it);
        }
    }
    for (int i = 0; i < REG_SIZE; i++) {
        if (regValid[i] == 1) {
            table.insert(make_pair(s, i));
            regValid[i] = 0;
            return "$" + regs[i];
        }
    }
    return "";
}

string getTempRegister()
{
    for (int i = 0; i < REG_SIZE; i++) {
        if (regValid[i] == 1) {
            regValid[i] = 0;
            tempreg.push_back(i);
            return "$" + regs[i];
        }
    }
    return "";
}

string emitCallBeforeCode()
{
    string s = "";
    int memory = 0;
    for (int i = 0; i < tempreg.size(); i++) {
        s += "\tsw $" + regs[tempreg[i]] + "," + to_string(memory) + "($sp)\n";
        memory += 4;
    }
    s += "\tsw $ra," + to_string(memory) + "($sp)\n";
    memory += 4;
    for (int i = 0; i < REG_SIZE; i++) {
        if (regValid[i] == 0 && find(tempreg.begin(), tempreg.end(), i) == tempreg.end() && find(variable2.begin(), variable2.end(), i) == variable2.end()) {
            s += "\tsw $" + regs[i] + "," + to_string(memory) + "($sp)\n";
            memory += 4;
        }
    }
    s = "\taddi $sp,$sp,-" + to_string(memory) + "\n" + s;
    return s;
}

string emitCallAfterCode()
{
    string s = "";
    int memory = 0;
    for (int i = 0; i < tempreg.size(); i++) {
        s += "\tlw $" + argregs[i] + "," + to_string(memory) + "($sp)\n";
        memory += 4;
    }
    s += "\tlw $ra," + to_string(memory) + "($sp)\n";
    memory += 4;
    for (int i = 0; i < REG_SIZE; i++) {
        if (regValid[i] == 0 && find(tempreg.begin(), tempreg.end(), i) == tempreg.end() && find(variable2.begin(), variable2.end(), i) == variable2.end()) {
            s += "\tlw $" + regs[i] + "," + to_string(memory) + "($sp)\n";
            memory += 4;
        }
    }
    s += "\taddi $sp,$sp," + to_string(memory) + "\n";
    return s;
}

void releaseTempReg()
{
    for (int i = 0; i < REG_SIZE; i++) {
        if (regValid[i] == 0 && (find(tempreg.begin(), tempreg.end(), i) != tempreg.end() || find(variable2.begin(), variable2.end(), i) == variable2.end())) {
            regValid[1] = 0;
        }
    }
    tempreg.clear();
}

string translate(vector<string> line)
{
    if (line[0] != "PARAM") {
        argument = REG_SIZE;
    }
    if (line[0] == "LABEL") {
        return line[1] + ":";
    }
    if (line[0] == "GOTO")
        return "\tj " + line[1];
    if (line[0] == "RETURN") {
        if (line[1] == "#0")
            return "\tmove $v0,$zero\n\tjr $ra";
        return "\tmove $v0," + getRegister(line[1]) + "\n\tjr $ra";
    }
    if (line[0] == "IF_FALSE") {
        return "\tbeq " + getRegister(line[1]) + ",$zero," + line[3];
    }
    if (line[0] == "IF") {
        return "\tbne " + getRegister(line[1]) + ",$zero," + line[3];
    }
    if (line[0] == "FUNCTION") {
        return line[1] + ":";
    }
    if (line[0] == "CALL") {
        argument1 = 0;
        if (line[line.size() - 1] == "READ" or line[line.size() - 1] == "WRITE" or line[line.size() - 1] == "WRITELN") {
            return "\taddi $sp,$sp,-8\n\tsw $a0,0($sp)\n\tsw $ra,4($sp)\n\tjal " + line[line.size() - 1] + "\n\tlw $a0,0($sp)\n\tlw $ra,4($sp)\n\taddi $sp,$sp,8";
        } else {
            string s = emitCallBeforeCode();
            s += "\tjal " + line[line.size() - 1] + "\n";
            s += emitCallAfterCode();
            s += "\tmove " + getRegister(line[0]) + " $v0";
            releaseTempReg();
            return s;
        }
    }
    if (line[0] == "ARG") {
        string s = "\tmove " + getTempRegister() + ",$" + argregs[argument1] + "\n\tmove $" + argregs[argument1] + "," + getRegister(line[line.size() - 1]);
        argument1++;
        return s;
    }
    if (line[0] == "PARAM") {
        return "\tmove " + getRegister(line[1]) + ",$" + regs[argument++];
    }
    if (line.size() > 1 && line[1] == "=") {
        if (line.size() == 3) {
            if (line[2][0] == '#') {
                return "\tli " + getRegister(line[0]) + "," + line[2].substr(1);
            } else {
                return "\tmove " + getRegister(line[0]) + "," + getRegister(line[2]);
            }
        } else if (line.size() == 5) {
            if (line[3] == "+") {
                if (line[4][0] == '#') {
                    return "\taddi " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + line[4].substr(1);
                } else {
                    return "\tadd " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
                }
            } else if (line[3] == "-") {
                if (line[4][0] == '#') {
                    return "\taddi " + getRegister(line[0]) + "," + getRegister(line[2]) + ",-" + line[4].substr(1);
                } else {
                    return "\tsub " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
                }
            } else if (line[3] == "*") {
                return "\tmul " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
            } else if (line[3] == "/") {
                return "\tdiv " + getRegister(line[2]) + "," + getRegister(line[4]) + "\n\tmflo " + getRegister(line[0]);
            } else if (line[3] == "%") {
                return "\tdiv " + getRegister(line[2]) + "," + getRegister(line[4]) + "\n\tmfhi " + getRegister(line[0]);
            } else if (line[3] == "<") {
                return "\tslt " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
            } else if (line[3] == ">") {
                return "\tsgt " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
            } else if (line[3] == "==") {
                return "\tseq " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
            } else if (line[3] == "!=") {
                return "\tsne " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
            } else if (line[3] == ">=") {
                return "\tsge " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
            } else if (line[3] == "<=") {
                return "\tsle " + getRegister(line[0]) + "," + getRegister(line[2]) + "," + getRegister(line[4]);
            }
        }
        if (line.size() > 2 && line[2] == "CALL") {
            argument1 = 0;
            if (line[3] == "READ" or line[3] == "WRITE" or line[line.size() - 1] == "WRITELN") {
                return "\taddi $sp,$sp,-8\n\tsw $a0,0($sp)\n\tsw $ra,4($sp)\n\tjal " + line[line.size() - 1] + "\n\tlw $a0,0($sp)\n\tlw $ra,4($sp)\n\t move " + getRegister(line[0]) + ",$v0\n\taddi $sp,$sp,8";
            } else {
                string s = emitCallBeforeCode();
                s += "\tjal " + line[line.size() - 1] + "\n";
                s += emitCallAfterCode();
                s += "\tmove " + getRegister(line[0]) + " $v0";
                releaseTempReg();
                return s;
            }
        }
    }
    return "";
}

