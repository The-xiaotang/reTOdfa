#ifndef DFA2MINDFA_H
#define DFA2MINDFA_H

#include"nfa2dfa.h"

// DFA最小化

// 判断是否含有初态终态，含有则返回对应字符串
string min_set_end(set<int>& statusSet)
{
    string result = "";
    if (statusSet.count(startStaus) > 0) {
        result += "-";
    }

    for (const int& element : dfaEndStatusSet) {
        if (statusSet.count(element) > 0) {
            result += "+";
            break;  // 若包含多个终态也只需要一个加号
        }
    }

    return result;
}

// dfa最小化节点
struct dfaMinNode
{
    string flag; // 是否包含终态（+）或初态（-）
    int id;
    map<char, int> transitions; // 字符到下一状态的映射
    dfaMinNode() {
        flag = "";
    }
};

vector<dfaMinNode> dfaMinVector;

// 用于分割集合
vector<set<int>> dividedSetVec;

map<int, int> dfaSameMinSet;    // DFA的dfaStatusCount只要被划分到同一个集合就会被映射成同样的值，值依赖于dividedSetVec大小

// 根据字符 ch 将状态集合 node 分成两个子集合
void divideSet(int i, char ch)
{
    set<int> result;
    auto& node = dividedSetVec[i];   // node是dfaStatusCount，而dfaStatusCount从1开始
    int s = -2;

    for (auto state : node)
    {
        int sameSetMark;
        // 当前DFA结点无法通过ch转移状态
        if (dfaTable[state - 1].transitions.find(ch) == dfaTable[state - 1].transitions.end())
        {
            sameSetMark = -1;
        }
        else
        {
            // 根据字符 ch 找到下一个状态
            int next_state = DFAToID[dfaTable[state - 1].transitions[ch]];//dfaStatusCount
            sameSetMark = dfaSameMinSet[next_state];    //
        }

        if (s == -2)    // 初始下标
        {
            s = sameSetMark;
        }
        else if (sameSetMark != s)   // 如果下标不同，就是有问题，需要分出来
        {
            result.insert(state);
        }
    }

    // 删除要删除的元素
    for (int state : result) {
        node.erase(state);
    }

    // 都遍历完了，如果result不是空，证明有新的，加入vector中
    if (!result.empty())
    {
        dividedSetVec.push_back(result);
        // 同时更新映射值
        for (auto a : result)
        {
            dfaSameMinSet[a] = dividedSetVec.size() - 1;
        }
    }

}

void DFAminimize()
{
    dividedSetVec.clear();
    dfaSameMinSet.clear();

    // 存入非终态集合
    if (dfaNotEndStatusSet.size() != 0)
    {
        dividedSetVec.push_back(dfaNotEndStatusSet);
    }
    // 初始化map
    for (auto t : dfaNotEndStatusSet)
    {
        dfaSameMinSet[t] = dividedSetVec.size() - 1;     // 0 或 -1
    }

    // 存入终态集合
    dividedSetVec.push_back(dfaEndStatusSet);

    // 初始化map
    for (auto t : dfaEndStatusSet)
    {
        dfaSameMinSet[t] = dividedSetVec.size() - 1;     // 1 或 0
    }

    // 当flag为1时，一直循环
    int continueFlag = 1;

    while (continueFlag)
    {
        continueFlag = 0;
        int size1 = dividedSetVec.size();

        for (int i = 0; i < size1; i++)
        {

            // 逐个字符尝试分割状态集合
            for (char ch : dfaCharSet)
            {
                divideSet(i, ch);
            }
        }
        int size2 = dividedSetVec.size();
        if (size2 > size1)      // 如果size2 > size1 不成立，说明已经无法再划分出新的集合（size2 = size1）；否则还有划分的可能
        {
            continueFlag = 1;
        }
    }

    for (int dfaMinCount = 0; dfaMinCount < dividedSetVec.size(); dfaMinCount++)
    {
        auto& v = dividedSetVec[dfaMinCount];
        dfaMinNode d;
        d.flag = min_set_end(v);
        // 为最小化后得到的结点附上id
        d.id = dfaMinCount;
        // 逐个字符
        for (char ch : dfaCharSet)
        {
            if (v.size() == 0)
            {
                d.transitions[ch] = -1;   // 空集特殊判断
                continue;
            }
            int i = *(v.begin());
            if (dfaTable[i - 1].transitions.find(ch) == dfaTable[i - 1].transitions.end())
            {
                d.transitions[ch] = -1;   // 空集特殊判断
                continue;
            }
            int next_state = DFAToID[dfaTable[i - 1].transitions[ch]];
            int sameSetMark = dfaSameMinSet[next_state];
            d.transitions[ch] = sameSetMark;
        }
        dfaMinVector.push_back(d);
    }
}


#endif // DFA2MINDFA_H
