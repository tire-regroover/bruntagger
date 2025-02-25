#ifndef PICSVIEW_H_
#define PICSVIEW_H_

#include <QListView>

namespace bruntagger {

class PicsView: public QListView {
    Q_OBJECT

public:
    PicsView(QWidget *parent = 0);
    virtual ~PicsView();

private:
    int action_row_;

public:
    int action_row() const { return action_row_; }

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);

signals:
    void drawMe();
    void unDrawMe();
};

}

#endif /* PICSVIEW_H_ */
