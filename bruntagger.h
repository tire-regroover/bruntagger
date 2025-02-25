#ifndef BRUNTAGGER_H
#define BRUNTAGGER_H

#include "ui_bruntagger.h"
#include "version.h"
#include "tagsmodel.h"

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QSplashScreen>

namespace bruntagger {

class PicsModel;
class Mp3;


class bruntagger : public QMainWindow {
    Q_OBJECT

public:
    bruntagger(QWidget *parent = 0);
    ~bruntagger();
    void populatePicsModel(Mp3 *mp3);
private:
    static const int kStatusTimeout;

    Ui::bruntaggerClass ui;

    bool first_show_;
    bool auto_resize_cols_;

    QString set_same_;
    QString clicked_file_path_;
    int clicked_header_column_;

    QList<QString> prev_dirs_;
    int prev_dirs_index_;

    TagsModel *model_;
    QSortFilterProxyModel *proxy_model_;
    PicsModel *pics_model_;

    QSplashScreen *splash_;

    QList<QAction*> header_actions_;

    QList< QList<QVariant> > presets_;

    void restoreDefaultState();
    void restoreDefaultHeader();
    void addPreset(QString title, QString input, QString find, QString replace, TagsModel::Column col);
    void applyStyle();
    void changeDir(int index);
    void changeDir(QString dir, bool append = true);
    void showMime(const QFileInfo &file);
    void filter();
    void clearFilter();
    void loadSettings();
    void saveSettings();
    static void replaceMacros(QString &str, Mp3 *mp3, int row);

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void changeEvent(QEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    void on_actionResetView_triggered();
    void on_actionAboutQt_triggered();
    void on_actionOpenDir_triggered();
    void on_actionBack_triggered();
    void on_actionForward_triggered();
    void on_actionUpDir_triggered();
    void on_actionChangeDir_triggered();
    void on_actionRefresh_triggered();
    void on_lineFilter_textEdited(const QString &text);
    void on_lineFilter_returnPressed();
    void on_actionFilter_toggled(bool checked);
    void on_comboPresets_currentIndexChanged(int index);
    void on_actionEditPresets_triggered();
    void on_actionClear_triggered();
    void on_actionRegex_triggered();
    void on_actionDiscardChanges_triggered();
    void on_actionSave_triggered();
    void on_actionStop_triggered();
    void hheader_doubleClicked();
    void hheader_sectionResized();
    void hheader_customContextMenuRequested(const QPoint &pos);
    void showColumn();
    void on_actionHideColumn_triggered();
    void selectedRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_tableView_customContextMenuRequested(const QPoint &pos);
    void on_actionLaunch_triggered();
    void on_actionLaunchFolder_triggered();
    void on_actionOpenContaining_triggered();
    void on_actionSetSame_triggered();
    void on_picsView_customContextMenuRequested(const QPoint &pos);
    void on_actionExtractPic_triggered();
    void on_actionAddPics_triggered();
    void on_actionRemovePics_triggered();
    void on_actionRemovePic_triggered();
    void on_actionRemoveId3V1_triggered();
    void showPic();
    void closePic();
    void on_actionAbout_triggered();
};

}
#endif // BRUNTAGGER_H
