#ifndef MP3_H_
#define MP3_H_

#include <QString>
#include <QList>
#include <QStringList>
#include <QImage>
#include <QTime>
#include <QRegExp>

namespace TagLib {
namespace ID3v2 {
class Tag;
}
}

namespace bruntagger {

class Mp3 : QObject {
    Q_OBJECT

private:
    QString original_path_;
    QString original_filename_;
    QString path_;
    QString filename_;
    QDateTime modified_;
    QString track_;
    QString title_;
    QString artist_;
    QString composer_;
    QString album_;
    QString date_;
    QString disc_;
    QString genre_;
    QString bitrate_;
    QTime length_;
    bool hasId3V1_;
    QString raw_;
    QStringList pic_mimes_;
    int pics_count_;
    bool dirty_;

    QRegExp unsafe_file_chars_;

    QStringList added_pic_filenames_;
    QList<int> removed_pic_indexes_;

    void setTextFrame(TagLib::ID3v2::Tag *id3v2_tag, QString frame_id, QString &text);
    void reset();

public:
    Mp3(const QString &path, QObject *parent = 0);
    virtual ~Mp3();

    void parse();

    const QString & original_path() const { return original_path_; }
    const QString & path() const { return path_; }
    const QString & filename() const { return filename_; }
    const QDateTime & modified() const { return modified_; }
    const QString & track() const { return track_; }
    const QString & title() const { return title_; }
    const QString & artist() const { return artist_; }
    const QString & composer() const { return composer_; }
    const QString & album() const { return album_; }
    const QString & date() const { return date_; }
    const QString & disc() const { return disc_; }
    const QString & genre() const { return genre_; }
    const QString & bitrate() const { return bitrate_; }
    const QTime & length() const { return length_; }
    const bool & hasId3V1() const { return hasId3V1_; }
    const QString & raw() const { return raw_; }
    const QStringList & pic_mimes() const { return pic_mimes_; }
    int pics_count() const { return pics_count_; }
    bool is_dirty() const { return dirty_; }

    const QList<QImage> pics() const;
    bool has_pics() const;
    bool isMp3() const;

    bool set_path(QString path);
    bool set_filename(QString filename);
    bool set_track(QString track);
    bool set_title(QString title);
    bool set_artist(QString artist);
    bool set_composer(QString composer);
    bool set_album(QString album);
    bool set_date(QString date);
    bool set_genre(QString genre);
    bool set_disc(QString disc);

    bool set_hasId3V1(bool hasId3V1);

    bool save();
    bool extractPic(int index, QString &pic_filepath);

    void addPics(QStringList &filenames);
    void removePics();
    void removePic(int index);

    void removeId3V1();
};

}

#endif /* MP3_H_ */
