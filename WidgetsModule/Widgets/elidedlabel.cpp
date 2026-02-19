/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "elidedlabel.h"

#include <QPainter>
#include <QTextLayout>
#include <QDebug>
#include <WidgetsModule/Utils/widgethelpers.h>

//! [0]
ElidedLabel::ElidedLabel(QWidget *parent)
    : Super(parent)
    , m_alignment(Qt::AlignLeft | Qt::AlignVCenter)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ElidedLabel::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
    update();
}
//! [0]

//! [1]
void ElidedLabel::setText(const QString &newText)
{
    content = newText;
    if(WidgetWrapper(this).WidgetToolTip()->Native().isEmpty()){
        setToolTip(content);
    }
    update();
}
//! [1]

//! [2]
void ElidedLabel::paintEvent(QPaintEvent *event)
{
    Super::paintEvent(event);

    QPainter painter(this);
    QFontMetrics fontMetrics = painter.fontMetrics();

    int lineSpacing = fontMetrics.lineSpacing();
    int y = 0;
//    painter.setBrush(Qt::red);
//    painter.drawRect(rect());

    auto currentSize = size();
    QStyleOption opt;
    auto sizeWithMargins = style()->sizeFromContents(QStyle::CT_PushButton, &opt, currentSize, this);
    ///PM_DockWidgetTitleBarButtonMargin to get top margin
    auto leftMargin = style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, &opt, this);
    auto rightMargin = sizeWithMargins.width() - currentSize.width() - leftMargin;

    if(m_alignment != (Qt::AlignLeft | Qt::AlignVCenter)) {
        painter.drawText(rect().adjusted(leftMargin, 0, -rightMargin, 0), content, QTextOption(m_alignment));
    } else {
        auto w = currentSize.width() - leftMargin - rightMargin;

        QTextLayout textLayout(content, painter.font());
        textLayout.beginLayout();
        QVector<QTextLine> textLines;
        forever {
            QTextLine line = textLayout.createLine();

            if (!line.isValid())
                break;

            line.setLineWidth(w);
            textLines.append(line);

            int nextLineY = y + lineSpacing;

            if (height() >= nextLineY + lineSpacing) {
                y = nextLineY;
            } else {
                y = nextLineY;
                break;
            }
        }
        textLayout.endLayout();
        auto diff = rect().height() - y;
        if(diff > 0) {
            y = diff / 2 - fontMetrics.height() / 4;
        } else {
            y = 0;
        }
        for(const QTextLine& line : textLines) {
            int nextLineY = y + lineSpacing;
            y = nextLineY;
            QString lastLine = content.mid(line.textStart(), line.textLength());
            QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width());
            painter.drawText(QPointF(leftMargin, y), elidedLastLine);
        }
    }
}

QSize ElidedLabel::minimumSizeHint() const
{
    if(text().isEmpty()) {
        return QSize();
    }
    auto fm = fontMetrics();
    return QSize(fm.width("..."), fm.height());
}
//! [4]
