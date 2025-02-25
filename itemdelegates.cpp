#include "itemdelegates.h"

#include <QStyle>
#include <QApplication>
#include <QTime>

namespace bruntagger {

DurationDelegate::DurationDelegate(QObject *parent) : QStyledItemDelegate(parent) {
}

DurationDelegate::~DurationDelegate() {
}

void DurationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_ASSERT(index.isValid());
    QStyleOptionViewItemV4 v4option = option;
    initStyleOption(&v4option, index);
    const QWidget *widget;
    const QStyleOptionViewItemV4 *v4 = qstyleoption_cast<const QStyleOptionViewItemV4 *>(&option);
    v4 ? widget = v4->widget : widget = 0;
    QStyle *style = widget ? widget->style() : QApplication::style();
    if (index.model()->data(index, Qt::DisplayRole).type() == QVariant::Time) {
        QTime result = index.model()->data(index, Qt::DisplayRole).toTime();
        QString format;
        result >= QTime(1, 0) ? format = QString("h:mm:ss") : format = QString("m:ss");
        v4option.text = result.toString(format);
    }
    style->drawControl(QStyle::CE_ItemViewItem, &v4option, painter, widget);
}

DateTimeDelegate::DateTimeDelegate(QObject *parent) : QStyledItemDelegate(parent) {
}

DateTimeDelegate::~DateTimeDelegate() {
}

void DateTimeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_ASSERT(index.isValid());
    QStyleOptionViewItemV4 v4option = option;
    initStyleOption(&v4option, index);
    const QWidget *widget;
    const QStyleOptionViewItemV4 *v4 = qstyleoption_cast<const QStyleOptionViewItemV4 *>(&option);
    v4 ? widget = v4->widget : widget = 0;
    QStyle *style = widget ? widget->style() : QApplication::style();
    if (index.model()->data(index, Qt::DisplayRole).type() == QVariant::DateTime) {
        QDateTime result = index.model()->data(index, Qt::DisplayRole).toDateTime();
        QString format = "yyyy-MM-dd h:mm:ssa";
        v4option.text = result.toString(format);
    }
    style->drawControl(QStyle::CE_ItemViewItem, &v4option, painter, widget);
}

}
