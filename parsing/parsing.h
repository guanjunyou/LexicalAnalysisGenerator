#ifndef PARSING_H
#define PARSING_H

#include <QMainWindow>

namespace Ui {
class Parsing;
}

class Parsing : public QMainWindow
{
    Q_OBJECT

public:
    explicit Parsing(QWidget *parent = 0);
    ~Parsing();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

private:
    Ui::Parsing *ui;
};

#endif // PARSING_H
