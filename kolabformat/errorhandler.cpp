/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "errorhandler.h"

#include <qdebug.h>
#include <QTime>
#include <QStringList>
#include <qmutex.h>
#include <iostream>

#include <kolabformat.h>

namespace Kolab {

QMutex mutex;
    
void logMessage(const QString &message, const QString &file, int line, ErrorHandler::Severity s)
{
    ErrorHandler::instance().addError(s, message, file+" "+QString::number(line));
}


void ErrorHandler::addError(ErrorHandler::Severity s, const QString& message, const QString &location)
{
    QMutexLocker locker(&mutex);
    QString filename = location;
    if (!filename.split(QLatin1Char('/')).isEmpty()) {
       filename = filename.split(QLatin1Char('/')).last();
    }
    const QString output = QTime::currentTime().toString(QLatin1String("(hh:mm:ss) ")) + filename + QLatin1String(":\t") + message;
    std::cout << output.toStdString() << std::endl;
    if (s == Debug) {
        return;
    }
    if (s > m_worstError) {
        m_worstError = s;
        m_worstErrorMessage = message;
    }
    m_errorQueue.append(Err(s, message, location));
}

ErrorHandler::Severity ErrorHandler::error() const
{
    QMutexLocker locker(&mutex);
    return m_worstError;
}

QString ErrorHandler::errorMessage() const
{
    QMutexLocker locker(&mutex);
    return m_worstErrorMessage;
}

const QList< ErrorHandler::Err >& ErrorHandler::getErrors() const
{
    QMutexLocker locker(&mutex);
    return m_errorQueue;
}

void ErrorHandler::clear()
{
    QMutexLocker locker(&mutex);
    m_errorQueue.clear();
    m_worstError = Debug;
}

void ErrorHandler::handleLibkolabxmlErrors()
{
    switch (Kolab::error()) {
        case Kolab::Warning:
            instance().addError(ErrorHandler::Warning, QString::fromStdString(Kolab::errorMessage()), "libkolabxml");
            break;
        case Kolab::Error:
            instance().addError(ErrorHandler::Error, QString::fromStdString(Kolab::errorMessage()), "libkolabxml");
            break;
        case Kolab::Critical:
            instance().addError(ErrorHandler::Critical, QString::fromStdString(Kolab::errorMessage()), "libkolabxml");
            break;
        default:
            //Do nothing, there is no message available in this case
            break;
    }
}


}
