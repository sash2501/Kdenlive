/***************************************************************************
 *   Copyright (C) 2011 by Jean-Baptiste Mardelle (jb@kdenlive.org)        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include "abstractmonitor.h"
#include "monitormanager.h"

#include "kdenlivesettings.h"

#include <QDebug>

#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QMouseEvent>

AbstractMonitor::AbstractMonitor(Kdenlive::MonitorId id, MonitorManager *manager, QWidget *parent): 
    QWidget(parent),
    videoSurface(NULL),
    m_id(id),
    m_monitorManager(manager)
{
    videoBox = new VideoContainer(this);
}


AbstractMonitor::~AbstractMonitor()
{
    delete videoSurface;
}

void AbstractMonitor::createVideoSurface()
{
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setContentsMargins(0, 0, 0, 0);
    videoSurface = new VideoSurface;
    lay->addWidget(videoSurface);
    videoBox->setLayout(lay);
}

bool AbstractMonitor::isActive() const
{
    return m_monitorManager->isActive(m_id);
}

bool AbstractMonitor::slotActivateMonitor(bool forceRefresh)
{
    return m_monitorManager->activateMonitor(m_id, forceRefresh);
}

VideoContainer::VideoContainer(AbstractMonitor* monitor, QWidget *parent) :
    QFrame(parent)
    , m_monitor(monitor)
{
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::ClickFocus);
    //setEnabled(false);
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

// virtual
void VideoContainer::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() != Qt::RightButton) {
        if (m_monitor->isActive()) {
            m_monitor->slotPlay();
        }
    }
}

// virtual
void VideoContainer::keyPressEvent(QKeyEvent *event)
{
    // Exit fullscreen with Esc key
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        switchFullScreen();
        event->setAccepted(true);
    } else event->setAccepted(false);
}

// virtual
void VideoContainer::wheelEvent(QWheelEvent * event)
{
    m_monitor->slotMouseSeek(event->delta(), event->modifiers() == Qt::ControlModifier);
    event->accept();
}

void VideoContainer::mouseDoubleClickEvent(QMouseEvent * event)
{
    if (!KdenliveSettings::openglmonitors())
        switchFullScreen();
    event->accept();
}

void VideoContainer::switchFullScreen()
{
    // TODO: disable screensaver?
    Qt::WindowFlags flags = windowFlags();
    if (!isFullScreen()) {
        // Check if we ahave a multiple monitor setup
        setUpdatesEnabled(false);
        int monitors = QApplication::desktop()->screenCount();
        if (monitors > 1) {
            QRect screenres;
            // Move monitor widget to the second screen (one screen for Kdenlive, the other one for the Monitor widget
            int currentScreen = QApplication::desktop()->screenNumber(this);
            if (currentScreen < monitors - 1)
                screenres = QApplication::desktop()->screenGeometry(currentScreen + 1);
            else
                screenres = QApplication::desktop()->screenGeometry(currentScreen - 1);
            move(QPoint(screenres.x(), screenres.y()));
            resize(screenres.width(), screenres.height());
        }

        m_baseFlags = flags & (Qt::Window | Qt::SubWindow);
        flags |= Qt::Window;
        flags ^= Qt::SubWindow;
        setWindowFlags(flags);
#ifdef Q_WS_X11
        // This works around a bug with Compiz
        // as the window must be visible before we can set the state
        show();
        raise();
        setWindowState(windowState() | Qt::WindowFullScreen);   // set
#else
        setWindowState(windowState() | Qt::WindowFullScreen);   // set
        setUpdatesEnabled(true);
        show();
#endif
        setEnabled(true);
    } else {
        setUpdatesEnabled(false);
        flags ^= (Qt::Window | Qt::SubWindow); //clear the flags...
        flags |= m_baseFlags; //then we reset the flags (window and subwindow)
        setWindowFlags(flags);
        setWindowState(windowState()  ^ Qt::WindowFullScreen);   // reset
        setUpdatesEnabled(true);
        setEnabled(false);
        show();
    }
}



