#ifndef TAGLISTITEM_H
#define TAGLISTITEM_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QString>

namespace Ui {
class TagListItem;
}

class TagListItem : public QWidget
{
    Q_OBJECT

public:
    explicit TagListItem(QWidget *parent = nullptr);
    ~TagListItem();

    void setText(const QString& text);
    void setData(const QString& data);
    QString getData();

private:

    Ui::TagListItem *ui;

};

#endif // TAGLISTITEM_H
