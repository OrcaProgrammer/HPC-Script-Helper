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

    templateFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("Templates (*.txt)"));

    ui->templateFileLabel->setText(templateFilePath);
    if (templateFilePath == "") return;

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    auto secondLastIndex{ templateFilePath.lastIndexOf('/', lastIndex - 1) };
    QString fileName{ templateFilePath.sliced(secondLastIndex) };

    ui->templateFileLabel->setText("..." + fileName);
};



void MainWindow::startManualSetup() {


    // Try to open template file ----------------------------

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

    // ------------------------------------------------------



    // Parse template file ----------------------------------

    std::string templateLine{};
    int i = 0;

    while (std::getline(templateFile, templateLine)) {

        QString lineText{ QString::fromStdString(templateLine) };
        templateLines.push_back(lineText);

        bool endOfLine = false;
        int prevTagBegin = 0;
        int prevTagEnd = 0;

        while (!endOfLine) {

            auto tagBegin{ lineText.indexOf("{%", prevTagBegin + 1) };
            auto tagEnd{ lineText.indexOf("%}", prevTagEnd + 1) };
            int tagSize{ 2 };
            int sliceBegin;
            int sliceEnd;

            if (tagBegin != -1 && tagEnd != -1) {

                // Delete any spaces after tag start
                sliceBegin = tagBegin + tagSize;
                while (lineText[sliceBegin] == ' ') {
                    sliceBegin++;
                }

                // Delete any spaces before tag end
                sliceEnd = (tagEnd + 1) - tagSize;
                while (lineText[sliceEnd] == ' ') {
                    sliceEnd--;
                }

                TagData newTag;
                newTag.name = lineText.sliced(sliceBegin, (sliceEnd + 1) - sliceBegin);
                tagData.push_back(newTag);

                prevTagBegin = tagBegin;
                prevTagEnd = tagEnd;
            } else {
                endOfLine = true;
            }
        }

        i++;
    }

    // -----------------------------------------------------------



    // Clean duplicate tags --------------------------------------

    std::vector<int> tagRemovePositions;
    for (int i = 0; i < tagData.size(); i++) {
        for (int j = 0; j < tagData.size(); j++) {
            if (tagData[i].name == tagData[j].name && i != j)
                tagRemovePositions.push_back(j);
        }
    }

    std::vector<TagData> cleanedData;
    for (int i = 0; i < tagData.size(); i++) {

        bool cleanTag = true;

        for (int j = 0; j < tagRemovePositions.size(); j+=2) {
            if (i == tagRemovePositions[j])
                cleanTag = false;
        }

        if (cleanTag)
            cleanedData.push_back(tagData[i]);
    }

    tagData = cleanedData;

    // ---------------------------------------------------------




    // Update UI -----------------------------------------------

    for (int i = 0; i < tagData.size(); ++i) {

        items.push_back(new QListWidgetItem());
        itemWidgets.push_back(new TagListItem());

        itemWidgets[i]->setText(tagData[i].name);
        items[i]->setSizeHint(itemWidgets[i]->sizeHint());

        ui->manualListWidget->addItem(items[i]);
        ui->manualListWidget->setItemWidget(items[i], itemWidgets[i]);

    }

    ui->stackedWidget->setCurrentWidget(ui->manualSetupPage);

    // ---------------------------------------------------------
}



void MainWindow::getManualSetupData() {

    // Get user input from UI ---------------------------------

    for (int i = 0; i < itemWidgets.size(); i++) {
        QString data = itemWidgets[i]->getData();

        if (data != "")
            tagData[i].value = itemWidgets[i]->getData();
    }

    // --------------------------------------------------------
}



void MainWindow::saveManualInput() {

    getManualSetupData();

    // Check if user has missed any fields -------------------

    bool isEmpty = false;

    for (int i = 0; i < itemWidgets.size(); ++i) {
        if (itemWidgets[i]->getData() == "")
            isEmpty = true;
    }

    if (isEmpty) {
        QMessageBox msgBox;
        msgBox.setText("Missing data from input fields.");
        msgBox.exec();
        return;
    }

    // -------------------------------------------------------



    // Save data to csv file ---------------------------------

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    saveFilePath = QFileDialog::getSaveFileName(this, tr("Save Input to File"), templateFilePath.first(lastIndex + 1), tr("CSV (*.csv)"));

    if(saveFilePath == "") return;

    std::ofstream saveFile{ saveFilePath.toStdString() };

    saveFile << "Tag Name:" << "," << "Tag Data:" << ",\n";

    for (int i = 0; i < tagData.size(); i++) {
        saveFile << tagData[i].name.toStdString() << "," << tagData[i].value.toStdString() << ",\n";
    }

    saveFile.close();

    QMessageBox msgBox;
    msgBox.setText("The input was succesfully saved at: " + saveFilePath);
    msgBox.exec();

    // --------------------------------------------------------
}



void MainWindow::getManualInput() {

    // Try to open file ---------------------------------------

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    saveFilePath = QFileDialog::getOpenFileName(this, tr("Load Tag Inputs from File"), templateFilePath.first(lastIndex + 1), tr("CSV (*.csv)"));

    if (saveFilePath == "") return;

    std::ifstream inputFile{ saveFilePath.toStdString() };

    if (!inputFile) {
        QMessageBox fileError;
        fileError.setText("File failed to read!");
        fileError.setIcon(QMessageBox::Critical);
        fileError.exec();
        return;
    }

    // --------------------------------------------------------



    // Read data from file ------------------------------------

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

    // ---------------------------------------------------------



    // Update program variables and UI -------------------------

    for (int i = 0; i < rows.size(); i++) {

        QString row = QString::fromStdString(rows[i]);
        auto firstIndex{ row.indexOf(',') };
        auto secondIndex{ row.indexOf(',', firstIndex + 1) };

        if (tagData[i].name == row.first(firstIndex) && secondIndex != -1) {
            tagData[i].value = row.sliced(firstIndex + 1, (secondIndex - firstIndex) - 1);
            itemWidgets[i]->setData(tagData[i].value);
        } else if (tagData[i].name == row.first(firstIndex) && secondIndex == -1) {
            tagData[i].value = row.sliced(firstIndex + 1, (row.size() - firstIndex) - 1);
            itemWidgets[i]->setData(tagData[i].value);
        }
    }

    // ----------------------------------------------------------
}



void MainWindow::startAutoSetup() {

}



void MainWindow::genJobFile() {

    // Get data from either manual or automatic method -------

    if (ui->stackedWidget->currentWidget() == ui->manualSetupPage) {
        getManualSetupData();
    } else {
        // getAutoSetupData();
    }

    // -------------------------------------------------------



    // Check if user has missed any fields -------------------

    bool isEmpty = false;

    for (int i = 0; i < itemWidgets.size(); ++i) {
        if (itemWidgets[i]->getData() == "")
            isEmpty = true;
    }

    if (isEmpty) {
        QMessageBox msgBox;
        msgBox.setText("Missing data from input fields.");
        msgBox.exec();
        return;
    }

    // -------------------------------------------------------



    // Open save file ----------------------------------------

    auto lastIndex{ templateFilePath.lastIndexOf('/') };
    saveFilePath = QFileDialog::getSaveFileName(this, tr("Generate Script"), templateFilePath.first(lastIndex + 1));

    if(saveFilePath == "") return;

    std::ofstream jobFile{ saveFilePath.toStdString() };

    // -------------------------------------------------------



    // Fill in tag data --------------------------------------

    for (int i = 0; i < templateLines.size(); i++) {

        QString lineText{ templateLines[i] };
        QString filledLineText{};

        if(lineText == "") continue;

        bool endOfLine = false;
        bool tagFound = false;
        int prevTagBegin = 0;
        int prevTagEnd = 0;

        while (!endOfLine) {

            auto tagBegin{ lineText.indexOf("{%", prevTagBegin + 1) };
            auto tagEnd{ lineText.indexOf("%}", prevTagEnd + 1) };
            int tagSize{ 2 };
            int sliceBegin;
            int sliceEnd;

            if (tagBegin != -1 && tagEnd != -1) {

                tagFound = true;

                if (prevTagEnd == 0) {
                    filledLineText += lineText.sliced(prevTagEnd, tagBegin - prevTagEnd);
                } else {
                    filledLineText += lineText.sliced(prevTagEnd + tagSize, tagBegin - (prevTagEnd+ tagSize));
                }

                // Delete any spaces after tag start
                sliceBegin = tagBegin + tagSize;
                while (lineText[sliceBegin] == ' ') {
                    sliceBegin++;
                }

                // Delete any spaces before tag end
                sliceEnd = (tagEnd + 1) - tagSize;
                while (lineText[sliceEnd] == ' ') {
                    sliceEnd--;
                }

                QString tagName = lineText.sliced(sliceBegin, (sliceEnd + 1) - sliceBegin);

                for (int j = 0; j < tagData.size(); j++) {
                    if (tagName == tagData[j].name){
                        filledLineText += tagData[j].value;
                        break;
                    }
                }

                prevTagBegin = tagBegin;
                prevTagEnd = tagEnd;
            } else {

                if (tagFound) {
                    filledLineText += lineText.sliced(prevTagEnd + tagSize, lineText.size() - (prevTagEnd+ tagSize));
                } else{
                    filledLineText = lineText;
                }

                endOfLine = true;
            }
        }

        templateLines[i] = filledLineText;
    }

    // -------------------------------------------------------



    // Save data to file -------------------------------------

    for (int i = 0; i < templateLines.size(); i++) {
        jobFile << templateLines[i].toStdString() << '\n';
    }

    jobFile.close();

    QMessageBox msgBox;
    msgBox.setText("The job script was generated at: " + saveFilePath);
    msgBox.exec();

    // --------------------------------------------------------



    // Wipe data and return to start --------------------------

    templateFilePath = "";
    ui->templateFileLabel->setText("");

    returnToStart();

    // --------------------------------------------------------
}



void MainWindow::returnToStart() {

    ui->manualListWidget->clear();

    items.clear();
    itemWidgets.clear();
    tagData.clear();

    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}
