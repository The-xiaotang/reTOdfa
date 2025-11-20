#ifndef NFA2DFA_H
#define NFA2DFA_H

#include"re2nfa.h"

// NFA转DFA

// dfa节点
struct dfaNode
{
    string flag; // 是否包含终态（+）或初态（-）
    set<int> nfaStates; // 该DFA状态包含的NFA状态的集合
    map<char, set<int>> transitions; // 字符到下一状态的映射
    dfaNode() {
        flag = "";
    }
};
// dfa状态去重集
set<set<int>> dfaStatusSet;

// dfa最终结果
vector<dfaNode> dfaTable;

//下面用于DFA最小化
// dfa有且仅有一个初态，因此不需要初态集合
// dfa终态集合
set<int> dfaEndStatusSet;
// dfa非终态集合
set<int> dfaNotEndStatusSet;
// set对应序号MAP
map<set<int>, int> DFAToID;
int startStaus;

// 判断是否含有初态终态，含有则返回对应字符串
// 查看DFA的状态集合是否包含NFA中的初态或终态
// 若包含初态，则当前DFA状态为初态（-）；若包含终态，则当前DFA状态为终态（+）；
// 若都包含，则当前DFA状态同时为初态和终态（-+）；若都不包含，则当前DFA状态为普通状态（空字符串）
string start_end(set<int>& statusSet)
{
    string result = "";
    for (const int& element : startNFAstatus) {
        if (statusSet.count(element) > 0) {
            result += "-";
        }
    }

    for (const int& element : endNFAstatus) {
        if (statusSet.count(element) > 0) {
            result += "+";
        }
    }

    return result;
}

set<int> epsilonClosure(int id)
{
    set<int> eResult{ id };     // 结点本身也属于epsilon闭包
    stack<int> stack;
    stack.push(id);

    while (!stack.empty())
    {
        int current = stack.top();
        stack.pop();

        set<int> eClosure = statusTable[current].transition[EPSILON];    // nfa中current结点的id仅通过一次epsilon能够到达的结点的id集合
        for (auto t : eClosure)
        {
            // 防止遇到值全是epsilon的边组成的圈而导致陷入死循环
            if (eResult.find(t) == eResult.end())
            {
                eResult.insert(t);
                stack.push(t);          // 通过epsilon到达当前结点后，还需查看当前结点是否能通过epsilon到达其它结点
            }
        }
    }

    return eResult;     // epsilon闭包
}

// 转换闭包
set<int> transitionClosure(int source, char ch)
{
    set<int> result{};

    set<int> chClosure = statusTable[source].transition[ch];
    for (auto o : chClosure)
    {
        result.insert(o);
        auto tmp = epsilonClosure(o);
        result.insert(tmp.begin(), tmp.end());     // 当前结点的字符闭包就是当前结点通过字符指向的下一个结点加上“下一个结点”的esilon闭包
    }

    return result;
}


void NFAToDFA(NFA& nfa)
{
    // 获取DFA的起始结点（nfa初态的epsilon闭包） 将DFA的起始节点（set类型）映射成整型编号（如{1, 2, 6}用1代替） 判断起始节点的状态
    int dfaStatusCount = 1;
    auto start = nfa.start; // 获得NFA图的起始位置
    auto startId = start->id;   // 获得起始编号
    dfaNode startDFANode;
    startDFANode.nfaStates = epsilonClosure(startId); // 初始闭包
    startDFANode.flag = start_end(startDFANode.nfaStates); // 判断初态终态
    deque<set<int>> newStatus{};
    DFAToID[startDFANode.nfaStates] = dfaStatusCount;
    startStaus = dfaStatusCount;
    // 查看startDFANode.flag是否包含"+" (查看startDFANode是否是终态)
    if (start_end(startDFANode.nfaStates).find("+") != string::npos) {
        // 若包含（若是）
        dfaEndStatusSet.insert(dfaStatusCount++);
    }
    else
    {
        // 若不包含（若不是）
        dfaNotEndStatusSet.insert(dfaStatusCount++);
    }
    // 对dfa初始状态中的每个nfa状态进行对应字符的状态转换
    // {1, 2, 6}_a -> {3, 4, 7, 8}   {1, 2, 6}_b -> error
    for (auto ch : dfaCharSet)
    {
        set<int> currentChClosure{};
        for (auto c : startDFANode.nfaStates)
        {
            set<int> tmp = transitionClosure(c, ch);
            currentChClosure.insert(tmp.begin(), tmp.end());   // 状态集合中各个字符的转移闭包
        }
        if (currentChClosure.empty())  // 如果这个闭包是空集没必要继续下去了 因此可能会有些字符不会记录在transitions中
        {
            continue;
        }
        // 假设{1, 2, 6}_a -> {3, 4, 7, 8}   {1, 2, 6}_b -> {3, 4, 7, 8} 即{1， 2， 6}对a和b都转移到统一集合
        // 遍历到b字符时无需再对{3, 4, 7, 8}进行状态标识，也无需再将它映射成整型编号，也无需再将其压入队列中进行后序遍历，因为这些都在遍历到a字符时完成
        int presize = dfaStatusSet.size();
        dfaStatusSet.insert(currentChClosure);
        int postsize = dfaStatusSet.size();
        // 无论是否一样都是该节点这个字符的状态
        startDFANode.transitions[ch] = currentChClosure;
        // 如果大小不一样，说明集合并未去重，新添的集合元素与原有的元素不重复，证明是新状态
        if (postsize > presize)
        {
            DFAToID[currentChClosure] = dfaStatusCount;
            newStatus.push_back(currentChClosure);
            if (start_end(currentChClosure).find("+") != string::npos) {
                dfaEndStatusSet.insert(dfaStatusCount++);
            }
            else
            {
                dfaNotEndStatusSet.insert(dfaStatusCount++);
            }

        }

    }
    dfaTable.push_back(startDFANode);

    // 对后面的dfa新状态进行不停遍历
    while (!newStatus.empty())
    {
        // 拿出一个新状态
        set<int> newStat = newStatus.front();
        newStatus.pop_front();
        dfaNode DFANode;
        DFANode.nfaStates = newStat;  // 该节点状态集合
        DFANode.flag = start_end(newStat);

        for (auto ch : dfaCharSet)
        {

            set<int> currentChClosure{};
            for (auto c : newStat)
            {
                set<int> tmp = transitionClosure(c, ch);
                currentChClosure.insert(tmp.begin(), tmp.end());
            }
            if (currentChClosure.empty())  // 如果这个闭包是空集没必要继续下去了
            {
                continue;
            }
            int presize = dfaStatusSet.size();
            dfaStatusSet.insert(currentChClosure);
            int lastsize = dfaStatusSet.size();
            // 无论是否一样都是该节点这个字符的状态
            DFANode.transitions[ch] = currentChClosure;
            // 如果大小不一样，说明集合并未去重，新添的集合元素与原有的元素不重复，证明是新状态
            if (lastsize > presize)
            {
                DFAToID[currentChClosure] = dfaStatusCount;
                newStatus.push_back(currentChClosure);
                if (start_end(currentChClosure).find("+") != string::npos) {
                    dfaEndStatusSet.insert(dfaStatusCount++);
                }
                else
                {
                    dfaNotEndStatusSet.insert(dfaStatusCount++);
                }

            }

        }
        dfaTable.push_back(DFANode);

    }

}


#endif // NFA2DFA_H
