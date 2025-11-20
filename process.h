#ifndef PROCESS_H
#define PROCESS_H


#include <QTextStream>
#include <QDebug>
#include <QString>
#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <sstream>
#include <QFile>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include <iostream>
using namespace std;

// nfa图的结点数量，用于为nfa结点附上id值
int nodeCount = 0;

// 空字符串用“#”表示
const char EPSILON = '#';

// 全局字符统计
set<char> nfaCharSet;
set<char> dfaCharSet;

// 赋值映射 将命名语句中的变量名映射成正则表达式
unordered_map<string, string> name;

unordered_map<string, char> charClassToSymbol;  // 字符类到符号的映射
unordered_map<char, string> symbolToCharClass;  // 符号到字符类的映射
char currentSymbol = 'A';  // 从'A'开始分配符号


unordered_map<string, char> specialCharToSymbol;  // 特殊字符到符号的映射
unordered_map<char, string> symbolToSpecialChar;  // 符号到特殊字符的映射
char currentSpecialSymbol = 'W';  // 从'W'开始分配特殊符号

// 将多行正则表达式合并成一行
/*
    line1
    line2    =>    (line1)|(line2)|(line3)
    line3
*/

/*
QString merge(QString regex)
{
    string stdRegex = regex.toStdString();
    vector<string> lines;
    vector<string> resVec;
    istringstream iss(stdRegex);
    string line;
    string output;

    while (std::getline(iss, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    for (int i = 0; i < lines.size(); ++i) {
        size_t assignPos = lines[i].find('=');
        // 命名
        if(assignPos != string::npos && lines[i][assignPos-1] != 'B'){
            string variable = lines[i].substr(0, assignPos);
            string value = lines[i].substr(assignPos + 1);
            name[variable] = value;
        }
        // 将等号右边的正则表达式分离出来
        else if(assignPos != string::npos && lines[i][assignPos-1] == 'B'){//else if(assignPos != string::npos && lines[i][0] == '_'){
            string exp = lines[i].substr(assignPos + 1);
            cout << exp << endl;
            string result = exp;

            // 遍历映射表的键值对，替换字符串中的对应子串
            for (auto& mapping : name) {
                size_t pos = 0;
                string replacement = "(" + mapping.second + ")";       // 为每个映射值添加括号
                // 查找并替换所有匹配的键
                while ((pos = result.find(mapping.first, pos)) != std::string::npos) {
                    result.replace(pos, mapping.first.length(), replacement);
                    pos += mapping.second.length(); // 跳过替换后的部分，防止重复替换
                }
            }
            cout << result << endl;
            resVec.push_back(result);
        }
    }

    for(size_t i = 0; i < resVec.size(); i++)
    {
        output += resVec[i];
        cout << "test" << endl;
        if(i < resVec.size() - 1){
            output += "|";
        }
    }

    //    for(auto &it : assign){
    //        cout << it.first << it.second << endl;
    //    }
    return QString::fromStdString(output);
}
*/

QString merge(QString regex)
{
    string stdRegex = regex.toStdString();
    vector<string> lines;
    vector<string> resVec;
    istringstream iss(stdRegex);
    string line;
    string output;

    // 清空之前的映射，确保每次运行都是独立的
    charClassToSymbol.clear();
    symbolToCharClass.clear();
    specialCharToSymbol.clear();
    symbolToSpecialChar.clear();
    name.clear();
    currentSymbol = 'A';
    currentSpecialSymbol = 'W';

    // 读取所有行
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    // 第一遍：处理所有定义（字符类和特殊字符）
    for (int i = 0; i < lines.size(); ++i) {
        size_t assignPos = lines[i].find('=');
        if (assignPos == string::npos) continue;

        string variable = lines[i].substr(0, assignPos);
        string value = lines[i].substr(assignPos + 1);

        // 去除变量名和值的首尾空格
        variable.erase(0, variable.find_first_not_of(' '));
        variable.erase(variable.find_last_not_of(' ') + 1);
        value.erase(0, value.find_first_not_of(' '));
        value.erase(value.find_last_not_of(' ') + 1);

        // 记录所有命名
        name[variable] = value;

        // 命名字符类定义（如 digit=[0-9]）
        if (value.find('[') != string::npos && value.find(']') != string::npos) {
            // 为字符类分配符号
            if (charClassToSymbol.find(variable) == charClassToSymbol.end()) {
                charClassToSymbol[variable] = currentSymbol;
                symbolToCharClass[currentSymbol] = variable;
                cout << "分配字符类符号: " << variable << " -> " << currentSymbol << endl;
                currentSymbol++;
            }
        }

        // 扫描值中的转义字符，并为它们分配符号
        vector<string> escapeSequences = {"\\+", "\\*", "\\?", "\\|"};
        for (const auto& esc : escapeSequences) {
            size_t pos = 0;
            while ((pos = value.find(esc, pos)) != string::npos) {
                // 如果这个转义序列还没有分配符号，就分配一个
                if (specialCharToSymbol.find(esc) == specialCharToSymbol.end()) {
                    specialCharToSymbol[esc] = currentSpecialSymbol;
                    symbolToSpecialChar[currentSpecialSymbol] = esc;
                    cout << "分配特殊字符符号: " << esc << " -> " << currentSpecialSymbol << endl;
                    currentSpecialSymbol++;
                }
                pos += esc.length(); // 移动位置继续查找
            }
        }
    }

    // 第二遍：处理以B结尾的变量定义，进行替换
    for (int i = 0; i < lines.size(); ++i) {
        size_t assignPos = lines[i].find('=');
        if (assignPos == string::npos) continue;

        string variable = lines[i].substr(0, assignPos);
        string value = lines[i].substr(assignPos + 1);

        // 去除变量名和值的首尾空格
        variable.erase(0, variable.find_first_not_of(' '));
        variable.erase(variable.find_last_not_of(' ') + 1);
        value.erase(0, value.find_first_not_of(' '));
        value.erase(value.find_last_not_of(' ') + 1);

        // 处理以B结尾的变量定义
        if (variable.back() == 'B') {
            cout << "处理变量: " << variable << " = " << value << endl;
            string result = value;

            // 第一步：替换转义符号
            for (auto& mapping : specialCharToSymbol) {
                size_t pos = 0;
                string searchStr = mapping.first;
                char symbol = mapping.second;

                // 直接查找并替换整个转义序列
                while ((pos = result.find(searchStr, pos)) != std::string::npos) {
                    result.replace(pos, searchStr.length(), string(1, symbol));
                    pos += 1;
                }
            }

            // 第二步：替换字符类
            for (auto& mapping : name) {
                size_t pos = 0;
                string searchStr = mapping.first;

                // 跳过以B结尾的变量（避免递归替换）
                if (searchStr.back() == 'B') continue;

                // 如果是字符类，使用分配的符号
                if (charClassToSymbol.find(searchStr) != charClassToSymbol.end()) {
                    char symbol = charClassToSymbol[searchStr];
                    while ((pos = result.find(searchStr, pos)) != std::string::npos) {
                        result.replace(pos, searchStr.length(), string(1, symbol));
                        pos += 1;
                    }
                }
                // 如果是普通命名（不是字符类）
                else {
                    string replacement = "(" + mapping.second + ")";
                    while ((pos = result.find(searchStr, pos)) != std::string::npos) {
                        result.replace(pos, searchStr.length(), replacement);
                        pos += replacement.length();
                    }
                }
            }

            cout << "替换后: " << result << endl;
            resVec.push_back(result);
        }
    }

    // 将多个表达式用'|'连接
    for(size_t i = 0; i < resVec.size(); i++) {
        output += resVec[i];
        if(i < resVec.size() - 1){
            output += "|";
        }
    }

    cout << "最终输出: " << output << endl;
    return QString::fromStdString(output);
}


// 将set转为string，将set中结果在表格中展示
string setTostring(set<int> s)
{
    string result;

    for (int i : s) {
        result.append(to_string(i));
        result.append(",");
    }

    if (result.size() != 0)
        result.pop_back();

    return result;
}

/*
bool basicChar(char c)
{
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '!' || c == '@' || c == '+' ||
        c == '$' || c == '%' || c == '^' ||
        c == '&' || c == ',' || c == '=' ||
        c == '>' || c == '<' || c == '/' )
        return true;
    return false;
}
*/
bool basicChar(char c)
{
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '!' || c == '@' ||
        c == '$' || c == '%' || c == '^' ||
        c == '&' || c == ',' || c == '=' ||
        c == '>' || c == '<' || c == '/' ||
        // 包含字符类符号和特殊字符符号
        symbolToCharClass.find(c) != symbolToCharClass.end() ||
        symbolToSpecialChar.find(c) != symbolToSpecialChar.end())
        return true;
    return false;
}

// 预处理正则表达式
QString preprocess(QString regex)
{
    string stdRegex = regex.toStdString();



    /*
    // 处理中括号  [ABC] -> (A|B|C) [a-z] -> (a|b|c...|y|z)
    string result;
    bool isContent = false;     // 中括号内容范围标志
    string currentString;


    for (int i = 0; i < stdRegex[i]; i++) {
        if (stdRegex[i] == '[') {
            isContent = true;
            currentString.push_back('(');
        }
        else if (stdRegex[i] == ']') {
            isContent = false;
            currentString.push_back(')');
            result += currentString;
            currentString.clear();
        }
        else if (isContent) {
            // if条件以防出现 [ab] -> (|a|b)
            if (currentString.length() > 1) {
                currentString.push_back('|');
            }
            currentString.push_back(stdRegex[i]);
            if (stdRegex[i] == '-') {
                // 处理[a-z]
                if (islower(stdRegex[i - 1])) {
                    currentString.pop_back();       // 弹出'-'
                    for (int j = 1; j < stdRegex[i + 1] - 'a'; j++) {
                        currentString.push_back(stdRegex[i - 1] + j);
                        if (j != stdRegex[i + 1] - 'a' - 1) {
                            currentString.push_back('|');
                        }
                    }
                }
                // 处理[A-Z]
                else if (isupper(stdRegex[i - 1])) {
                    currentString.pop_back();       // 弹出'-'
                    for (int j = 1; j < stdRegex[i + 1] - 'A'; j++) {
                        currentString.push_back(stdRegex[i - 1] + j);
                        if (j != stdRegex[i + 1] - 'A' - 1) {
                            currentString.push_back('|');
                        }
                    }
                }
                // 处理[0-9]
                else if (isdigit(stdRegex[i - 1])) {
                    currentString.pop_back();       // 弹出'-'
                    for (int j = 1; j < stdRegex[i + 1] - '0'; j++) {
                        currentString.push_back(stdRegex[i - 1] + j);
                        if (j != stdRegex[i + 1] - '0' - 1) {
                            currentString.push_back('|');
                        }
                    }
                }
            }
        }
        // 正则表达式没有中括号的情况
        else {
            result.push_back(stdRegex[i]);
        }
    }
    */

    // 处理正闭包   (ab)+cd+ -> (ab)(ab)*cdd*
    for (int i = 0; i < stdRegex.size(); i++)
    {
        if (stdRegex[i] == '+' && stdRegex[i - 1] != '\\')
        {
            int bracketPairs = 0;     // 通过自减和自增匹配“(”和“)”
            int j = i;
            do
            {
                j--;
                if (stdRegex[j] == ')')
                {
                    bracketPairs++;
                }
                else if (stdRegex[j] == '(')
                {
                    bracketPairs--;
                }
            } while (bracketPairs != 0);

            string str1 = stdRegex.substr(0, j);            // 受正闭包约束的字符串的前一部分字符串
            string str2 = stdRegex.substr(j, i - j);        // 受正闭包“+”约束的字符串
            string str3 = stdRegex.substr(i + 1, (stdRegex.size() - i));        // 受正闭包约束的字符串的后一部分字符串
            stdRegex = str1 + str2 + str2 + "*" + str3;
        }
        /*
        // 处理\+
        else if(stdRegex[i] == '+' && stdRegex[i - 1] == '\\'){
            stdRegex.replace(i - 1, 1, ".");
        }
        */
    }



    /*
    //    int atPos = 0; // 记录“.”被替换成“@”在字符串中的位置
    // 处理转义字符
    for(size_t i = 1; i < stdRegex.size(); i++){
        // 处理\*
        if(stdRegex[i] == '*' && stdRegex[i - 1] == '\\'){
            stdRegex.replace(i - 1, 1, "");
        }
        // 处理\?
        if(stdRegex[i] == '?' && stdRegex[i - 1] == '\\'){
            stdRegex.replace(i - 1, 1, "");
        }
        // 处理\|
        //        if(regexStd[i] == '|' && regexStd[i - 1] == '\\'){
        //            atPos = i;
        //            regexStd.replace(i, 1, "@");
        //            regexStd.replace(i - 1, 1, ".");
        //        }
    }
    */


    for (int i = 0; i < stdRegex.size() - 1; i++)
    {
        if ((basicChar(stdRegex[i]) && basicChar(stdRegex[i + 1]))              // 连接两个操作数 ab -> a.b
            || (basicChar(stdRegex[i]) && stdRegex[i + 1] == '(')            // 连接操作数与括号 a(xxx) -> a.(xxx)
            || (stdRegex[i] == ')' && basicChar(stdRegex[i + 1]))            // 连接操作数与括号 (xxx)a -> (xxx).a
            || (stdRegex[i] == ')' && stdRegex[i + 1] == '(')             // 连接两个括号 (xxx)(xxx) -> (xxx).(xxx)
            || (stdRegex[i] == '*' && stdRegex[i + 1] != ')' && stdRegex[i + 1] != '|' && stdRegex[i + 1] != '?' && stdRegex[i + 1] != '*')     // (a.b)*c -> (a.b)*.c
            || (stdRegex[i] == '?' && stdRegex[i + 1] != ')' && stdRegex[i + 1] != '|' && stdRegex[i + 1] != '?' && stdRegex[i + 1] != '*')
            )
        {
            string str1 = stdRegex.substr(0, i + 1);
            string str2 = stdRegex.substr(i + 1, (stdRegex.size() - i));
            str1 += ".";
            stdRegex = str1 + str2;
        }
    }
    // 将被替换成“@”的“|”还原
    //    for(int i = 0; i < regexStd.length(); i++){
    //        if(regexStd[i] == '@' && i == atPos){
    //            regexStd.replace(i, 1, "|");
    //        }
    //    }

    cout << "regexStd: " << stdRegex << endl;
    return QString::fromStdString(stdRegex);
}

#endif // PROCESS_H
