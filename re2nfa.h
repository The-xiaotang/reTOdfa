#ifndef RE2NFA_H
#define RE2NFA_H

#include"process.h"
// 正则转NFA
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <map>
#include <vector>
#include <stack>
#include <unordered_map>
#include <set>
#include <string>
#include <QFile>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include <iostream>
using namespace std;

struct nfaNode;

// NFA图的边
struct nfaEdge
{
    char c;
    nfaNode* next;
};

// NFA图的结点
struct nfaNode
{
    vector<nfaEdge> edges;  // 一个结点可从vector中选择某条边转移状态
    int id; // 结点唯一编号
    bool isStart;   // 初态标识
    bool isEnd; // 终态标识
    nfaNode() {
        id = nodeCount++;
        isStart = false;
        isEnd = false;
    }
};

// NFA图
struct NFA
{
    // 只需起始结点和终止结点就能标识一个NFA图
    nfaNode* start;
    nfaNode* end;
    NFA() {}
    NFA(nfaNode* s, nfaNode* e)
    {
        start = s;
        end = e;
    }
};

/*
// 基本字符NFA
NFA basicCharNFA(char character) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge;
    edge.c = character;
    edge.next = end;
    start->edges.push_back(edge);

    NFA nfa(start, end);

    nfaCharSet.insert(character);
    dfaCharSet.insert(character);

    return nfa;
}
*/

// 修改 basicCharNFA 函数，处理字符类符号
NFA basicCharNFA(char character) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge;
    edge.c = character;
    edge.next = end;
    start->edges.push_back(edge);

    NFA nfa(start, end);

    nfaCharSet.insert(character);
    dfaCharSet.insert(character);

    return nfa;
}

// 连接运算符的NFA图
NFA concatNFA(NFA nfa1, NFA nfa2) {
    // 把nfa1的终止状态与nfa2的起始状态连接起来
    nfa1.end->isEnd = false;
    nfa2.start->isStart = false;

    nfaEdge edge;
    edge.c = EPSILON;
    edge.next = nfa2.start;
    nfa1.end->edges.push_back(edge);

    NFA nfa;
    nfa.start = nfa1.start;
    nfa.end = nfa2.end;

    return nfa;
}

// 选择运算符的NFA图
NFA orNFA(NFA nfa1, NFA nfa2) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    // 把新的初态与nfa1和nfa2的初态连接起来
    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;    // 将nfa1的原初态修改为普通状态

    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = nfa2.start;
    start->edges.push_back(edge2);
    nfa2.start->isStart = false;    // 将nfa2的原初态修改为普通状态

    // 把nfa1和nfa2的终止状态与新的终止状态连接起来
    nfa1.end->isEnd = false;
    nfa2.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = end;
    nfa1.end->edges.push_back(edge3);

    nfaEdge edge4;
    edge4.c = EPSILON;
    edge4.next = end;
    nfa2.end->edges.push_back(edge4);

    NFA nfa(start, end);

    return nfa;
}

// 闭包运算符的NFA图
NFA closureNFA(NFA nfa1) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    // 令nfa1的新初态指向nfa1的原初态
    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;    // 将原初态修改为普通状态

    // 令nfa1的新初态指向nfa1的新终态
    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = end;
    start->edges.push_back(edge2);

    // 令nfa1的原终止状态指向nfa1的原初始状态
    nfa1.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = nfa1.start;
    nfa1.end->edges.push_back(edge3);

    // 令nfa1的原终态指向nfa1的新终态
    nfaEdge edge4;
    edge4.c = EPSILON;
    edge4.next = end;
    nfa1.end->edges.push_back(edge4);

    NFA nfa(start,end);

    return nfa;
}

// 可选运算符的NFA图
// ?的NFA图其实就是无需自环的闭包NFA图
NFA optionalNFA(NFA nfa1) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    // 令nfa1的新初态指向nfa1原初态
    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;    // 初态结束

    // 令nfa1的新初态指向nfa1新终态
    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = end;
    start->edges.push_back(edge2);

    // 令nfa1的原终态指向nfa1原终态
    nfa1.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = end;
    nfa1.end->edges.push_back(edge3);

    NFA nfa(start, end);

    return nfa;
}

// 优先级判断
int priority(char op) {
    if (op == '|') {
        return 1;  // 选择运算符 "|" 的优先级
    }
    else if (op == '.') {
        return 2;  // 连接运算符 "." 的优先级
    }
    else if (op == '*' || op == '?') {
        return 3;  // 闭包运算符 "*"和 "?" 的优先级
    }
    else {
        return 0;  // 默认情况下，将其它字符的优先级设为0
    }
}

struct nfaStatusNode
{
    string flag;  // 标记初态还是终态
    int id; // 唯一id值  nfa状态结点的id对应的是nfa结点的id
    map<char, set<int>> transition;  // 对应字符能到达的结点集合
    nfaStatusNode()
    {
        flag = "";
    }
};

// 状态转换表
// nfa结点id映射至nfa状态结点
unordered_map<int, nfaStatusNode> statusTable;    // nfaStatusNode类型的数组
// insertionOrder记录插入statusTable的顺序
vector<int> insertionOrder;
set<int> startNFAstatus;
set<int> endNFAstatus;

// 对NFA图进行DFS的过程中，形成状态转换表

void formNFAStatus(NFA& nfa)
{
    stack<nfaNode*> nfaStack;
    set<nfaNode*> visitedNodes;

    // 初态
    // 从初态出发
    nfaNode* startNode = nfa.start;
    nfaStatusNode startStatusNode;
    startStatusNode.flag = '-'; // -表示初态
    startStatusNode.id = startNode->id;
    statusTable[startNode->id] = startStatusNode;
    insertionOrder.push_back(startNode->id);
    startNFAstatus.insert(startNode->id);

    nfaStack.push(startNode);

    while (!nfaStack.empty()) {
        nfaNode* currentNode = nfaStack.top();
        nfaStack.pop();
        visitedNodes.insert(currentNode);

        for (nfaEdge edge : currentNode->edges) {
            char transitionChar = edge.c;
            nfaNode* nextNode = edge.next;

            // 记录状态转换信息 当前状态结点经过transitionChar能到达它对应的结点指向的下一个结点，将这个“下一个结点”的id记录在当前状态结点的状态转换集合（set<int>）中
            statusTable[currentNode->id].transition[transitionChar].insert(nextNode->id);

            // 如果下一个状态未被访问，将其加入堆栈
            if (visitedNodes.find(nextNode) == visitedNodes.end()) {
                nfaStack.push(nextNode);

                // 记录状态信息
                // 为记录状态转换信息（statusTable[currentNode->id]）做铺垫
                nfaStatusNode nextStatus;
                nextStatus.id = nextNode->id;
                if (nextNode->isStart) {
                    nextStatus.flag += '-'; // -表示初态
                    startNFAstatus.insert(nextStatus.id);
                }
                else if (nextNode->isEnd) {
                    nextStatus.flag += '+'; // +表示终态
                    endNFAstatus.insert(nextStatus.id);
                }
                statusTable[nextNode->id] = nextStatus;
                // 确保终态在插入顺序表的最后一位
                if (!nextNode->isEnd)
                {
                    insertionOrder.push_back(nextNode->id);
                }
            }
        }
    }

    // 确保终态在插入顺序表的最后一位
    nfaNode* endNode = nfa.end;
    insertionOrder.push_back(endNode->id);
}


// 正则表达式转NFA
NFA regexToNFA(string regex)
{
    // 双栈法，创建两个栈opStack（运算符栈）,nfaStack（nfa图栈）
    stack<char> opStack;
    stack<NFA> nfaStack;

    int i = -1;
    // 对表达式进行后缀表达式处理
    // 运算符：| .（） *
    // 由于闭包*、?是单目运算符，闭包运算符与其前一个“表达式”为一个整体，与连接和或运算符不同，因此闭包运算符无需入栈
    for (char c : regex)
    {
        switch (c)
        {
        case ' ':
            i++;
            cout << "i1 = " << i << endl;
            break;
        case '(':
            i++;
            cout << "i2 = " << i << endl;
            opStack.push(c);
            break;
        case ')':
            // 扫描正则表达式，遇到")"则依次弹出栈内运算符，直到弹出一个“(”为止。
            i++;
            cout << "i3 = " << i << endl;
            while (!opStack.empty() && opStack.top() != '(')
            {
                // 处理栈顶运算符，构建NFA图，并将结果入栈
                char op = opStack.top();
                opStack.pop();

                if (op == '|') {
                    // 处理并构建"|"运算符
                    NFA nfa2 = nfaStack.top();      // “右”操作数
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();      // “左”操作数
                    nfaStack.pop();

                    // 创建新的NFA，表示nfa1 | nfa2
                    NFA resultNFA = orNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
                else if (op == '.') {
                    // 处理并构建"."运算符
                    NFA nfa2 = nfaStack.top();      // “右”操作数
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();      // “左”操作数
                    nfaStack.pop();

                    // 创建新的NFA，表示nfa1 . nfa2
                    NFA resultNFA = concatNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
            }
            if (opStack.empty())
            {
                qDebug() << "括号未闭合！";
                exit(-1);
            }
            else
            {
                opStack.pop(); // 弹出(
            }
            break;
        case '|':
        case '.':
            // 处理运算符 | 和 .
            i++;
            cout << "i4 = " << i << endl;
            // _test=\*|\?|\||\+|a|>=
            //            if(i - 1 == 0 && regex[i - 1] == '.' || (regex[i - 2] == '|' && regex[i - 1] == '.')){
            //                opStack.pop();      // 弹出“.”
            //                NFA nfa = basicCharNFA(c); // 创建基本字符 \| NFA
            //                nfaStack.push(nfa);
            //            }
            //            if(regex[i] == '|' && regex[i - 1] == '.'){
            //                cout << "! _test=\+|\.|\* !" << endl;
            //                NFA nfa = basicCharNFA(c); // 创建基本的字符 \| 的NFA
            //                nfaStack.push(nfa);
            //            }
            //            else{
            //            }
            while (!opStack.empty() && (opStack.top() == '|' || opStack.top() == '.') &&
                   priority(opStack.top())>= priority(c))
            // 遇到运算符，依次弹出栈中优先级高于或等于当前扫描到的运算符的所有运算符，与“操作数”（即nfa图）运算
            // 然后将当前运算符压入栈中
            {
                char op = opStack.top();
                opStack.pop();

                // 处理栈顶运算符，构建NFA图，并将结果入栈
                if (op == '|') {
                    // 处理并构建"|"运算符
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    // 创建新的NFA，表示nfa1 | nfa2
                    NFA resultNFA = orNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
                else if (op == '.') {
                    // 处理并构建"."运算符
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    // 创建新的 NFA，表示 nfa1 . nfa2
                    NFA resultNFA = concatNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
            }
            opStack.push(c);
            break;
        case '?':
        case '*':
            i++;
            cout << "i5 = " << i << endl;
            // 处理闭包运算符 ? *
            // 从nfaStack弹出NFA，应用相应的闭包操作，并将结果入栈
            if (!nfaStack.empty() || regex[i - 1] == '.') {
                if (c == '?') {
                    // 处理前面没有基本字符的“\?”
                    if((i - 1 == 0 && regex[i - 1] == '.') || (regex[i - 2] == '|')){
                        cout << "opStack.top()=" << opStack.top() << endl;
                        opStack.pop();      // 弹出“.”
                        NFA nfa = basicCharNFA(c); // 创建基本字符 \? NFA
                        nfaStack.push(nfa);
                        // _test=\?
                    }
                    // 处理 a\?
                    else if(regex[i - 1] == '.'){
                        cout << "here?" << endl;
                        // _test=a\*
                        NFA nfa = basicCharNFA(c); // 创建基本字符 \? NFA
                        nfaStack.push(nfa);
                        cout << "nfaStack.size()" << nfaStack.size() << endl;
                    }
                    // 处理 ?
                    else{
                        NFA nfa = nfaStack.top();
                        nfaStack.pop();
                        NFA resultNFA = optionalNFA(nfa);
                        nfaStack.push(resultNFA);
                    }
                }
                else if (c == '*') {
                    // 处理前面没有基本字符的“\*”
                    if((i - 1 == 0 && regex[i - 1] == '.') || (regex[i - 2] == '|')){
                        cout << "opStack.top()=" << opStack.top() << endl;
                        opStack.pop();      // 弹出“.”
                        NFA nfa = basicCharNFA(c); // 创建基本的字符NFA
                        nfaStack.push(nfa);
                        // _test=\*
                    }
                    // 处理 a\*
                    else if(regex[i - 1] == '.'){
                        cout << "here*" << endl;
                        // _test=a\*
                        NFA nfa = basicCharNFA(c); // 创建基本的字符NFA
                        nfaStack.push(nfa);
                        cout << "nfaStack.size()" << nfaStack.size() << endl;
                    }
                    // 处理*
                    else{
                        NFA nfa = nfaStack.top();
                        nfaStack.pop();
                        NFA resultNFA = closureNFA(nfa);
                        nfaStack.push(resultNFA);
                    }
                }
            }
            else {
                qDebug() << "错误1";
                exit(-1);
            }
            break;
        default:
            i++;
            cout << "i6 = " << i << endl;
            // 处理其他字符
            NFA nfa = basicCharNFA(c); // 创建基本的字符NFA
            nfaStack.push(nfa);
            cout << "nfaStack.size()" << nfaStack.size() << endl;
            break;
        }

    }

    // 处理栈中剩余的运算符
    while (!opStack.empty())
    {
        char op = opStack.top();
        opStack.pop();

        if (op == '|' || op == '.')
        {
            // 处理并构建运算符 | 和 .
            if (nfaStack.size() < 2)
            {
                qDebug() << "错误2";
                exit(-1);
            }

            NFA nfa2 = nfaStack.top();
            nfaStack.pop();
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();

            if (op == '|')
            {
                // 创建新的 NFA，表示 nfa1 | nfa2
                NFA resultNFA = orNFA(nfa1, nfa2);
                nfaStack.push(resultNFA);
            }
            else if (op == '.')
            {
                // 创建新的 NFA，表示 nfa1 . nfa2
                NFA resultNFA = concatNFA(nfa1, nfa2);
                nfaStack.push(resultNFA);
            }
        }
        else
        {
            qDebug() << "错误3";
            exit(-1);
        }
    }

    // 最终的NFA图在nfaStack的顶部
    NFA result = nfaStack.top();

    formNFAStatus(result);

    return result;
}
#endif // RE2NFA_H
