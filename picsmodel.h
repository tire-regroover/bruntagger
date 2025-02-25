#ifndef PICSMODEL_H_
#define PICSMODEL_H_

#include <QAbstractListModel>
#include <QImage>
#include <QStringList>

namespace bruntagger {

class PicsModel: public QAbstractListModel {
    Q_OBJECT

private:
    QList<QImage> pics_;
    QStringList strings_;

public:
    PicsModel(QObject *parent = 0);
    virtual ~PicsModel();

    const QList<QImage> & pics() const { return pics_; }

    //QAbstractListModel:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    void clear();
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    void addPic(QImage &pic, QString &text);
};

}

#endif /* PICSMODEL_H_ */
