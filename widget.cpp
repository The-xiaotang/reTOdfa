#include "widget.h"
#include "ui_widget.h"

#include"re2nfa.h"
#include"nfa2dfa.h"
#include"dfa2mindfa.h"
#include"process.h"


#include "widget.h"
#include "ui_widget.h"
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <map>
#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <sstream>
#include <fstream>
#include <QFile>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include <iostream>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void clearAll(){
    nodeCount = 0;
    nfaCharSet.clear();
    dfaCharSet.clear();
    statusTable.clear();
    insertionOrder.clear();
    startNFAstatus.clear();
    endNFAstatus.clear();
    dfaStatusSet.clear();
    dfaEndStatusSet.clear();
    dfaNotEndStatusSet.clear();
    dfaMinVector.clear();
    dividedSetVec.clear();
    dfaSameMinSet.clear();
    dfaTable.clear();
    charClassToSymbol.clear();
    symbolToCharClass.clear();
    specialCharToSymbol.clear();
    symbolToSpecialChar.clear();
    currentSymbol = 'A';  // 重置符号分配
    currentSpecialSymbol = 'W';  // 重置特殊符号分配
    name.clear();  // 清空命名映射
}

void Widget::on_run_clicked()
{
    clearAll();

    nfaCharSet.insert(EPSILON); // 放入epsilon
    QString regex = ui->input->toPlainText();   // 拿到正则表达式

    regex.remove(' ');

    // 多行表达式合并成一行
    regex = merge(regex);

    // 预处理
    regex = preprocess(regex);
    qDebug() << regex;


    string regexStd = regex.toStdString();

    NFA nfa = regexToNFA(regexStd);

    // NFA转DFA
    NFAToDFA(nfa);

    DFAminimize();

    QMessageBox::about(this,"提示" ,"运行成功！");


    displayNFAButton();
    displayDFAButton();
    displayMinDFAButton();
}

/*
// NFA展示按钮
void Widget::displayNFAButton()
{
    ui->NFAtable->clearContents(); // 清除表格中的数据
    ui->NFAtable->setRowCount(0); // 清除所有行
    ui->NFAtable->setColumnCount(0); // 清除所有列
    // 设置列数
    int n = 2 + nfaCharSet.size(); // 默认两列：Flag 和 ID
    ui->NFAtable->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "-:初态 +:终态" << "结点标号";
    int headerCount = 3;
    for (const auto& ch : nfaCharSet) {
        headerLabels << QString(ch);
        headerCharNum[ch] = headerCount++;
    }
    ui->NFAtable->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = statusTable.size();
    ui->NFAtable->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto id : insertionOrder) {
        const nfaStatusNode& node = statusTable[id];
        cout << "id: " << id << endl;

        // Flag 列
        ui->NFAtable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(node.flag)));

        // ID 列
        ui->NFAtable->setItem(row, 1, new QTableWidgetItem(QString::number(node.id)));

        // TransitionChar 列
        int col = 2;
        for (const auto& transitionEntry : node.transition) {
            string resutlt = setTostring(transitionEntry.second);

            // 放到指定列数据
            ui->NFAtable->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(QString::fromStdString(resutlt)));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->NFAtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 显示表格
    ui->NFAtable->show();
}
*/

void Widget::displayNFAButton()
{
    ui->NFAtable->clearContents();
    ui->NFAtable->setRowCount(0);
    ui->NFAtable->setColumnCount(0);

    int n = 2 + nfaCharSet.size();
    ui->NFAtable->setColumnCount(n);

    map<char, int> headerCharNum;

    QStringList headerLabels;
    headerLabels << "-:初态 +:终态" << "结点标号";
    int headerCount = 2;
    for (const auto& ch : nfaCharSet) {
        QString headerName;
        // 如果是字符类符号，使用字符类名称
        if (symbolToCharClass.find(ch) != symbolToCharClass.end()) {
            headerName = QString::fromStdString(symbolToCharClass[ch]);
        }
        // 如果是特殊字符符号，使用特殊字符名称
        else if (symbolToSpecialChar.find(ch) != symbolToSpecialChar.end()) {
            headerName = QString::fromStdString(symbolToSpecialChar[ch]);
        }
        // 普通字符
        else {
            headerName = QString(ch);
        }
        headerLabels << headerName;
        headerCharNum[ch] = headerCount++;
    }
    ui->NFAtable->setHorizontalHeaderLabels(headerLabels);

    int rowCount = statusTable.size();
    ui->NFAtable->setRowCount(rowCount);

    int row = 0;
    for (auto id : insertionOrder) {
        const nfaStatusNode& node = statusTable[id];
        cout << "id: " << id << endl;

        ui->NFAtable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(node.flag)));
        ui->NFAtable->setItem(row, 1, new QTableWidgetItem(QString::number(node.id)));

        for (const auto& transitionEntry : node.transition) {
            string result = setTostring(transitionEntry.second);
            ui->NFAtable->setItem(row, headerCharNum[transitionEntry.first],
                                  new QTableWidgetItem(QString::fromStdString(result)));
        }
        row++;
    }

    ui->NFAtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->NFAtable->show();
}

/*
// DFA展示按钮
void Widget::displayDFAButton()
{
    ui->DFAtable->clearContents(); // 清除表格中的数据
    ui->DFAtable->setRowCount(0); // 清除所有行
    ui->DFAtable->setColumnCount(0); // 清除所有列

    // 设置列数
    int n = 2 + dfaCharSet.size(); // 默认两列：Flag 和 状态集合
    ui->DFAtable->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "-:初态 +:终态" << "状态集合";
    int headerCount = 3;
    for (const auto& ch : dfaCharSet) {
        headerLabels << QString(ch);
        headerCharNum[ch] = headerCount++;
    }
    ui->DFAtable->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = dfaTable.size();
    ui->DFAtable->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaTable) {

        // Flag 列
        ui->DFAtable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->DFAtable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString("{" + setTostring(dfaNode.nfaStates) + "}")));

        // 状态转换 列
        int col = 2;
        for (const auto& transitionEntry : dfaNode.transitions) {
            string re = setTostring(transitionEntry.second);

            // 放到指定列数据
            ui->DFAtable->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(QString::fromStdString("{" + re + "}")));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->DFAtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 显示表格
    ui->DFAtable->show();
}
*/
void Widget::displayDFAButton()
{
    ui->DFAtable->clearContents(); // 清除表格中的数据
    ui->DFAtable->setRowCount(0); // 清除所有行
    ui->DFAtable->setColumnCount(0); // 清除所有列

    // 设置列数
    int n = 2 + dfaCharSet.size(); // 默认两列：Flag 和 状态集合
    ui->DFAtable->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "-:初态 +:终态" << "状态集合";
    int headerCount = 2;
    for (const auto& ch : dfaCharSet) {
        QString headerName;
        // 如果是字符类符号，使用字符类名称
        if (symbolToCharClass.find(ch) != symbolToCharClass.end()) {
            headerName = QString::fromStdString(symbolToCharClass[ch]);
        }
        // 如果是特殊字符符号，使用特殊字符名称
        else if (symbolToSpecialChar.find(ch) != symbolToSpecialChar.end()) {
            headerName = QString::fromStdString(symbolToSpecialChar[ch]);
        }
        // 普通字符
        else {
            headerName = QString(ch);
        }
        headerLabels << headerName;
        headerCharNum[ch] = headerCount++;
    }
    ui->DFAtable->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = dfaTable.size();
    ui->DFAtable->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaTable) {

        // Flag 列
        ui->DFAtable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->DFAtable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString("{" + setTostring(dfaNode.nfaStates) + "}")));

        // 状态转换 列
        for (const auto& transitionEntry : dfaNode.transitions) {
            char ch = transitionEntry.first;
            const set<int>& nextStates = transitionEntry.second;

            string re = setTostring(nextStates);

            // 找到对应的列
            if (headerCharNum.find(ch) != headerCharNum.end()) {
                int col = headerCharNum[ch];
                ui->DFAtable->setItem(row, col, new QTableWidgetItem(QString::fromStdString("{" + re + "}")));
            }
        }

        row++;
    }

    // 调整列宽
    ui->DFAtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 显示表格
    ui->DFAtable->show();
}

/*
// 最小化DFA展示按钮
void Widget::displayMinDFAButton()
{
    ui->MIN_DFAtable->clearContents(); // 清除表格中的数据
    ui->MIN_DFAtable->setRowCount(0); // 清除所有行
    ui->MIN_DFAtable->setColumnCount(0); // 清除所有列

    // 设置列数
    int n = 2 + dfaCharSet.size(); // 默认两列：Flag 和 状态集合
    ui->MIN_DFAtable->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "-:初态 +:终态" << "结点标号";
    int headerCount = 3;
    for (const auto& ch : dfaCharSet) {
        headerLabels << QString(ch);
        headerCharNum[ch] = headerCount++;
    }
    ui->MIN_DFAtable->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = dfaMinVector.size();
    ui->MIN_DFAtable->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaMinVector) {

        // Flag 列
        ui->MIN_DFAtable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->MIN_DFAtable->setItem(row, 1, new QTableWidgetItem(QString::number(dfaNode.id)));

        // 状态转换 列
        int col = 2;
        for (const auto& transitionEntry : dfaNode.transitions) {
            // 放到指定列数据
            ui->MIN_DFAtable->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(transitionEntry.second == -1 ? QString::fromStdString("") : QString::number(transitionEntry.second)));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->MIN_DFAtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 显示表格
    ui->MIN_DFAtable->show();

}
*/

void Widget::displayMinDFAButton()
{
    ui->MIN_DFAtable->clearContents(); // 清除表格中的数据
    ui->MIN_DFAtable->setRowCount(0); // 清除所有行
    ui->MIN_DFAtable->setColumnCount(0); // 清除所有列

    // 设置列数
    int n = 2 + dfaCharSet.size(); // 默认两列：Flag 和 状态集合
    ui->MIN_DFAtable->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "-:初态 +:终态" << "结点标号";
    int headerCount = 2;
    for (const auto& ch : dfaCharSet) {
        QString headerName;
        // 如果是字符类符号，使用字符类名称
        if (symbolToCharClass.find(ch) != symbolToCharClass.end()) {
            headerName = QString::fromStdString(symbolToCharClass[ch]);
        }
        // 如果是特殊字符符号，使用特殊字符名称
        else if (symbolToSpecialChar.find(ch) != symbolToSpecialChar.end()) {
            headerName = QString::fromStdString(symbolToSpecialChar[ch]);
        }
        // 普通字符
        else {
            headerName = QString(ch);
        }
        headerLabels << headerName;
        headerCharNum[ch] = headerCount++;
    }
    ui->MIN_DFAtable->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = dfaMinVector.size();
    ui->MIN_DFAtable->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaMinVector) {

        // Flag 列
        ui->MIN_DFAtable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->MIN_DFAtable->setItem(row, 1, new QTableWidgetItem(QString::number(dfaNode.id)));

        // 状态转换 列
        for (const auto& transitionEntry : dfaNode.transitions) {
            char ch = transitionEntry.first;
            int nextState = transitionEntry.second;

            // 找到对应的列
            if (headerCharNum.find(ch) != headerCharNum.end()) {
                int col = headerCharNum[ch];
                QString displayText = (nextState == -1) ? "" : QString::number(nextState);
                ui->MIN_DFAtable->setItem(row, col, new QTableWidgetItem(displayText));
            }
        }

        row++;
    }

    // 调整列宽
    ui->MIN_DFAtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 显示表格
    ui->MIN_DFAtable->show();
}

// 打开正则表达式txt文件
void Widget::on_upload_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("文本文件 (*.txt);;所有文件 (*.*)"));

    if (!filePath.isEmpty())
    {
        ifstream inputFile;
        QTextCodec* code = QTextCodec::codecForName("GB2312");

        string selectedFile = code->fromUnicode(filePath.toStdString().c_str()).data();
        inputFile.open(selectedFile.c_str(), ios::in);


        //        cout<<filePath.toStdString();
        //        ifstream inputFile(filePath.toStdString());
        if (!inputFile) {
            QMessageBox::critical(this, "错误", "无法打开文件！");
            cerr << "Error opening file." << endl;
        }
        // 读取文件内容并显示在 plainTextEdit_2
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContents = QString::fromStdString(buffer.str());
        ui->input->setPlainText(fileContents);
    }
}


void Widget::on_tabWidget_currentChanged(int index)
{

}

