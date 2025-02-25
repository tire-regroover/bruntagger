#ifndef DURATIONDELEGATE_H_
#define DURATIONDELEGATE_H_

#include <QStyledItemDelegate>

namespace bruntagger {

class DurationDelegate: public QStyledItemDelegate {
    Q_OBJECT

public:
    DurationDelegate(QObject *parent = 0);
    virtual ~DurationDelegate();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class DateTimeDelegate: public QStyledItemDelegate {
    Q_OBJECT

public:
    DateTimeDelegate(QObject *parent = 0);
    virtual ~DateTimeDelegate();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

}

#endif /* DURATIONDELEGATE_H_ */
