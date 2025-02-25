#include "picsmodel.h"

#include <QIcon>
#include <QMimeData>

namespace bruntagger {

PicsModel::PicsModel(QObject *parent) : QAbstractListModel(parent) {
}

PicsModel::~PicsModel() {
}

int PicsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return pics_.count();
}

QVariant PicsModel::data(const QModelIndex &index, int role) const {
    switch (role) {
        case Qt::DecorationRole:
            return QIcon(QPixmap::fromImage(pics_.at(index.row())));
            break;
        case Qt::ToolTipRole:
        case Qt::EditRole:
        case Qt::DisplayRole:
            return strings_.at(index.row());
            break;
    }
    return QVariant();
}

void PicsModel::clear() {
    beginResetModel();
    pics_.clear();
    strings_.clear();
    endResetModel();
}

bool PicsModel::removeRows(int row, int count, const QModelIndex &parent) {
    beginRemoveRows(parent, row, row + count);
    for (int i = 0; i < count; i++) {
        pics_.removeAt(row + i);
        strings_.removeAt(row + i);
    }
    endRemoveRows();
    return count > 0;
}

void PicsModel::addPic(QImage &pic, QString &text) {
    beginInsertRows(QModelIndex(), pics_.count() - 1, pics_.count() - 1);
    pics_.append(pic);
    strings_.append(text);
    endInsertRows();
}


}
