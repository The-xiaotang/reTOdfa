#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:

    void on_run_clicked();

    void displayNFAButton();

    void displayDFAButton();

    void displayMinDFAButton();

    void on_tabWidget_currentChanged(int index);

    void on_upload_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
