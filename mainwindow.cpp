#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>

#include <QMessageBox>

#include <fstream>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->mainPage);

    connect(ui->templateFileButton, SIGNAL(clicked()), this, SLOT(getTemplateFile()));
    connect(ui->manSetupButton, SIGNAL(clicked()), this, SLOT(startManualSetup()));
    connect(ui->saveInputButton, SIGNAL(clicked()), this, SLOT(saveManualInput()));
    connect(ui->loadInputButton, SIGNAL(clicked()), this, SLOT(getManualInput()));
    //connect(ui->autoSetupButton, SIGNAL(clicked()), this, SLOT(startAutoSetup()));
    connect(ui->returnButton1, SIGNAL(clicked()), this, SLOT(returnToStart()));
    connect(ui->generateButton1, SIGNAL(clicked()), this, SLOT(genJobFile()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getTemplateFile() {

    templateFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("Templates (*.tpl)"));

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    auto secondLastIndex{ templateFilePath.lastIndexOf('/', lastIndex - 1) };
    QString fileName{ templateFilePath.sliced(secondLastIndex) };

    ui->templateFileLabel->setText("..." + fileName);
};

void MainWindow::startManualSetup() {

    if (templateFilePath == "") {
        QMessageBox::warning(this, tr("My Application"), tr("No template file selected."));
        return;
    }

    std::ifstream templateFile{ templateFilePath.toStdString() };

    if (!templateFile) {
        QMessageBox fileError;
        fileError.setText("File failed to read!");
        fileError.setIcon(QMessageBox::Critical);
        fileError.exec();
        return;
    }

    std::string templateLine{};
    int i = 0;

    while (std::getline(templateFile, templateLine)) {

        QString lineText{ QString::fromStdString(templateLine) };
        templateLines.push_back(lineText);

        auto tagBegin{ lineText.indexOf("{%") };
        auto tagEnd{ lineText.indexOf("%}") };
        int tagSize{ 2 };
        int sliceBegin;
        int sliceEnd;

        if (tagBegin != -1 && tagEnd != -1) {

            sliceBegin = tagBegin + tagSize;
            while (lineText[sliceBegin] == ' ') {
                sliceBegin++;
            }

            sliceEnd = (tagEnd + 1) - tagSize;
            while (lineText[sliceEnd] == ' ') {
                sliceEnd--;
            }

            tagNames.push_back(lineText.sliced(sliceBegin, (sliceEnd + 1) - sliceBegin));
            tagPositions.push_back(i);
        }

        i++;
    }

    for (int i = 0; i < tagNames.size(); ++i) {
        items.push_back(new QListWidgetItem());
        itemWidgets.push_back(new TagListItem());

        itemWidgets[i]->setText(tagNames[i]);
        items[i]->setSizeHint(itemWidgets[i]->sizeHint());

        ui->manualListWidget->addItem(items[i]);
        ui->manualListWidget->setItemWidget(items[i], itemWidgets[i]);
    }

    ui->stackedWidget->setCurrentWidget(ui->manualSetupPage);
}

void MainWindow::getManualSetupData() {

    if (tagData.size() > 0) return;

    for (int i = 0; i < itemWidgets.size(); i++) {
        QString data = itemWidgets[i]->getData();

        if (data != "")
            tagData.push_back(itemWidgets[i]->getData());
    }
}

void MainWindow::saveManualInput() {

    getManualSetupData();

    if (tagData.size() == 0) {
        QMessageBox msgBox;
        msgBox.setText("No data to save.");
        msgBox.exec();
        return;
    }

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    saveFilePath = QFileDialog::getSaveFileName(this, tr("Save Input to File"), templateFilePath.first(lastIndex + 1), tr("CSV (*.csv)"));

    if(saveFilePath == "") return;

    std::ofstream saveFile{ saveFilePath.toStdString() };

    saveFile << "Tag Name:" << "," << "Tag Data:" << ",\n";

    for (int i = 0; i < tagNames.size(); i++) {
        saveFile << tagNames[i].toStdString() << "," << tagData[i].toStdString() << ",\n";
    }

    saveFile.close();

    QMessageBox msgBox;
    msgBox.setText("The input was succesfully saved at: " + saveFilePath);
    msgBox.exec();
}

void MainWindow::getManualInput() {

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    saveFilePath = QFileDialog::getOpenFileName(this, tr("Load Tag Inputs from File"), templateFilePath.first(lastIndex + 1), tr("CSV (*.csv)"));

    std::ifstream inputFile{ saveFilePath.toStdString() };

    if (!inputFile) {
        QMessageBox fileError;
        fileError.setText("File failed to read!");
        fileError.setIcon(QMessageBox::Critical);
        fileError.exec();
        return;
    }

    std::vector<std::string> rows{};
    std::string data{};
    bool firstLine = true;

    while (std::getline(inputFile, data)) {

        if (firstLine) {
            firstLine = false;
            continue;
        } else {
            rows.push_back(data);
        }
    }

    tagData.clear();

    for (int i = 0; i < rows.size(); i++) {

        QString row = QString::fromStdString(rows[i]);
        auto firstIndex{ row.indexOf(',') };
        auto secondIndex{ row.indexOf(',', firstIndex + 1) };

        if (tagNames[i] == row.first(firstIndex) && secondIndex != -1) {
            tagData.push_back(row.sliced(firstIndex + 1, (secondIndex - firstIndex) - 1));
            itemWidgets[i]->setData(tagData[i]);
        } else if (tagNames[i] == row.first(firstIndex) && secondIndex == -1) {
            tagData.push_back(row.sliced(firstIndex + 1, (row.size() - firstIndex) - 1));
            itemWidgets[i]->setData(tagData[i]);
        }
    }
}

void MainWindow::startAutoSetup() {

}

void MainWindow::genJobFile() {

    if (ui->stackedWidget->currentWidget() == ui->manualSetupPage) {
        getManualSetupData();
    } else {
        // getAutoSetupData();
    }

    if (tagData.size() == 0) {
        QMessageBox msgBox;
        msgBox.setText("No data to generate job script.");
        msgBox.exec();
        return;
    }

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    saveFilePath = QFileDialog::getSaveFileName(this, tr("Generate Script"), templateFilePath.first(lastIndex + 1));

    if(saveFilePath == "") return;

    std::ofstream jobFile{ saveFilePath.toStdString() };

    for (int i = 0; i < tagData.size(); i++) {

        QString lineText{ templateLines[tagPositions[i]] };
        auto tagBegin{ lineText.indexOf("{%") };
        auto tagEnd{ lineText.indexOf("%}") };

        if (tagBegin != -1 && tagEnd != -1) {
            templateLines[tagPositions[i]] = lineText.replace(tagBegin, tagBegin + tagEnd, tagData[i]);
        }
    }

    for (int i = 0; i < templateLines.size(); i++) {
        jobFile << templateLines[i].toStdString() << '\n';
    }

    jobFile.close();

    QMessageBox msgBox;
    msgBox.setText("The job script was generated at: " + saveFilePath);
    msgBox.exec();

    templateFilePath = "";
    ui->templateFileLabel->setText("");

    returnToStart();
}

void MainWindow::returnToStart() {

    ui->manualListWidget->clear();

    items.clear();
    itemWidgets.clear();
    tagNames.clear();
    tagData.clear();
    tagPositions.clear();

    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}
