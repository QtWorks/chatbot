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

#include "simpleaimlengine.h"
#include "nlprule.h"

#include <exception>

//--------------------------------------------------------------------------------------------------
// SimpleAimlEngine::InvalidSyntaxException
//--------------------------------------------------------------------------------------------------

class Lvk::Nlp::SimpleAimlEngine::InvalidSyntaxException : public std::exception
{
public:
    InvalidSyntaxException(const QString &what) throw()
        : m_what(what.toUtf8())
    {
    }

    ~InvalidSyntaxException() throw() {}

    virtual const char* what() throw()
    {
        return m_what.constData();
    }

private:
    QByteArray m_what;
};

//--------------------------------------------------------------------------------------------------
// SimpleAimlEngine
//--------------------------------------------------------------------------------------------------

#define VAR_NAME_REGEX  "\\[([A-Za-z_]+)\\]"
#define IF_REGEX        "\\{\\s*if\\s*" VAR_NAME_REGEX "\\s*=\\s*([^}]+)\\}" "([^{]+)"
#define ELSE_REGEX      "\\{\\s*else\\s*\\}(.+)"
#define IF_ELSE_REGEX   IF_REGEX ELSE_REGEX


Lvk::Nlp::SimpleAimlEngine::SimpleAimlEngine()
    : AimlEngine()
{
    initRegexs();
}

//--------------------------------------------------------------------------------------------------

Lvk::Nlp::SimpleAimlEngine::SimpleAimlEngine(Sanitizer *sanitizer)
    : AimlEngine(sanitizer)
{
    initRegexs();
}

//--------------------------------------------------------------------------------------------------

void Lvk::Nlp::SimpleAimlEngine::initRegexs()
{
    QString localizedIfRegex = QString(IF_REGEX)
            .replace("if", QObject::tr("if"));

    QString localizedIfElseRegex  = QString(IF_ELSE_REGEX)
            .replace("if", QObject::tr("if"))
            .replace("else", QObject::tr("else"));

    m_varNameRegex = QRegExp(VAR_NAME_REGEX);
    m_ifRegex = QRegExp(localizedIfRegex);
    m_ifElseRegex = QRegExp(localizedIfElseRegex);

    m_varNameRegex.setCaseSensitive(false);
    m_ifRegex.setCaseSensitive(false);
    m_ifElseRegex.setCaseSensitive(false);
}

//--------------------------------------------------------------------------------------------------

const Lvk::Nlp::RuleList & Lvk::Nlp::SimpleAimlEngine::rules() const
{
    return m_rules;
}

//--------------------------------------------------------------------------------------------------

Lvk::Nlp::RuleList & Lvk::Nlp::SimpleAimlEngine::rules()
{
    return m_rules;
}

//--------------------------------------------------------------------------------------------------

void Lvk::Nlp::SimpleAimlEngine::setRules(const Lvk::Nlp::RuleList &rules)
{
    m_rules = rules;

    RuleList pureAimlRules;
    buildPureAimlRules(pureAimlRules);

    AimlEngine::setRules(pureAimlRules);
}

//--------------------------------------------------------------------------------------------------

void Lvk::Nlp::SimpleAimlEngine::buildPureAimlRules(Lvk::Nlp::RuleList &pureAimlRules) const
{
    pureAimlRules.clear();

    for (int i = 0; i < m_rules.size(); ++i) {
        try {
            Lvk::Nlp::Rule pureAimlRule;
            buildPureAimlRule(pureAimlRule, m_rules[i]);
            pureAimlRules.append(pureAimlRule);
        } catch (std::exception &) {
            // Nothing to to. If the rule is invalid, we just skip it
        }
    }
}

//--------------------------------------------------------------------------------------------------

void Lvk::Nlp::SimpleAimlEngine::buildPureAimlRule(Lvk::Nlp::Rule &pureAimlRule,
                                                   const Lvk::Nlp::Rule &rule) const
{
    QString varNameOnInput;

    pureAimlRule.setId(rule.id());
    buildPureAimlInputList(pureAimlRule.input(), varNameOnInput, rule.input());
    buildPureAimlOutputList(pureAimlRule.output(), varNameOnInput, rule.output());
}

//--------------------------------------------------------------------------------------------------

void Lvk::Nlp::SimpleAimlEngine::buildPureAimlInputList(QStringList &pureAimlInputList,
                                                        QString &varNameOnInput,
                                                        const QStringList &inputList) const
{
    pureAimlInputList.clear();
    varNameOnInput.clear();

    /*
     * For each input, convert strings like:
     *    Do you like [VarName]
     *
     * To pure AIML:
     *    Do you like *
     */

    for (int i = 0; i < inputList.size(); ++i) {
        const QString &input = inputList[i];
        QString pureAimlInput = input;

        int pos = m_varNameRegex.indexIn(input);

        // If variable decl found
        if (pos != -1) {

            if (pos != m_varNameRegex.lastIndexIn(input)) {
                throw InvalidSyntaxException(
                        QObject::tr("A rule input cannot contain two or more variables"));
            }

            if (!varNameOnInput.isNull() && varNameOnInput != m_varNameRegex.cap(1)) {
                throw InvalidSyntaxException(
                        QObject::tr("Rules cannot contain two or more different variable names"));
            }

            varNameOnInput = m_varNameRegex.cap(1);

            pureAimlInput.replace(m_varNameRegex, " * ");
        }

        pureAimlInputList.append(pureAimlInput);
    }
}

//--------------------------------------------------------------------------------------------------

void Lvk::Nlp::SimpleAimlEngine::buildPureAimlOutputList(QStringList &pureAimlOutputList,
                                                         const QString &varNameOnInput,
                                                         const QStringList &outputList) const
{
    pureAimlOutputList.clear();

    /*
     * For each output, convert strings like:
     *    {if [VarName] = football}
     *       Yes I like [VarName]
     *    {else}
     *       No, I don't
     *
     * To pure AIML:
     *    <think><set name="like"><star/></set></think>
     *    <condition>
     *       <li name="like" value="football"> Yes I like <star/></li>
     *       <li>No, I don't</li>
     *    </condition>
     */

    for (int i = 0; i < outputList.size(); ++i) {

        // TODO refactor!

        QString output = outputList[i];
        QString pureAimlOutput = outputList[i];
        int pos;

        // Parse If-Else

        pos = m_ifElseRegex.indexIn(output);
        if (pos != -1) {
            pureAimlOutput = QString("<condition>"
                                     "<li name=\"%1\" value=\"%2\">%3</li>"
                                     "<li>%4</li>"
                                     "</condition>")
                                        .arg(m_ifElseRegex.cap(1))
                                        .arg(m_ifElseRegex.cap(2).trimmed())
                                        .arg(m_ifElseRegex.cap(3).trimmed())
                                        .arg(m_ifElseRegex.cap(4).trimmed());
        } else {
            // Parse If

            pos = m_ifRegex.indexIn(output);
            if (pos != -1) {
                pureAimlOutput = QString("<condition>"
                                         "<li name=\"%1\" value=\"%2\">%3</li>"
                                         "</condition>")
                                            .arg(m_ifRegex.cap(1))
                                            .arg(m_ifRegex.cap(2).trimmed())
                                            .arg(m_ifRegex.cap(3).trimmed());
            }
        }

        // Parse Variables

        pos = 0;
        while (pos != -1) {
            pos = m_varNameRegex.indexIn(output, pos);

            if (pos != -1) {
                QString varName = m_varNameRegex.cap(1);

                if (varName == varNameOnInput) {
                    pureAimlOutput.replace("[" + varName + "]", "<star/>", Qt::CaseInsensitive);
                } else {
                    pureAimlOutput.replace("[" + varName + "]", "<get name=\"" + varName + "\" />",
                                           Qt::CaseInsensitive);
                }

                pos++;
            }

        }

        if (!varNameOnInput.isNull()) {
            pureAimlOutput.prepend("<think>"
                                   "<set name=\"" + varNameOnInput + "\"><star/></set>"
                                   "</think>");
        }

        pureAimlOutputList.append(pureAimlOutput);
    }
}