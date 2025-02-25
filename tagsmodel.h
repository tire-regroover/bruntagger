#ifndef TAGSMODEL_H_
#define TAGSMODEL_H_

#include <QAbstractListModel>
#include <QStringList>
#include <QFileIconProvider>

namespace bruntagger {

class Mp3;

class TagsModel: public QAbstractListModel {
    Q_OBJECT
    Q_ENUMS(Column)

public:
    enum Column {
        Album = 0,
        Artist,
        BitRate,
        Composer,
        Date,
        Disc,
        Ext,
        File,
        Genre,
        HasId3V1,
        Length,
        Modified,
        Path,
        Pics,
        Title,
        Track
    };

private:
    bool stop_requested_;
    QList<Mp3*> mp3s_;
    QStringList columns_;
    QList<QModelIndex> dirty_indexes_;
    QFileIconProvider icon_provider_;

    QList<int> read_only_columns_;

public:
    TagsModel(QObject *parent = 0);
    virtual ~TagsModel();

    const QString & column(int column) const;
    const QStringList & columns() const { return columns_; }
    const QList<int> & read_only_columns() const { return read_only_columns_; }
    const QList<Mp3*> & mp3s() const { return mp3s_; }

    //QAbstractListModel:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void go(QString path, QString filter, bool recurse, bool show_dirs);
    void stop();
    int save();
    void undoChanges();

    static QVariant tryToInt(QString input);
};

}

#endif /* TAGSMODEL_H_ */
