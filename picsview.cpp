#include "picsview.h"

#include <QMouseEvent>
#include <QIcon>

namespace bruntagger {

PicsView::PicsView(QWidget *parent) : QListView(parent) {
    action_row_ = -1;
}

PicsView::~PicsView() {
}

void PicsView::mousePressEvent(QMouseEvent *event) {
    QListView::mousePressEvent(event);
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        action_row_ = index.row();
        if (event->button() == Qt::LeftButton)
            emit drawMe();
    }
}

void PicsView::mouseReleaseEvent(QMouseEvent *event) {
    QListView::mouseReleaseEvent(event);
    if (action_row_ != -1 && event->button() == Qt::LeftButton) {
        emit unDrawMe();
        action_row_ = -1;
    }
}

void PicsView::dragMoveEvent(QDragMoveEvent *event) {
    QListView::dragMoveEvent(event);
    if (action_row_ != -1) {
        emit unDrawMe();
        action_row_ = -1;
    }
}

void PicsView::dragLeaveEvent(QDragLeaveEvent *event) {
    QListView::dragLeaveEvent(event);
    if (action_row_ != -1) {
        emit unDrawMe();
        action_row_ = -1;
    }
}


}
