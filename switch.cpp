#include "switch.h"

Switch::Switch(QWidget *parent):
    QAbstractButton(parent),
    _switch(false),
    _orientation(false),
    _opacity(0.000),
    _margin(3),
    _thumb("#ff2c26"),
    _anim(new QPropertyAnimation(this, "offset", this))
{
    setMinimumHeight(_orientation ? 12 : 18);
    setMinimumWidth(_orientation ? 18 : 12);
    setCheckable(true);
    setBrush(QColor("#b0d959"));
    connect(this, &Switch::toggled, this, [=](){
        _switch = isChecked();
        _thumb = _switch ? _brush : QBrush("#ff2c26");
        //setOffset(_switch ? (_orientation ? width() - height() + _margin : _margin) : (_orientation ?  _margin : height() - width() + _margin));
        setOffset(_margin + (_switch == _orientation ? abs(width() - height()) : 0));
    });
}

Switch::Switch(const QBrush &brush, QWidget *parent):
    QAbstractButton(parent),
    _switch(false),
    _orientation(false),
    _opacity(0.000),
    _margin(3),
    _thumb("#ff2c26"),
    _anim(new QPropertyAnimation(this, "offset", this))
{
    this->setCheckable(true);
    setBrush(brush);
    if(!_orientation)
    {
        this->setMinimumHeight(18);
        this->setMinimumWidth(12);
    } else {
        this->setMinimumHeight(12);
        this->setMinimumWidth(18);
    }
    connect(this, &Switch::toggled, this, [=](){
        _switch = this->isChecked();
        _thumb = _switch ? _brush : QBrush("#ff2c26");
        setOffset(_switch ? (_orientation ? width() - height() + _margin : _margin) : (_orientation ?  _margin : height() - width() + _margin));
    });
}

void Switch::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(isEnabled() ? (_switch ? brush() : Qt::red) : Qt::gray);
    p.setOpacity(isEnabled() ? (_switch ? 0.5 : 0.38) : 0.12);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(QRect(_margin, _margin,
                            width()  - 2 * _margin,
                            height() - 2 * _margin),
                            ((_orientation ? height() : width()) - 2 * _margin)/2,
                            ((_orientation ? height() : width()) - 2 * _margin)/2);
    p.setOpacity(1.0);
    if (isEnabled()) p.setBrush(_thumb);
    p.drawEllipse(QRectF( _orientation ? offset() : _x ,
                          _orientation ? _y : offset(),
                         (_orientation ? height() : width()) - 2 * _margin,
                         (_orientation ? height() : width()) - 2 * _margin));
    e->accept();
}

void Switch::mouseReleaseEvent(QMouseEvent *e) {
    if (_switchOnClick) {
        if (e->button() & Qt::LeftButton) {
            swipe();
        }
        QAbstractButton::mouseReleaseEvent(e);
        emit switch_changed();
    } else if (e->button() & Qt::LeftButton) {
        emit clicked(isChecked());
    }
}

void Switch::swipe() {
    _switch = !_switch;
    _thumb = _switch ? _brush : QBrush("#ff2c26");
    if (_switch) {
        _anim->setStartValue(offset());
        setOffset(_orientation ? width() - height() + _margin :  _margin);
        _anim->setEndValue(_orientation ? width() - height() + _margin :  _margin);
    } else {
        _anim->setStartValue(_orientation ? width() - height() + _margin :  _margin);
        setOffset(_orientation ? _margin : height() - width() + _margin);
        _anim->setEndValue(offset());
    }
    _anim->setDuration(200);
    _anim->start();
    if(!this->getSwitchOnClick()) this->setChecked(_switch);
}

void Switch::enterEvent(QEvent *e) {
    setCursor(Qt::PointingHandCursor);
    QAbstractButton::enterEvent(e);
}

QSize Switch::sizeHint() const {return QSize(height() + 2 * _margin, 2 * (height() + _margin));}
