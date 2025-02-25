#include "mp3.h"

#include <taglib/tag.h>
#include <taglib/tbytevector.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v1tag.h>
#include <taglib/apetag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/tlist.h>
#include <taglib/textidentificationframe.h>
#include <taglib/taglib.h>
#include <taglib/xingheader.h>

#include <kmimetype.h>

#include <QFileInfo>
#include <QDebug>
#include <QDir>

namespace bruntagger {

Mp3::Mp3(const QString &path, QObject *parent) : QObject(parent) {
    path_ = path;
    filename_ = QFileInfo(path).fileName();
    original_path_ = path_;
    original_filename_ = filename_;
    hasId3V1_ = false;
    pics_count_ = 0;
    dirty_ = false;    

    unsafe_file_chars_ = QRegExp("[:*?\"<>|]");

    if (isMp3()) {
        parse();
    }
}

Mp3::~Mp3() {
}

void Mp3::reset() {
    track_.clear();
    title_.clear();
    artist_.clear();
    composer_.clear();
    album_.clear();
    date_.clear();
    disc_.clear();
    genre_.clear();
    bitrate_.clear();
    raw_.clear();

    path_ = original_path_;
    filename_ = original_filename_;

    added_pic_filenames_.clear();
    removed_pic_indexes_.clear();

    pic_mimes_.clear();
    added_pic_filenames_.clear();
    removed_pic_indexes_.clear();
    hasId3V1_ = false;
    pics_count_ = 0;
    dirty_ = false;
}

void Mp3::parse() {
    TagLib::MPEG::File mpeg_file(original_path_.toUtf8());
    TagLib::ID3v2::Tag *id3v2_tag = mpeg_file.ID3v2Tag();
    TagLib::ID3v1::Tag *id3v1_tag = mpeg_file.ID3v1Tag();
    TagLib::APE::Tag *ape_tag = mpeg_file.APETag();

    QString *data = new QString();
    QTextStream stream(data, QIODevice::ReadOnly);

    reset();

    stream << original_path_ << "\n";

    modified_ = QFileInfo(original_path_).lastModified();

    bitrate_ = mpeg_file.audioProperties()->bitrate() == 0 ? QString() : QString::number(mpeg_file.audioProperties()->bitrate());

    if (mpeg_file.audioProperties()->xingHeader() != NULL && mpeg_file.audioProperties()->xingHeader()->isValid())
        bitrate_.prepend("(vbr) ");

    stream << tr("bitrate: ") << bitrate_ << '\n';


    int secs = mpeg_file.audioProperties()->length();

    length_ = QTime(0, 0).addSecs(secs);

    stream << tr("length: ") << (length_ >= QTime(1, 0) ? length_.toString("H:mm:ss") : length_.toString("m:ss")) << '\n';


    stream << "\nAPE\n";
    if (ape_tag && !ape_tag->isEmpty()) {
        for (TagLib::APE::ItemListMap::ConstIterator it = ape_tag->itemListMap().begin(); it
                != ape_tag->itemListMap().end(); ++it) {
            stream << "- " << (*it).first.toCString() << ": " << (*it).second.values().toString().toCString() << '\n';
        }
    } else
        stream << "(none)\n";


    stream << "\nID3v1\n";
    if (id3v1_tag && !id3v1_tag->isEmpty()) {
        stream << "- title: " << id3v1_tag->title().toCString() << '\n' << "- artist: "
                << id3v1_tag->artist().toCString() << '\n' << "- album: " << id3v1_tag->album().toCString() << '\n'
                << "- date: " << id3v1_tag->year() << '\n' << "- comment: " << id3v1_tag->comment().toCString() << '\n'
                << "- track: " << id3v1_tag->track() << '\n' << "- genre: " << id3v1_tag->genre().toCString() << '\n';
        hasId3V1_ = true;
    } else {
        stream << "(none)\n";
        hasId3V1_ = false;
    }


    stream << "\nID3v2\n";
    if (id3v2_tag && !id3v2_tag->isEmpty()) {
        for (TagLib::ID3v2::FrameList::ConstIterator it = id3v2_tag->frameList().begin(); it
                != id3v2_tag->frameList().end(); ++it) {
            QString frame_id = TagLib::String((*it)->frameID()).toCString();

            if (frame_id == "TRCK")
                track_ = (*it)->toString().toCString();
            else if (frame_id == "TIT2")
                title_ = (*it)->toString().toCString();
            else if (frame_id == "TCOM")
                composer_ = (*it)->toString().toCString();
            else if (frame_id == "TCON")
                genre_ = (*it)->toString().toCString();
            else if (frame_id == "TPE1")
                artist_ = (*it)->toString().toCString();
            else if (frame_id == "TALB")
                album_ = (*it)->toString().toCString();
            else if (frame_id == "TDRC")
                date_ = (*it)->toString().toCString();
            if (frame_id == "TPOS")
                disc_ = (*it)->toString().toCString();

            else if (frame_id == "APIC") {
                TagLib::ID3v2::AttachedPictureFrame *apic = static_cast<TagLib::ID3v2::AttachedPictureFrame *> (*it);

                QString mime = apic->mimeType().toCString();
                pic_mimes_ << mime;

                pics_count_++;
            }

            stream << "- " << frame_id << ": " << QString((*it)->toString().toCString());
            if (*it != id3v2_tag->frameList().back())
                stream << '\n';
        }
    } else
        stream << "(none)";

    raw_ = *data;
    delete data;
}

void Mp3::setTextFrame(TagLib::ID3v2::Tag *id3v2_tag, QString frame_id, QString &text) {
    id3v2_tag->removeFrames(frame_id.toLatin1().data());
    if (!text.isEmpty()) {
        TagLib::ID3v2::TextIdentificationFrame *frame
            = new TagLib::ID3v2::TextIdentificationFrame(frame_id.toLatin1().data());
        frame->setTextEncoding(TagLib::String::Latin1);
        frame->setText(text.toLatin1().data());
        id3v2_tag->addFrame(frame);
    }
}

const QList<QImage> Mp3::pics() const {
    QList<QImage> pics;

    if (isMp3()) {
        TagLib::MPEG::File mpeg_file(original_path_.toUtf8());
        TagLib::ID3v2::Tag *id3v2_tag = mpeg_file.ID3v2Tag();

        int pic_index = 0;
        for (TagLib::ID3v2::FrameList::ConstIterator it = id3v2_tag->frameList().begin(); it != id3v2_tag->frameList().end(); ++it) {
            QString frame_id = TagLib::String((*it)->frameID()).toCString();
            if (frame_id == "APIC") {
                if (!removed_pic_indexes_.contains(pic_index)) {
                    QImage img;
                    TagLib::ID3v2::AttachedPictureFrame *apic = static_cast<TagLib::ID3v2::AttachedPictureFrame *> (*it);

                    img.loadFromData((const unsigned char*) (apic->picture().data()), apic->picture().size());
                    pics.append(img);
                }
                pic_index++;
            }
        }

        for (int i = 0; i < added_pic_filenames_.count(); i++) {
            if (!removed_pic_indexes_.contains(pic_index)) {
                QImage img(added_pic_filenames_.at(i));
                pics.append(img);
            }
            pic_index++;
        }
    }

    return pics;
}

bool Mp3::has_pics() const {
    return pics_count_ - added_pic_filenames_.count() + removed_pic_indexes_.count() > 0;
}

bool Mp3::isMp3() const {
    QFileInfo file(original_path_);
    return file.suffix().toLower() == "mp3";
}

bool Mp3::set_path(QString path) {
    if (!path.startsWith(QDir::separator())) {
        path = QDir::current().canonicalPath() + QDir::separator() + path;
    }
    path = path.replace(unsafe_file_chars_, "_");
    path_ = path;
    filename_ = QFileInfo(path).fileName();

    if (path == original_path_)
        return false;
    dirty_ = true;
    return true;
}

bool Mp3::set_filename(QString filename) {
    filename = filename.replace(unsafe_file_chars_, "_");
    filename_ = filename;
    path_ = QFileInfo(path_).canonicalPath() + "/" + filename;
    path_ = path_.replace(unsafe_file_chars_, "_");

    if (filename == original_filename_)
        return false;
    dirty_ = true;
    return true;
}

bool Mp3::set_track(QString track) {
    if (track_ == track)
        return false;
    track_ = track;
    dirty_ = true;
    return true;
}

bool Mp3::set_title(QString title) {
    if (title_ == title)
        return false;
    title_ = title;
    dirty_ = true;
    return true;
}

bool Mp3::set_artist(QString artist) {
    if (artist_ == artist)
        return false;
    artist_ = artist;
    dirty_ = true;
    return true;
}

bool Mp3::set_composer(QString composer) {
    if (composer_ == composer)
        return false;
    composer_ = composer;
    dirty_ = true;
    return true;
}

bool Mp3::set_album(QString album) {
    if (album_ == album)
        return false;
    album_ = album;
    dirty_ = true;
    return true;
}

bool Mp3::set_date(QString date) {
    if (date_ == date)
        return false;
    date_ = date;
    dirty_ = true;
    return true;
}

bool Mp3::set_disc(QString disc) {
    if (disc_ == disc)
        return false;
    disc_ = disc;
    dirty_ = true;
    return true;
}

bool Mp3::set_genre(QString genre) {
    if (genre_ == genre)
        return false;
    genre_ = genre;
    dirty_ = true;
    return true;
}

bool Mp3::set_hasId3V1(bool hasId3V1) {
    if (hasId3V1_ == hasId3V1)
        return false;
    hasId3V1_ = hasId3V1;
    dirty_ = true;
    return true;
}

bool Mp3::save() {
    bool saved = false;

    if (QFileInfo(original_path_).exists() && isMp3() && dirty_) {
        TagLib::MPEG::File mpeg_file(original_path_.toUtf8());
        TagLib::ID3v2::Tag *id3v2_tag = mpeg_file.ID3v2Tag(true);
        setTextFrame(id3v2_tag, "TRCK", track_);
        setTextFrame(id3v2_tag, "TIT2", title_);
        setTextFrame(id3v2_tag, "TCOM", composer_);
        setTextFrame(id3v2_tag, "TPE1", artist_);
        setTextFrame(id3v2_tag, "TALB", album_);
        setTextFrame(id3v2_tag, "TDRC", date_);
        setTextFrame(id3v2_tag, "TPOS", disc_);
        setTextFrame(id3v2_tag, "TCON", genre_);


        TagLib::ID3v2::FrameList remove_frames;
        foreach (int i, removed_pic_indexes_)
            remove_frames.append(id3v2_tag->frameListMap()["APIC"][i]);
        foreach (int i, removed_pic_indexes_)
            id3v2_tag->removeFrame(remove_frames[i]);

        foreach (QString filename, added_pic_filenames_) {
            TagLib::ID3v2::AttachedPictureFrame *apic = new TagLib::ID3v2::AttachedPictureFrame();
            QFile file(filename);
            file.open(QIODevice::ReadOnly);

            QByteArray data = file.readAll();

            KMimeType::Ptr kmime = KMimeType::findByContent(data);
            apic->setMimeType(TagLib::String(kmime.data()->name().toStdString()));
            apic->setPicture(TagLib::ByteVector(data.data(), data.size()));
            id3v2_tag->addFrame(apic);
        }

        mpeg_file.save(TagLib::MPEG::File::ID3v2, false);

        if (!hasId3V1_) {
            mpeg_file.strip(TagLib::MPEG::File::ID3v1);
            mpeg_file.save(TagLib::MPEG::File::ID3v2, false);
        }

        if (path_ != original_path_) {
            QFileInfo path(path_);
            if (!path.absoluteDir().exists()) {
                QString dir_name = path.absolutePath();
                QDir(dir_name).mkpath(dir_name);
            }
            if (QFile::rename(original_path_, path_)) {
                original_path_ = path_;
                original_filename_ = filename_;
            }
        }

        saved = true;
        parse();
    }

    return saved;
}

bool Mp3::extractPic(int index, QString &pic_filepath) {
    bool success = false;

    if (isMp3()) {
        TagLib::MPEG::File mpeg_file(original_path_.toUtf8());
        TagLib::ID3v2::Tag *id3v2_tag = mpeg_file.ID3v2Tag();

        int i = 0;
        bool stop = false;
        for (TagLib::ID3v2::FrameList::ConstIterator it = id3v2_tag->frameList().begin(); !stop && it != id3v2_tag->frameList().end(); ++it) {
            QString frame_id = TagLib::String((*it)->frameID()).toCString();
            if (frame_id == "APIC") {
                QImage img;
                TagLib::ID3v2::AttachedPictureFrame *apic = static_cast<TagLib::ID3v2::AttachedPictureFrame *> (*it);

                if (i == index) {
                    QFile file(pic_filepath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write((const char*)apic->picture().data(), apic->picture().size());
                        file.flush();
                        file.close();
                        success = true;
                        qDebug() << pic_filepath;
                    }

                    stop = true;
                }
                i++;
            }
        }
    }

    return success;
}

void Mp3::addPics(QStringList &filenames) {
    added_pic_filenames_.append(filenames);

    for (int i = 0; i < filenames.count(); i++) {
        pic_mimes_ << KMimeType::findByFileContent(filenames.at(i)).data()->name();
    }

    pics_count_ += filenames.count();
    dirty_ = true;
}

void Mp3::removePics() {
    if (pics_count_ > 0) {
        added_pic_filenames_.clear();
        removed_pic_indexes_.clear();
        pic_mimes_.clear();
        for (int i = 0; i < pics_count_; i++)
            removed_pic_indexes_ << i;
        pics_count_ = 0;
        dirty_ = true;
    }
}

void Mp3::removePic(int index) {
    removed_pic_indexes_ << index;
    pics_count_--;
    dirty_ = true;
}

void Mp3::removeId3V1() {
    hasId3V1_ = false;

    dirty_ = true;
}

}
