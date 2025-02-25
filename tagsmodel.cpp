#include "tagsmodel.h"
#include "mp3.h"
#include <QtCore>
#include <QStandardItem>

namespace bruntagger {

TagsModel::TagsModel(QObject *parent) : QAbstractListModel(parent) {
    columns_
            << tr("Album")
            << tr("Artist")
            << tr("Bit Rate")
            << tr("Composer")
            << tr("Date")
            << tr("Disc")
            << tr("Ext")
            << tr("File")
            << tr("Genre")
            << tr("HasId3V1")
            << tr("Length")
            << tr("Modified")
            << tr("Path")
            << tr("Pics")
            << tr("Title")
            << tr("Track");

    read_only_columns_ << BitRate << Ext << HasId3V1 << Length << Modified << Pics;
}

TagsModel::~TagsModel() {
    while (mp3s_.count() > 0) {
        delete mp3s_.at(0);
        mp3s_.removeAt(0);
    }
}

const QString & TagsModel::column(int column) const {
    return columns_.at(column);
}

int TagsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return mp3s_.count();
}

int TagsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return columns_.count();
}

Qt::ItemFlags TagsModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (!index.isValid() || read_only_columns_.contains(index.column()))
        return flags;

    Mp3 *mp3 = mp3s_.at(index.row());
    if (!mp3->isMp3())
        return flags;

    flags = flags | Qt::ItemIsEditable;
    return flags;
}

QVariant TagsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (orientation) {
        case Qt::Vertical: {
            Mp3 *mp3 = mp3s_.at(section);
            switch (role) {
                case Qt::DisplayRole:
                case Qt::ToolTipRole:
                    return QString();
                case Qt::DecorationRole: {
                    if (mp3->is_dirty())
                        return QIcon::fromTheme("flag-red");
                    QPixmap pic(16, 16);
                    pic.fill(QColor::fromRgb(255, 255, 255, 0));
                    return QIcon(pic);
                }
            }
            break;
        }
        case Qt::Horizontal:
            switch (role) {
                case Qt::DisplayRole:
                case Qt::ToolTipRole:
                    return column(section);
            }
            break;
    }

    return QVariant();
}

QVariant TagsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    Mp3 *mp3 = mp3s_.at(index.row());

    switch (role) {
        case Qt::TextAlignmentRole: {
            switch (index.column()) {
                case Ext:
                case BitRate:
                case Length:
                case Track:
                case Disc:
                case Date:
                case Pics:
                case HasId3V1:
                    return Qt::AlignCenter;
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            }
            break;
        }
        case Qt::DecorationRole: {
            if (dirty_indexes_.contains(index))
                return QIcon::fromTheme("flag-red");
            switch (index.column()) {
                case File:
                case Path:
                    return icon_provider_.icon(QFileInfo(mp3->path()));
            }
            break;
        }
        case Qt::ToolTipRole: {
            switch (index.column()) {
                case Ext: {
                    QFileInfo file(mp3->path());
                    QString ext = file.suffix();
                    if (!mp3->filename().isEmpty() && file.isDir() && !dirty_indexes_.contains(index))
                        ext = "<DIR>";
                    return ext;
                }
                case Modified:
                    return mp3->modified();
                case BitRate:
                    return mp3->bitrate();
                case Length:
                    return mp3->length();
                case Track:
                    return mp3->track();
                case Title:
                    return mp3->title();
                case Artist:
                    return mp3->artist();
                case Composer:
                    return mp3->composer();
                case Album:
                    return mp3->album();
                case Disc:
                    return mp3->disc();
                case Date:
                    return mp3->date();
                case Genre:
                    return mp3->genre();
                case HasId3V1:
                    return mp3->hasId3V1();
                case File:
                    return mp3->raw();
                case Pics: {
                    if (mp3->has_pics()) {
                        QString tip;
                        for (int i = 0; i < mp3->pics_count(); i++)
                            tip.append("<img>");
                        return tip;
                    }
                }
                case Path:
                    return mp3->raw();
            }
            break;
        }
        case Qt::EditRole:
        case Qt::DisplayRole: {
            switch (index.column()) {
                case File:
                    return mp3->filename();
                case Ext: {
                    QFileInfo file(mp3->path());
                    QString ext = file.suffix();
                    if (!mp3->filename().isEmpty() && file.isDir() && !dirty_indexes_.contains(index))
                        ext = "<DIR>";
                    return ext;
                }
                case Modified:
                    return mp3->modified();
                case BitRate:
                    if (role == Qt::DisplayRole)
                        return tryToInt(mp3->bitrate());
                    return mp3->bitrate();
                case Length:
                    return mp3->length();
                case Track:
                    if (role == Qt::DisplayRole)
	                    return tryToInt(mp3->track());
                    return mp3->track();
                case Title:
                    return mp3->title();
                case Artist:
                    return mp3->artist();
                case Composer:
                    return mp3->composer();
                case Album:
                    return mp3->album();
                case Disc:
                    if (role == Qt::DisplayRole)
	                    return tryToInt(mp3->disc());
                    return mp3->disc();
                case Date:
                    return mp3->date();
                case Genre:
                    return mp3->genre();
                case Pics:
                    return mp3->pics_count() == 0 ? QString() : QVariant(mp3->pics_count());
                case HasId3V1:
                    return QVariant(mp3->hasId3V1());
                case Path:
                    return mp3->path();

                default:
                    break;
            }
            break;
        }
    }

    return QVariant();
}

bool TagsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid())
        return false;

    Mp3 *mp3 = mp3s_.at(index.row());

    if (role == Qt::EditRole) {
        switch (index.column()) {
            case Path: {
                QString old_ext = QFileInfo(mp3->path()).suffix();
                if (mp3->set_path(value.toString())) {
                    dirty_indexes_.append(index);
                    dirty_indexes_.append(this->index(index.row(), File));
                    emit dataChanged(index, this->index(index.row(), File));
                } else {
                    dirty_indexes_.removeAll(index);
                    dirty_indexes_.removeAll(this->index(index.row(), File));
                }

                if (old_ext != QFileInfo(value.toString()).suffix()) {
                    dirty_indexes_.append(this->index(index.row(), Ext));
                    emit dataChanged(index, this->index(index.row(), Ext));
                }
                break;
            }
            case File: {
                QString old_ext = QFileInfo(mp3->path()).suffix();
                if (mp3->set_filename(value.toString())) {
                    dirty_indexes_.append(index);
                    dirty_indexes_.append(this->index(index.row(), Path));
                    emit dataChanged(index, this->index(index.row(), Path));
                } else {
                    dirty_indexes_.removeAll(index);
                    dirty_indexes_.removeAll(this->index(index.row(), Path));
                }

                if (old_ext != QFileInfo(value.toString()).suffix()) {
                    dirty_indexes_.append(this->index(index.row(), Ext));
                    emit dataChanged(index, this->index(index.row(), Ext));
                }
                break;
            }
            case Track:
                if (mp3->set_track(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Title:
                if (mp3->set_title(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Artist:
                if (mp3->set_artist(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Composer:
                if (mp3->set_composer(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Album:
                if (mp3->set_album(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Disc:
                if (mp3->set_disc(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Date:
                if (mp3->set_date(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Genre:
                if (mp3->set_genre(value.toString()))
                    dirty_indexes_.append(index);
                break;
            case Pics:
                dirty_indexes_.append(index);
                break;
            case HasId3V1:
                if (mp3->set_hasId3V1(value.toBool()))
                    dirty_indexes_.append(index);
                break;
            default:
                break;
        }

        bool dirty = false;
        for (int i = 0; !dirty && i < columnCount(); i++) {
            if (dirty_indexes_.contains(this->index(index.row(), i)))
                dirty = true;
        }

        if (!dirty)
            mp3->parse();
    }

    emit dataChanged(index, index);
    emit headerDataChanged(Qt::Vertical, index.row(), index.row());
    return true;
}

void TagsModel::go(QString path, QString filter, bool recurse, bool show_dirs) {
    QDir dir(path);
    dir.setFilter(QDir::Files | QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot);
    if (show_dirs)
        dir.setFilter(dir.filter() | QDir::Dirs);

    beginResetModel();
    while (mp3s_.count() > 0) {
        delete mp3s_.at(0);
        mp3s_.removeAt(0);
    }
    endResetModel();

    dirty_indexes_.clear();
    stop_requested_ = false;

    int i = 0;
    QDirIterator it(dir, recurse ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (it.hasNext() && !stop_requested_) {
        it.next();
        QFileInfo file = it.fileInfo();
        if ((show_dirs && file.isDir()) || QRegExp(filter, Qt::CaseInsensitive, QRegExp::Wildcard).exactMatch(file.fileName())) {

            beginInsertRows(QModelIndex(), i, i);
            mp3s_.append(new Mp3(file.absoluteFilePath(), this));
            endInsertRows();

            if (i % 30 == 0) {
                QCoreApplication::processEvents();
            }
            i++;
        }
    }

    stop_requested_ = false;
}

void TagsModel::stop() {
    stop_requested_ = true;
}

int TagsModel::save() {
    int count = 0;
    for (int i = 0; i < mp3s_.count(); i++) {
        if (mp3s_.at(i)->save())
            count++;
    }

    for (int i = 0; i < dirty_indexes_.count(); i++) {
        emit dataChanged(dirty_indexes_.at(i), dirty_indexes_.at(i));
        emit headerDataChanged(Qt::Vertical, dirty_indexes_.at(i).row(), dirty_indexes_.at(i).row());
    }
    dirty_indexes_.clear();
    return count;
}

void TagsModel::undoChanges() {
    for (int i = 0; i < dirty_indexes_.count(); i++) {
        Mp3 *mp3 = mp3s_.at(dirty_indexes_.at(i).row());
        mp3->parse();
        emit dataChanged(dirty_indexes_.at(i), dirty_indexes_.at(i));
        emit headerDataChanged(Qt::Vertical, dirty_indexes_.at(i).row(), dirty_indexes_.at(i).row());
    }
    dirty_indexes_.clear();
}

QVariant TagsModel::tryToInt(QString input) {
    bool ok = false;
    int num = input.toInt(&ok);
    if (ok)
        return QVariant(num);
    return input;
}


}
