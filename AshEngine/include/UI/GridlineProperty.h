#pragma once

#include <Gridline.h>
#include <Vector3DEditSlider.h>

class GridlineProperty: public QWidget {
    Q_OBJECT

public:
    GridlineProperty(Gridline* gridline, QWidget * parent = 0);

private:
    Gridline *m_host;
    Vector3DEdit *m_xargsEdit, *m_yargsEdit, *m_zargsEdit;
    Vector3DEditSlider *m_colorEditSlider;

    void configLayout();
    void configSignals();

private slots:
    void hostDestroyed(QObject* host);
};
