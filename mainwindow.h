#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "taglistitem.h"

#include <QMainWindow>
#include <QString>
#include <QListWidgetItem>
#include <vector>

struct TagData {
    QString name;
    QString value;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void getTemplateFile();
    void startManualSetup();
    void genJobFile();

    void saveManualInput();
    void getManualInput();
    void startAutoSetup();
    void returnToStart();

private:

    Ui::MainWindow *ui;
    QString templateFilePath;
    std::vector<QString> templateLines;

    std::vector<QListWidgetItem*> items;
    std::vector<TagListItem*> itemWidgets;

    std::vector<TagData> tagData;
    QString saveFilePath;

    void getManualSetupData();
};
#endif // MAINWINDOW_H
