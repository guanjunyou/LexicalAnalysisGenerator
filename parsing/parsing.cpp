/**
  作者：华南师范大学 关竣佑
  2023.10.14
  */

#include "parsing.h"
#include "ui_parsing.h"
#include<iostream>
#include<map>
#include<vector>
#include<stack>
#include<unordered_map>
#include<queue>
#include<set>
#include<unordered_set>
#include<algorithm>
#include<string>
#include<fstream>
#include<QString>
#include<QFile>
#include <QTextStream>
#include <fstream>
#include <QFileDialog>
#include <QMessageBox>


using namespace std;


Parsing::Parsing(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Parsing)
{
    ui->setupUi(this);
}

Parsing::~Parsing()
{
    delete ui;
}

int stateCounter = 0;
int DFANodeCounter = 0;
int SimplifyDFACounter = 0;

map<char, int> priorityMap;
set<char> operatorChar;
set<char> operatorCharTmp;

// 定义 DFA 结点
struct DFANode {
    int id;
    vector<pair<char, DFANode*>> transitions;
    set<int> nfaNodes;
    bool isAccepting; // 是否终态 若一个DFA结点包含一个终态的NFA结点，则该DFA结点为终态
    bool isStarting; // 起点
    int SDFANum; // 化简后所属的  DFA 结点

    DFANode() {
        isAccepting = false;
        isStarting = false;
        id = DFANodeCounter;
        DFANodeCounter++;
    }
};

// 简化后的 DFA 结点
struct SimplifyDFANode {
    int id;
    vector<pair<char, SimplifyDFANode*>> transitions;
    set<DFANode*> dfaNodes;
    bool isAccepting;
    bool isStarting;

    SimplifyDFANode(int counter,bool isAc = false) {
        id = counter;
        isAccepting = isAc;
        isStarting = false;
    }
};

// 定义NFA的节点
struct State {
    int id;
    vector<pair<char, State*>> transitions;
    bool isAccepting; // 是否终态
    bool isStarting; // 是否起点
    State() {
        isAccepting = false;
        isStarting = false;
    }
};

vector<DFANode*> dfaNodes;
vector<State*> G[1000];
vector<DFANode*> DFAG[1000];
vector<SimplifyDFANode*> SDFAG[1000];
int outDegree[1000];
int inDegree[1000];
map<int, State*> NFAMap;
map<int, SimplifyDFANode*> SDFAMap;
queue<SimplifyDFANode*> SDFAQue;

// 整理成输出格式
string nfaTable[1000][1000];
string dfaTable[1000][1000];
string sdfaTable[1000][1000];


// 是否为变量
bool isWordChar(char c) {
    if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
        return true;
    return false;
}

// 定义NFA图的数据结构
struct NFA {
    State* start;
    State* end;
};


// 创建一个新的NFA节点
State* createState(int id, bool isAccepting = false) {
    State* newState = new State;
    newState->id = id;
    newState->isAccepting = isAccepting;
    NFAMap.insert(make_pair(id, newState));
  //  cout << "createState: " << id << endl;
    return newState;
}

// 执行NFA图的连接操作
NFA concatenate(NFA nfa1, NFA nfa2) {
    nfa1.end->transitions.push_back(make_pair('#', nfa2.start));
    nfa1.end->isAccepting = false;
    nfa2.start->isStarting = false;
    G[nfa1.end->id].push_back(nfa2.start);
    return { nfa1.start, nfa2.end };
}

// 执行NFA图的选择操作
NFA unionNFA(NFA nfa1, NFA nfa2) {
    State* newStart = createState(stateCounter++);
    newStart->isStarting = true;
    newStart->transitions.push_back(make_pair('#', nfa1.start));
    newStart->transitions.push_back(make_pair('#', nfa2.start));
    G[newStart->id].push_back(nfa1.start);
    G[newStart->id].push_back(nfa2.start);

    State* newEnd = createState(stateCounter++, true);
    nfa1.end->transitions.push_back(make_pair('#', newEnd));
    nfa2.end->transitions.push_back(make_pair('#', newEnd));
    G[nfa1.end->id].push_back(newEnd);
    G[nfa2.end->id].push_back(newEnd);

    nfa1.end->isAccepting = false;
    nfa2.end->isAccepting = false;
    nfa1.start->isStarting = false;
    nfa2.start->isStarting = false;

    return { newStart, newEnd };
}

// 执行NFA图的闭包操作
NFA closure(NFA nfa) {
    State* newStart = createState(stateCounter++);
    newStart->isStarting = true;
    State* newEnd = createState(stateCounter++,true);
    newStart->transitions.push_back(make_pair('#', nfa.start));
    G[newStart->id].push_back(nfa.start);
    nfa.end->transitions.push_back(make_pair('#', newEnd));
    G[nfa.end->id].push_back(newEnd);

    // 产生正闭包
    nfa.end->transitions.push_back(make_pair('#', nfa.start));
    G[nfa.end->id].push_back(nfa.start);

    //绕路走的零环
   newStart->transitions.push_back(make_pair('#', newEnd));
    G[newStart->id].push_back(newEnd);

    nfa.end->isAccepting = false;
    nfa.start->isStarting = false;

    return { newStart, newEnd };
}

// 执行NFA图的正闭包操作
NFA closurePostive(NFA nfa) {

    // 产生正闭包
    nfa.end->transitions.push_back(make_pair('#', nfa.start));
    G[nfa.end->id].push_back(nfa.start);

    return nfa;
}

// NFA 图的可选操作
NFA select(NFA nfa) {
    State* newStart = createState(stateCounter++);
    newStart->isStarting = true;
    State* newEnd = createState(stateCounter++,true);
    newStart->transitions.push_back(make_pair('#', nfa.start));
    nfa.end->transitions.push_back(make_pair('#', newEnd));
    newStart->transitions.push_back(make_pair('#', newEnd));

    G[newStart->id].push_back(nfa.start);
    G[nfa.end->id].push_back(newEnd);
    G[newStart->id].push_back(newEnd);

    nfa.end->isAccepting = false;
    nfa.start->isStarting = false;

    return { newStart , newEnd };
}

string intToString(int num) {
    string result = "";
    if (num == 0) {
        return "0";
    }

    while (num > 0) {
        int digit = num % 10;
        char digitChar = '0' + digit;
        result = digitChar + result;
        num /= 10;
    }
    return result;
}

// 将正则表达式转换为NFA图
NFA regexToNFA(string regex) {
    stack<NFA> nfaStack;
    stack<char> operatorStack;

    for (char c : regex) {

        if (c == '(') {
            operatorStack.push(c);
        }
        else if (c == ')') {
            while (!operatorStack.empty() && operatorStack.top() != '(') {
                char op = operatorStack.top();
                operatorStack.pop();
                if (op == '|') {
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(unionNFA(nfa1, nfa2));
                }
                else if (op == '.') {
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(concatenate(nfa1, nfa2));
                }
                else if (op == '*') {
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(closure(nfa1));
                }
                else if (op == '+') {
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(closurePostive(nfa1));
                }
                else if (op == '?') {
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(select(nfa1));
                }
            }
            operatorStack.pop(); // Pop the '('
        }
        else if (c == '*') {
            //cout << "闭包" << endl;
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(closure(nfa1));
        }
        else if (c == '+') {
            //cout << "正闭包" << endl;
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(closurePostive(nfa1));
        }
        else if (c == '?' ){
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(select(nfa1));
        }
        else if ((c == '|' || c == '.' || c == '?') && !operatorStack.empty() && priorityMap[c] <= priorityMap[operatorStack.top()]) {
            while (!operatorStack.empty() && priorityMap[c] <= priorityMap[operatorStack.top()]) {
                char op = operatorStack.top();
                operatorStack.pop();
                if (op == '|') {
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(unionNFA(nfa1, nfa2));
                }
                else if (op == '.') {
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(concatenate(nfa1, nfa2));
                }
            }
            operatorStack.push(c);
        }
        else if (c == '|' || c == '.' || c == '?') {
            //cout << "push :" << c << endl;
            operatorStack.push(c);
        }
        else {  // 不是运算符，建立新的边
            State* state1 = createState(stateCounter++);
            state1->isStarting = true;
            State* state2 = createState(stateCounter++,true);
            state1->transitions.push_back(make_pair(c, state2));
            operatorChar.insert(c);
            G[state1->id].push_back(state2);

            outDegree[state1->id]--;
            inDegree[state2->id]--;
            nfaStack.push({ state1, state2 });
        }
    }

    while (!operatorStack.empty()) {
        char op = operatorStack.top();
        operatorStack.pop();
        if (op == '|') {
            NFA nfa2 = nfaStack.top();
            nfaStack.pop();
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(unionNFA(nfa1, nfa2));
        }
        else if (op == '.') {
            NFA nfa2 = nfaStack.top();
            nfaStack.pop();
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(concatenate(nfa1, nfa2));
        }
    }

    return nfaStack.top();
}




void dfs(int node) {
    //把这个 NFA 结点加入到新的 DFA 结点
    if (node == 0)
    {
        node = 0;
    }
    dfaNodes[dfaNodes.size() - 1]->nfaNodes.insert(node);
    if (NFAMap[node]->isAccepting == true) {
        dfaNodes[dfaNodes.size() - 1]->isAccepting = true; // 设置终态
    }
    if (NFAMap[node]->isStarting == true) {
        dfaNodes[dfaNodes.size() - 1]->isStarting = true; // 设置初态
    }
    if (outDegree[node] <= 0) {
        return; // 到达边缘点了
    }
    //遍历下一个
    for (int i = 0; i < G[node].size(); i++) {
        dfs(G[node][i]->id);
    }
}

void dfsFindDFA(State* node, set<int>* nfaSet) {
    //把这个 NFA 结点加入到新的 DFA 结点
    nfaSet->insert(node->id);
    if (outDegree[node->id] <= 0) {
        return; // 到达边缘点了
    }
    //遍历下一个
    for (int i = 0; i < G[node->id].size(); i++) {
        dfsFindDFA(G[node->id][i], nfaSet);
    }
}

// 找这个NFA 的 # 边缘点的下一个 DFA 点是什么
vector<pair<char,DFANode*>> findNextDFANode(State* node) {
    vector<pair<char, DFANode*>> dfas;
    if (node->transitions.size() == 0)
        return dfas;

    for (auto edge : node->transitions) {
        set<int> nfaSet;
        dfsFindDFA(edge.second, &nfaSet);

        for (auto dfa : dfaNodes) {
            if (dfa->nfaNodes == nfaSet) {
                dfas.push_back(make_pair(edge.first, dfa));
            }
        }
    }
    return dfas;
}

void UnionDFA(DFANode* dfa1, DFANode* dfa2) {
    if (dfa2->isAccepting)
        dfa1->isAccepting = true;
    if (dfa2->isStarting)
        dfa1->isStarting = true;
    for (auto nfa : dfa2->nfaNodes) {
        dfa1->nfaNodes.insert(nfa);
    }

    dfaNodes.erase(remove(dfaNodes.begin(), dfaNodes.end(), dfa2), dfaNodes.end());

}

void NFAtoDFA() {
    // 计算每个 DFA 结点拥有哪些 NFA 结点
    for (int i = 0; i < stateCounter; i++) {
        if (inDegree[i] <= 0) {
            DFANode* NewDFANode = new DFANode();
            dfaNodes.push_back(NewDFANode);
            dfs(i);
        }
    }

    //for (int i = 0; i < DFANodeCounter; i++) {
    //    cout << i << " 拥有的NFA结点 " << endl;
    //    for (auto nfa : dfaNodes[i]->nfaNodes) {
    //        cout << nfa;
    //    }
    //    cout << endl;
    //}

    for (int i = 0; i < dfaNodes.size(); i++) {
        DFANode* dfaNode = dfaNodes[i];
        map<char, DFANode*> opDfaMap;
        for (auto nfa : dfaNode->nfaNodes) { // 遍历每一个结点

            if (outDegree[nfa] > 0) // 不是边缘
                continue;
            vector<pair<char, DFANode*>> nextNode = findNextDFANode(NFAMap[nfa]);
            if (nextNode.empty()) {
                continue;
            }
            if (NFAMap[nfa]->transitions.size() == 0) // 后面一条边也没有
                continue;
            for (int j = 0; j < nextNode.size(); j++) {
                if (opDfaMap.find(nextNode[j].first) != opDfaMap.end()) {
                    UnionDFA(opDfaMap[nextNode[j].first], nextNode[j].second); // 把 后面合并到前面
                    i--;

                }
                opDfaMap.insert(make_pair(nextNode[j].first, nextNode[j].second));
            }
        }
    }

    DFANodeCounter = dfaNodes.size();
    for (int i = 0; i < dfaNodes.size(); i++) {
        //cout << i << " : " << dfaNodes[i]->id << endl;
        if (i != dfaNodes[i]->id) {
            int oldId = dfaNodes[i]->id;
            int newId = i;
            dfaNodes[i]->id = newId;
        }
    }
    for (int i = 0; i < dfaNodes.size(); i++) {
       cout << i << " : " << dfaNodes[i]->id << endl;
    }


    // 连接 DFA 点


    for (int i = 0; i < DFANodeCounter; i++) {
        DFANode* dfaNode = dfaNodes[i];
        vector<pair<char, DFANode*>> nextNode;
        map<char,set<int>> nextNFAs;
        for(auto op : operatorChar) {
            set<int> tmp;
            nextNFAs.insert(make_pair(op,tmp));
        }

        for (auto nfa : dfaNode->nfaNodes) { // 遍历每一个结点
            if (outDegree[nfa] > 0) // 不是边缘
                continue;
            if(NFAMap[nfa]->transitions.size() == 0)
                continue;

            char op = NFAMap[nfa]->transitions[0].first;
            set<int> nfaSet = nextNFAs[op];
            dfsFindDFA(NFAMap[nfa]->transitions[0].second,&nfaSet);

            nextNFAs[op] = nfaSet;

        }

        for(auto op : operatorChar) {
            set<int> nfaSet = nextNFAs[op];
            if(nfaSet.empty())
                continue;
            for(auto dfa : dfaNodes) {
                if(dfa->nfaNodes == nfaSet) {
                    nextNode.push_back(make_pair(op,dfa));
                }
            }
        }
        for (int j = 0; j < nextNode.size(); j++) {

            dfaNode->transitions.push_back(make_pair(nextNode[j].first, nextNode[j].second));
            DFAG[dfaNode->id].push_back(nextNode[j].second);
        }
    }

    for (int i = 0; i < DFANodeCounter ; i++) {
        cout << i << " : ";
        for (int j = 0; j < DFAG[i].size(); j++) {
            cout << DFAG[i][j]->id<<DFAG[i][j]->isAccepting << " ";
        }
        cout << endl;
    }
    cout << " ------  DFA图结束 ---------" << endl;
}

vector<SimplifyDFANode*> SplitDFANode(SimplifyDFANode* node, char op) {
    map<int, set<DFANode*>> splitMap; // 若 经过这个运算符没有下一个点则记下一个点为 -1
    vector<SimplifyDFANode*> ans;
    for (auto dfanode : node->dfaNodes) {
        int nextNode = -1;
        for (int i = 0; i < dfanode->transitions.size(); i++) {
            pair<char, DFANode*> tmp = dfanode->transitions[i];
            if (tmp.first == op) {
                nextNode = tmp.second->id;
            }
        }
        if (splitMap.find(nextNode) == splitMap.end()) { // 第一次放入
            set<DFANode*> newset;
            newset.insert(dfanode);
            splitMap.insert(make_pair(nextNode, newset));
        }
        else {
            set<DFANode*> newset = splitMap[nextNode];
            newset.insert(dfanode);
            splitMap[nextNode] = newset;
        }
    }
    // 建立新的 简化DFA 结点
    int i = 0;
    for (auto dfaset : splitMap) {
        SimplifyDFANode* newDfaNode;
        if (i == 0) { // 填了那个删掉的 拆分前的DFA 结点的编号的坑,防止化简后的DFA结点编号不连续
            newDfaNode = new SimplifyDFANode(node->id);
        }
        else {
            if (SimplifyDFACounter == 0 && node->id == 0)
            {
                newDfaNode = new SimplifyDFANode(1);
            }
            else
                newDfaNode = new SimplifyDFANode(SimplifyDFACounter++);
        }
        for (auto dfanode : dfaset.second) { // 重写设置是否为终态
            if (dfanode->isAccepting) {
                newDfaNode->isAccepting = true;
            }
            if (dfanode->isStarting) {
                newDfaNode->isStarting = true;
            }
        }
        newDfaNode->dfaNodes = dfaset.second;
        ans.push_back(newDfaNode);
        i++;
    }
    return ans;
}

void SimplifyDFA() {
    // 把 DFA 结点分成非终态 和 终态 两个类型
    set<DFANode*> NotACSet;
    set<DFANode*> ACSet;

    SimplifyDFANode* newStart = NULL;
    SimplifyDFANode* newEnd = NULL;
    SimplifyDFACounter = 0;
    for (int i = 0; i < dfaNodes.size(); i++) {
        if (dfaNodes[i]->isAccepting) {
            ACSet.insert(dfaNodes[i]);
        }
        else {
            NotACSet.insert(dfaNodes[i]);
        }
    }
    if(!NotACSet.empty())
        newStart = new SimplifyDFANode(SimplifyDFACounter++);
    if(!ACSet.empty())
        newEnd = new SimplifyDFANode(SimplifyDFACounter++, true);
    cout << SimplifyDFACounter << endl;

    if (newStart != NULL)
        newStart->dfaNodes = NotACSet;
    if (newEnd != NULL)
        newEnd->dfaNodes = ACSet;

    if (newStart != NULL)
        SDFAQue.push(newStart);
    if (newEnd != NULL)
        SDFAQue.push(newEnd);

    for (auto op : operatorChar) {

        int size = SDFAQue.size();
        // 类似于层序遍历那样, 每个符号边筛一次
        for (int i = 0; i < size; i++) {
            SimplifyDFANode* dfanode = SDFAQue.front();
            SDFAQue.pop();
            vector<SimplifyDFANode*> nodes = SplitDFANode(dfanode, op);
            for (int j = 0; j < nodes.size(); j++) {
                SDFAQue.push(nodes[j]);
            }
        }
    }

    while (!SDFAQue.empty()) {
        SimplifyDFANode* sdfa = SDFAQue.front();
        SDFAQue.pop();
        SDFAMap.insert(make_pair(sdfa->id, sdfa));
        for (auto dfa : sdfa->dfaNodes) {
            dfa->SDFANum = sdfa->id;
        }
    }

    //for (auto sdfa : SDFAMap) {
    //    cout << sdfa.first << " 含有 : ";
    //    for (auto dfa : sdfa.second->dfaNodes) {
    //        cout << dfa->id << " ";
    //    }
    //    cout << endl;
    //}
    //cout << " ----------  上面是简化后的 DFA 每个结点所包含的化简前的结点 -----------" << endl;

}



void BuildSimplifyDFA() {
    for (auto sdfa : SDFAMap) {
        for (auto dfa : sdfa.second->dfaNodes) {
            for (auto nextNode : dfa->transitions) {
                char op = nextNode.first;
                int nextSdfa = nextNode.second->SDFANum;
                bool hasAdd = false;
                for (int i = 0; i < sdfa.second->transitions.size(); i++) {
                    // 检查这条边是否已经加入
                    if (sdfa.second->transitions[i].first == op && sdfa.second->transitions[i].second->id == nextSdfa)
                        hasAdd = true;
                }
                if (!hasAdd) {
                    sdfa.second->transitions.push_back(make_pair(op, SDFAMap[nextSdfa]));
                    SDFAG[sdfa.first].push_back(SDFAMap[nextSdfa]);
                }
            }
        }
    }

    for (int i = 0; i < SimplifyDFACounter; i++) {
        cout << i << " : ";
        for (auto pr : SDFAMap[i]->transitions) {
            cout  << pr.first << " " << pr.second->id<<" ";
        }
        cout << endl;
    }
    cout << " ----------  上面是简化后的 DFA -----------" << endl;
}


void DisplayNFA() {
    operatorCharTmp = operatorChar;
    operatorCharTmp.insert('#');
    int rowNum = stateCounter;
    int colNum = operatorCharTmp.size() + 2;
    for (auto nfa : NFAMap) {
        int num = nfa.first;
        nfaTable[num][1] = intToString(num);
        State* nfaNode = nfa.second;
        if (nfaNode->isStarting)
            nfaTable[num][0] = '-';
        if (nfaNode->isAccepting)
            nfaTable[num][0] = '+';


        int col = 2;
        for (auto op : operatorCharTmp) {
            for (int i = 0; i < nfaNode->transitions.size(); i++) {
                if (nfaNode->transitions[i].first == op) {
                    nfaTable[num][col] += intToString(nfaNode->transitions[i].second->id);
                    nfaTable[num][col] += ",";
                }
            }
            col++;
        }
    }

    for (int i = 0; i < rowNum; i++) {
        for (int j = 0; j < colNum; j++) {
            if (nfaTable[i][j] == "")
                nfaTable[i][j] = " ";
        }
    }

    cout << "     ";
    for (auto op : operatorCharTmp) {
        cout << op << " ";
    }
    cout << endl;
    for (int i = 0; i < rowNum; i++) {
        for (int j = 0; j < colNum; j++) {
            cout << nfaTable[i][j] << " ";
        }
        cout << endl;
    }
}

bool cmp(DFANode* node1, DFANode* node2) {
    return node1->id < node2->id;
}

string getAllNfaFromDFANode(DFANode* dfaNode) {
    string ans = "{ ";
    for (auto nfaNum : dfaNode->nfaNodes) {
        ans += intToString(nfaNum);
        ans += ",";
    }
    ans[ans.size() - 1] = '}';
    return ans;
}

void DisplayDFA() {
    cout << "------ 测试 ----------" << endl;
    int rowNum = DFANodeCounter;
    int colNum = operatorChar.size() + 2;
    sort(dfaNodes.begin(), dfaNodes.end(), cmp);
    for (int i = 0; i < dfaNodes.size(); i++) {
        DFANode* dfa = dfaNodes[i];
        cout << i << " : ";
        dfaTable[i][1] = getAllNfaFromDFANode(dfa);
        if (dfa->isAccepting)
            dfaTable[i][0] = '+';
        if (dfa->isStarting)
            dfaTable[i][0] = '-';
       int col = 2;
        for (auto op : operatorChar) {
            for (int j = 0; j < dfa->transitions.size(); j++) {
                if (dfa->transitions[j].first == op) {
                    cout << dfa->transitions[j].first <<dfa->transitions[j].second->id << " ";
                    dfaTable[i][col] = getAllNfaFromDFANode(dfa->transitions[j].second);
                    //break;

                }
            }
            col++;
        }
        cout << endl;
    }

    for (int i = 0; i < rowNum; i++) {
        for (int j = 0; j < colNum; j++) {
            if (nfaTable[i][j] == "")
                nfaTable[i][j] = "-----------";
        }
    }

    cout << "-------------";
    for (auto op : operatorChar) {
        cout << op << " ";
    }
    cout << endl;
    for (int i = 0; i < rowNum; i++) {
        for (int j = 0; j < colNum; j++) {
            cout << dfaTable[i][j] << " ";
        }
        cout << endl;
    }

}

void DisplaySimplifyDFA() {
    int rowNum = SimplifyDFACounter;
    int colNum = operatorChar.size() + 2;

    for (auto sdfa : SDFAMap) {
        int num = sdfa.first;
        sdfaTable[num][1] = intToString(num);
        SimplifyDFANode* sdfaNode = sdfa.second;
        if (sdfaNode->isAccepting)
            sdfaTable[num][0] = '+';
        if (sdfaNode->isStarting)
            sdfaTable[num][0] = '-';

        int col = 2;
        for (auto op : operatorChar) {
            for (int i = 0; i < sdfaNode->transitions.size(); i++) {
                if (sdfaNode->transitions[i].first == op) {
                    sdfaTable[num][col] += intToString (sdfaNode->transitions[i].second->id );
                    //break;

                }
            }
            col++;
        }
    }

    for (int i = 0; i < rowNum; i++) {
        for (int j = 0; j < colNum; j++) {
            if (sdfaTable[i][j] == "")
                sdfaTable[i][j] = " ";
        }
    }

    cout << "     ";
    for (auto op : operatorChar) {
        cout << op << " ";
    }
    cout << endl;
    for (int i = 0; i < rowNum; i++) {
        for (int j = 0; j < colNum; j++) {
            cout << sdfaTable[i][j] << " ";
        }
        cout << endl;
    }
}

bool vis[1000];

void getCode(int v,vector<string> &lines) {
    if (vis[v])
        return;
    vis[v] = true;
    //cout << v << endl;
    string done = "cout<<\"Done\"<<endl; return 0;";
    if (SDFAMap[v]->isAccepting)
        lines.push_back(done);
    vector<char> while_char, if_char;
    //首先收集结点v有多少条指向自己的边，收集该边的运算符
    for (auto edge : SDFAMap[v]->transitions) {
        if (edge.second->id == v)
            while_char.push_back(edge.first);
        else
            if_char.push_back(edge.first);
    }
    if (!while_char.empty()) {
        // 如果不为空，说明存在指向自身的边，这个时候就要生成while语句
        //生成的对应C语言分析程序，首句为“char ch=getChar()”
        lines.push_back("char ch = getchar();");
        string line = "while(";
        int i = 0;
        for (; i < while_char.size() - 1;) {
            line += "ch ==";
            string str;
            str += "'";
            str += while_char[i];
            str += "'";
            line += str + "||";
            i++;
        }
        string str;
        str += while_char[i];
        line += "ch == ";
        line += "'";
        line += str;
        line += "'";
        line += " )";
        lines.push_back(line);
        lines.push_back("{");
        if (SDFAMap[v]->isAccepting)
            lines.push_back(done);
        lines.push_back("ch = getchar();");
        lines.push_back("}");
        if (if_char.empty())
        {
            lines.push_back("cout<<\"error\"<<endl; return 0;");
        }

    }
    //处理完指向自身结点的边后，接下里处理指向别的结点的边
    if (!if_char.empty())
    {
        if (while_char.empty())
            lines.push_back("char ch = getchar();");

        if (!SDFAMap[v]->transitions.empty()) {
            for (auto edge : SDFAMap[v]->transitions) {
                if (edge.second->id != v)
                {
                    string line_1 = "if( ch == '";
                    string str_2;
                    str_2 += edge.first;
                    line_1 += str_2 + "' )";
                    lines.push_back(line_1);
                    lines.push_back("{");
                    getCode(edge.second->id, lines);
                    lines.push_back("}");
                    lines.push_back("else");
                }

            }
            string str_3;
            str_3 += 48 + v;
            string line_2;
            line_2 += "cout<<\"error(" + str_3 + ")\"<<endl; return 0;";
            lines.push_back(line_2);
        }
    }
}


void ShowCode() {
    for (int i = 0; i < SimplifyDFACounter; i++) {
        vis[i] = false;
    }

    int startNum = 0;
    for (auto sdfa : SDFAMap) {
        if (sdfa.second->isStarting) {
            startNum = sdfa.first;
            break;
        }
    }

    vector<string> lines;
    lines.push_back("#include<iostream>");
    lines.push_back("#include<cstdio>");
    lines.push_back("using namespace std;");
    lines.push_back("int main() {");
    getCode(startNum, lines);
    lines.push_back("return 0;");
    lines.push_back("}");
    ofstream out("Code.cpp");
    for (int i = 0; i < lines.size(); i++)
    {
        out << lines[i] << endl;
    }
    out.close();
}

// 生成C++代码，将代码写入到dfa.cpp文件中
void generateCode() {
    ofstream outputFile("Code.cpp");

    outputFile << "#include <iostream>\n"
        << "#include <cstdio>\n"
        << "using namespace std;\n\n"
        << "int main() {\n"
        << "int state = 0;\n"
        << "while (true) {\n"
        << "char ch = getchar();\n\n"
        << "switch (state) {\n";
    string tmp[1000];
    int startIndex = 0;
    for(int i=0;i<SimplifyDFACounter;i++) {
        if(sdfaTable[i][0] == "-") {
            startIndex = i;
            for(int j=0;j<operatorChar.size() + 2;j++) {
                tmp[j] = sdfaTable[i][j];
            }
        }
    }
    for(int i=startIndex-1;i>=0;i--) {
        for(int j=0;j<operatorChar.size() + 2;j++) {
            sdfaTable[i+1][j] = sdfaTable[i][j];
        }
    }
    for(int i=0;i<operatorChar.size() + 2;i++) {
        sdfaTable[0][i] = tmp[i];
    }

    cout << "     ";
    for (auto op : operatorChar) {
        cout << op << " ";
    }
    cout << endl;
    for (int i = 0; i < SimplifyDFACounter; i++) {
        for (int j = 0; j < operatorChar.size() + 2; j++) {
            cout << sdfaTable[i][j] << " ";
        }
        cout << endl;
    }

    for (int i = 0; i < SimplifyDFACounter; ++i) {
        int col = 2; int time = 0;
        outputFile << "case " << i << ":\n";//case 0:
        for (char op : operatorChar) {
            int nextState;
            for (int k = 0; k < SimplifyDFACounter; ++k) {
                if (sdfaTable[i][col] == sdfaTable[k][1]) {
                    if (time == 0) {

                        outputFile << "if (ch == '" << op << "') {\n"
                            << "state = " << k << ";\n"
                            << "}\n";

                        time++;
                    }
                    else {
                        outputFile << "else if (ch == '" << op << "') {\n"
                            << "state = " << k << ";\n"
                            << "}\n";
                        time++;
                    }
                }
            }
            col++;
        }
        if (sdfaTable[i][0] == "+" &&time != 0) {
            outputFile << "else if (ch == '\\n'){"
                << "cout << \"corrent expression!!!\" << endl;\n"
                << "return 0;\n"
                << "}\n";

        }
        if (sdfaTable[i][0] == "+" &&time == 0) {
            outputFile << "if (ch == '\\n'){"
                << "cout << \"corrent expression!!!\" << endl;\n"
                << "return 0;\n"
                << "}\n";

        }
        outputFile << "else {\n"
            << "cout << \"wrong expression!!!wrong state(\" << state << \")\" << endl;\n"
            << "return 0;\n"
            << "}\n"
            << "break;\n";


    }
    outputFile << "default:\n"
        << "cout << \"wrong expression!!!wrong state(\" << state << \")\" << endl;\n"
        << "return 0;\n"
        << "}\n"
        << "}\n\n"
        << "cout << \"wrong expression!!!wrong state(\" << state << \")\" << endl;\n"
        << "return 0;\n"
        << "}\n";
}

void calculateDegree() {
    for (int i = 1; i <= stateCounter; i++) {
        outDegree[i] += G[i].size();
        for (int j = 0; j < G[i].size(); j++) {
            inDegree[G[i][j]->id] ++;
        }
    }
}


void Parsing::on_pushButton_clicked()//开始分析
{

    stateCounter = 0;
    DFANodeCounter = 0;
    SimplifyDFACounter = 0;
    priorityMap.clear();
    operatorChar.clear();
    operatorCharTmp.clear();
    dfaNodes.clear();
    for(int i=0;i<1000;i++) {
        G[i].clear();
    }
    for(int i=0;i<1000;i++) {
        DFAG[i].clear();
    }
    for(int i=0;i<1000;i++) {
        SDFAG[i].clear();
    }
    for(int i=0;i<1000;i++) {
        outDegree[i] = 0;
        inDegree[i] = 0;
        vis[i] = 0;
    }

    NFAMap.clear();
    SDFAMap.clear();

    while(!SDFAQue.empty()) {
        SDFAQue.pop();
    }
    for(int i=0;i<1000;i++) {
        for(int j=0;j<1000;j++) {
            nfaTable[i][j] = "";
            dfaTable[i][j] = "";
            sdfaTable[i][j] = "";
        }
    }



    //清空完毕
    QFont font;
    font.setPointSize(30);
    ui->textBrowser->setFont(font);
    ui->textBrowser->show();
    ui->textBrowser->setText("分析成功，请继续下一步");
    ui->tableWidget->close();

    // 设置优先级
    priorityMap.insert(make_pair('?', 3));
    priorityMap.insert(make_pair('*',3));
    priorityMap.insert(make_pair('+',3));
    priorityMap.insert(make_pair('.', 2));
    priorityMap.insert(make_pair('|', 1));

    string regex;
    // cout << "输入正则表达式: ";
    //regex = ui->lineEdit->text().toStdString();
    QString text = ui->textEdit->toPlainText();
    regex = text.toStdString();

    for(int i=0;i<regex.size();i++) {
        if(regex[i] == '\n' && i != regex.size() - 1)
            regex[i] = '|';
    }

    for (int i = 0; i < regex.size() - 1; i++) {
        if (isWordChar(regex[i]) && isWordChar(regex[i + 1]) || isWordChar(regex[i]) && regex[i+1] == '('
            || regex[i] == ')' && isWordChar(regex[i + 1]) || regex[i] == '*' && regex[i + 1] != ')' && regex[i+1]!='|' && regex[i+1]!='?' || regex[i] == '?' && regex[i+1] != ')'
                || regex[i] == '+' && regex[i + 1] != ')')
        {
            string str1 = regex.substr(0, i + 1);
            string str2 = regex.substr(i + 1, (regex.size() - i));
            str1 += ".";
            regex = str1 + str2;
        }
    }

    NFA nfa = regexToNFA(regex);
    calculateDegree();
    NFAtoDFA();
    SimplifyDFA();
    BuildSimplifyDFA();

    cout << " ------------- 开始输出 NFA 表--------------" << endl;
    DisplayNFA();
    cout << " ------------- 开始输出 DFA 表--------------" << endl;
    DisplayDFA();
    cout << " ------------- 开始输出化简后的 DFA 表--------------" << endl;
    DisplaySimplifyDFA();
    //ShowCode();
    generateCode();
}

void Parsing::on_pushButton_2_clicked()//NFA
{
    ui->textBrowser->close();
    ui->tableWidget->show();
    int rowNum = stateCounter;
    int colNum = operatorCharTmp.size() + 3;
    ui->tableWidget->setRowCount(rowNum + 1);
    ui->tableWidget->setColumnCount(colNum);
    QFont Font;
    Font.setPointSize(15);
    ui->tableWidget->setFont(Font);
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem("起，终点"));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem("点\\边"));
    int temp = 2;
    for (auto op : operatorCharTmp) {
        ui->tableWidget->setItem(0, temp, new QTableWidgetItem(QString(op)));
        temp++;
    }
    for (int i = 2; i < rowNum + 2; i++) {
        for (int j = 0; j < colNum; j++) {
            ui->tableWidget->setItem(i - 1, j, new QTableWidgetItem(QString::fromStdString(nfaTable[i-2][j])));
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

void Parsing::on_pushButton_3_clicked()//DFA
{
    ui->textBrowser->close();
    ui->tableWidget->show();
    int rowNum = DFANodeCounter;
    int colNum = operatorChar.size() + 2;
    ui->tableWidget->setRowCount(rowNum + 1);
    ui->tableWidget->setColumnCount(colNum);
    QFont Font;
    Font.setPointSize(15);
    ui->tableWidget->setFont(Font);
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem("起，终点"));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem("点\\边"));
    int temp = 2;
    for (auto op : operatorChar) {
        ui->tableWidget->setItem(0, temp, new QTableWidgetItem(QString(op)));
        temp++;
    }
    for (int i = 2; i < rowNum + 2; i++) {
        for (int j = 0; j < colNum; j++) {
            ui->tableWidget->setItem(i - 1, j, new QTableWidgetItem(QString::fromStdString(dfaTable[i-2][j])));
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

void Parsing::on_pushButton_4_clicked()//最小化NFA
{

    ui->textBrowser->close();
    ui->tableWidget->show();
    int rowNum = SimplifyDFACounter;
    int colNum = operatorChar.size() + 2;
    ui->tableWidget->setRowCount(rowNum + 1);
    ui->tableWidget->setColumnCount(colNum);
    QFont Font;
    Font.setPointSize(15);
    ui->tableWidget->setFont(Font);
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem("起，终点"));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem("点\\边"));
    int temp = 2;
    for (auto op : operatorChar) {
        ui->tableWidget->setItem(0, temp, new QTableWidgetItem(QString(op)));
        temp++;
    }
    for (int i = 2; i < rowNum + 2; i++) {
        for (int j = 0; j < colNum; j++) {
            ui->tableWidget->setItem(i - 1, j, new QTableWidgetItem(QString::fromStdString(sdfaTable[i-2][j])));
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

void Parsing::on_pushButton_5_clicked()
{
    QFont font;
    font.setPointSize(15);
    ui->textBrowser->setFont(font);
    ui->textBrowser->show();
    ui->tableWidget->close();
    QFile file("Code.cpp");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&file);
    ui->textBrowser->setText(in.readAll());

    file.close();
}

void Parsing::on_pushButton_6_clicked()
{
    QString text = ui->textEdit->toPlainText();
    string regex = text.toStdString();
    // 打开一个文件流
    ofstream outputFile("regex.txt");

    // 检查文件是否成功打开
    if (outputFile.is_open()) {
        // 写入字符串到文件
        outputFile << regex;

        // 关闭文件流
        outputFile.close();
    }
}

void Parsing::on_pushButton_7_clicked()
{
    // 打开文件选择对话框以获取用户选择的文件
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", QDir::homePath(), "文本文件 (*.txt);;所有文件 (*.*)");

    // 检查用户是否取消了文件选择
    if (filePath.isEmpty()) {
        return;
    }

    // 打开选择的文件
    QFile file(filePath);

    // 检查文件是否成功打开
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件");
        return;
    }

    // 创建文本流来读取文件内容
    QTextStream in(&file);

    // 读取文件中的文本到一个QString
    QString fileContents = in.readAll();

    // 关闭文件
    file.close();

    // 在这里，fileContents 包含了文件的文本内容，你可以对它进行操作，例如显示在UI上或者进行其他处理
    // 例如，将文本内容显示在一个文本编辑框中：
    ui->textEdit->setPlainText(fileContents);
}
