#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<map>
using namespace std;

//Pass1

//data structures

//condition and registers
map<string, int> conditionInd;
map<string, int> registerInd;
vector<pair<string, int>> symTab;
vector<pair<string, int>> litTab;

//mnemonics
struct mnemonics{
    string mnemonic;
    string m_class;
    int  opcode;
}optab[18];



void init_Pass1_DS(){

    registerInd.emplace("areg", 1);
    registerInd.emplace("breg", 2);
    registerInd.emplace("creg", 3);
    registerInd.emplace("dreg", 4);

    conditionInd.emplace("LT", 1);
    conditionInd.emplace("LE", 2);
    conditionInd.emplace("EQ", 3);
    conditionInd.emplace("GT", 4);
    conditionInd.emplace("GE", 5);
    conditionInd.emplace("ANY", 6);

    optab[0] = {"stop", "IS", 0};
    optab[1] = {"add", "IS", 1};
    optab[2] = {"sub", "IS", 2};
    optab[3] = {"mult", "IS", 3};
    optab[4] = {"mover", "IS", 4};
    optab[5] = {"movem", "IS", 5};
    optab[6] = {"comp", "IS", 6};
    optab[7] = {"bc", "IS", 7};
    optab[8] = {"div", "IS", 8};
    optab[9] = {"read", "IS", 9};
    optab[10] = {"print", "IS", 10};
    optab[11] = {"start", "AD", 1};
    optab[12] = {"end", "AD", 2};
    optab[13] = {"origin", "AD", 3};
    optab[14] = {"equ", "AD", 4};
    optab[15] = {"ltorg", "AD", 5};
    optab[16] = {"dc", "DL", 1};
    optab[17] = {"ds", "DL", 2}; 
} 

int ispresent_symTsb(string lab){
    for(int i = 0; i < symTab.size(); i++){
        if(symTab[i].first == lab) return i + 1;
    }
    
    pair<string, int> pr = {lab, 0};
    symTab.emplace_back(pr);
    
    return symTab.size();
}

void update_symTsb(string lab, int LC){
    for(int i = 0; i < symTab.size(); i++){
        if(symTab[i].first == lab) {symTab[i].second = LC; return;}
    }

    pair<string, int> pr = {lab, LC};
    symTab.emplace_back(pr);

    return;
}

int get_LC(string lab){
   for(int i = 0; i < symTab.size(); i++){
        if(symTab[i].first == lab) return symTab[i].second;
    } 
}

int processSubToken(string str){
    int nlc = 0;
    string o1, o2;
    char opr;
    for(auto c : str){
        if(c == '+' || c == '-'){
            opr = c;
            o1 = o2;
            o2 = "";
        }
        else{
            o2 += c;
        }
    }

    if(atoi(o2.c_str())){

        //op2 is number    
        if(opr == '+') nlc = get_LC(o1) + atoi(o2.c_str());
        else nlc = get_LC(o1) - atoi(o2.c_str());
    
    }
    else{
    
        if(opr == '+') nlc = get_LC(o1) + get_LC(o2);
        else nlc = get_LC(o1) - get_LC(o2);
    }

    return nlc;
}

bool issubtoken(string op){
    for(auto c : op) if(c == '+' || c == '-') return true;
    return false;
}

string IC_mnemonic(string mnemo){
    string mne = "(";
    int i = 0;
    for(; i < 18; i++) if(optab[i].mnemonic == mnemo) break;
    mne += optab[i].m_class;
    mne += ", ";
    mne += to_string(optab[i].opcode);
    mne += ")";
    return mne;
}

string IC_operand(string op){
    string operand = "-1";

    //registers
    if(op == "areg" || op == "breg" || op == "creg" || op == "dreg"){
        operand = "(";
        operand += to_string(registerInd.find(op)->second);
        operand += ")";
    }

    //conditions
    else if(op == "lt" || op == "le" || op == "eq" || op == "gt" || op == "ge" || op == "any"){
        operand = "(";
        operand +=  to_string(conditionInd.find(op)->second);
        operand +=  ")";
    }

    //literals
    else if(op[0] == '='){
        pair<string, int> pr;
        pr.first = op;
        pr.second = 0;
        litTab.emplace_back(pr);
        operand = "(l, ";
        operand +=  (pr.first).substr(2, (pr.first).length()- 3);
        operand +=  ")";
    }

    //string constants
    else if(op[0] == '\''){
        string str = op.substr(1, op.length()- 2);
        operand = "(c, ";
        operand +=  str;
        operand +=  ")";
    }

    //sub token
    else if(issubtoken(op)){

        string o1, o2;
        char opr;
        for(auto c : op){
            if(c == '+' || c == '-'){
                opr = c;
                o1 = o2;
                o2 = "";
            }
            else{
                o2 += c;
            }
        }

        operand = "(s, ";
        operand += to_string(ispresent_symTsb(o1));
        operand += ")";
        operand += opr;

        if(atoi(o2.c_str())){
            //op2 is number
            operand += o2;
        }
        else{
            operand = "(s, ";
            operand += to_string(ispresent_symTsb(o2));
            operand += ")";
        }
    }

    //numeric constants
    else if(atoi(op.c_str())){
        operand = "(c, ";
        operand +=  op;
        operand +=  ")";
    }

    //variables
    else{
        int ind = ispresent_symTsb(op);
        operand = "(s, ";
        operand +=  to_string(ind);
        operand +=  ")";
    }

    return operand;
}



void pass1(){
    
    init_Pass1_DS();

    int lc = 0; 
    int LitInd = 0; 
    fstream fin("testcase.txt", ios::in);
    fstream IC_fout("output1.txt", ios::out);
    fstream symTab_fout("symTab.txt", ios::out);
    fstream PoolTab_fout("PoolTab.txt", ios::out);
    fstream LitTab_fout("LitTab.txt", ios::out);
    string Str, label, opcode, op1, op2;
    bool end = false;
    getline(fin, Str);

    while(!end){

        stringstream ss;
        ss.str(Str);
        ss>>label>>opcode>>op1>>op2;
        
        //cout<<"label: "<<label<<"   opcode: "<<opcode<<"    op1: "<<op1<<"  op2: "<<op2<<endl;
        
        //label processing
        if(label != "$"  && opcode != "equ") update_symTsb(label, lc);

        //opcode processing
        if(opcode == "start"){
            lc = atoi(op1.c_str());
            cout<<"-"<<" ";
            IC_fout<<"-"<<" ";
        }

        else if(opcode == "stop" || opcode == "add" || opcode == "sub" || opcode == "mult" || opcode == "mover" || opcode == "movem" ||
                opcode == "comp" || opcode == "bc" || opcode == "div" || opcode == "read" || opcode == "print"){
            cout<<lc<<" ";
            IC_fout<<lc<<" ";
            lc++;
        }

        else if(opcode == "end"){
            cout<<"-"<<" ";
            IC_fout<<"-"<<" ";
            end = true;
        }

        else if(opcode == "dc"){
            cout<<lc<<" ";
            IC_fout<<lc<<" ";
            lc++;
        }

        else if(opcode == "ds"){
            cout<<lc<<" ";
            IC_fout<<lc<<" ";
            lc = lc + atoi(op1.c_str());
        }

        else if(opcode == "equ"){

            //process lc for sub token
            update_symTsb(label, processSubToken(op1));
        }

        else if(opcode == "origin"){
            cout<<"-"<<" ";
            IC_fout<<"-"<<" ";

            lc = processSubToken(op1);
        }

        else if(opcode == "ltorg"){
            
            PoolTab_fout<<"#" + to_string(LitInd + 1)<<endl;
            vector<pair<string, int>> :: iterator itr;
            string line = "(DL, 1) (c, "; 
            for(itr = litTab.begin() + LitInd; itr != litTab.end(); itr++){
                itr->second = lc;
                line += (itr->first).substr(2, (itr->first).length()- 3);
                line += ")";
                IC_fout<<to_string(lc)<<" "<<line<<endl;
                lc++;
                line = "(DL, 1) (c, ";
            }
            
            LitInd = litTab.size();

        }

        string IC_opcode, IC_op1, IC_op2;

        IC_opcode = IC_mnemonic(opcode);
        
        //operand processing
        if(op1 != "-"){IC_op1 = IC_operand(op1);}

        if(op2 != "-"){IC_op2 = IC_operand(op2);}
        
        //printing IC
        if(opcode != "equ" && opcode != "ltorg"){
            cout<<IC_opcode<<" "<<IC_op1<<" "<<IC_op2<<endl;        
            IC_fout<<IC_opcode<<" "<<IC_op1<<" "<<IC_op2<<endl;
        }

        getline(fin, Str);
    }


    cout<<endl;
    cout<<"Sym tab"<<endl;
    for(int i = 0; i < symTab.size(); i++){
        cout<<i+1<<"  "<<symTab[i].first<<"   "<<symTab[i].second<<endl;
        symTab_fout<<i+1<<" "<<symTab[i].first<<" "<<to_string(symTab[i].second)<<endl;
    }

    if(LitInd != litTab.size()){
        PoolTab_fout<<"#" + to_string(LitInd + 1)<<endl;
        vector<pair<string, int>> :: iterator itr;
        string line = "(DL, 1) (c, "; 
        for(itr = litTab.begin() + LitInd; itr != litTab.end(); itr++){
            itr->second = lc;
            line += (itr->first).substr(2, (itr->first).length()- 3);
            line += ")";
            IC_fout<<to_string(lc)<<" "<<line<<endl;
            lc++;
            line = "(DL, 1) (c, ";
        }
    }
            
    cout<<endl;
    cout<<"Literal tab"<<endl;
    for(int i = 0; i < litTab.size(); i++){
        cout<<i+1<<"  "<<litTab[i].first<<"   "<<litTab[i].second<<endl;
        LitTab_fout<<i+1<<" "<<litTab[i].first<<" "<<to_string(litTab[i].second)<<endl;
    }
}


int main(){
    pass1();
    /*string opc = "='5'";
    cout<<IC_operand("='5'")<<endl;
    cout<<IC_operand("='1'")<<endl;
    cout<<IC_operand("='1'")<<endl;
    for(int i = 0; i < litTab.size(); i++){
        cout<<i+1<<"  "<<litTab[i].first<<"   "<<litTab[i].second<<endl;
        //symTab_fout<<i+1<<" "<<litTab[i].first<<" "<<litTab[i].second<<endl;
    }*/
    return 0;
}