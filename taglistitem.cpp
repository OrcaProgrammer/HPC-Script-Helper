#include "taglistitem.h"
#include "ui_taglistitem.h"

TagListItem::TagListItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TagListItem)
{
    ui->setupUi(this);
}

TagListItem::~TagListItem()
{
    delete ui;
}

void TagListItem::setText(const QString& text) {
    ui->label->setText(text);
}

void TagListItem::setData(const QString& data) {
    ui->lineEdit->setText(data);
}

QString TagListItem::getData() {
    return ui->lineEdit->text();
}
