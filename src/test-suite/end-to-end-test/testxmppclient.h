/*
 * Copyright (C) 2012 Andres Pagliano, Gabriel Miretti, Gonzalo Buteler,
 * Nestor Bustamante, Pablo Perez de Angelis
 *
 * This file is part of LVK Botmaster.
 *
 * LVK Botmaster is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LVK Botmaster is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LVK Botmaster.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LVK_TESTXMPPCLIENT_H
#define LVK_TESTXMPPCLIENT_H

#include <QString>

class TestXmppClient
{
public:
    TestXmppClient();

    ~TestXmppClient();

    bool connect(const QString &jid, const QString &passwd);

    void disconnect();

    void sendMessage(const QString &to, const QString &msg);

    bool waitForResponse(int msecs, QString &response);
};

#endif // LVK_TESTXMPPCLIENT_H