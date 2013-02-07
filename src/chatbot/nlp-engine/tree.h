/*
 * Copyright (C) 2012 Andres Pagliano, Gabriel Miretti, Gonzalo Buteler,
 * Nestor Bustamante, Pablo Perez de Angelis
 *
 * This file is part of LVK Chatbot.
 *
 * LVK Chatbot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LVK Chatbot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LVK Chatbot.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LVK_NLP_TREE_H
#define LVK_NLP_TREE_H

#include <QStringList>
#include <QPair>
#include <QList>

#include "nlp-engine/engine.h"
#include "nlp-engine/word.h"

namespace Lvk
{

/// \addtogroup Lvk
/// @{

namespace Nlp
{

class Rule;
class Node;
class MatchPolicy;
class ScoringAlgorithm;

/// \ingroup Lvk
/// \addtogroup Nlp
/// @{

/**
 * \brief The Tree class provides ...
 */
class Tree
{
public:

    /**
     * Constructs an empty tree
     */
    Tree();

    /**
     * Destroys the object
     */
    ~Tree();

    /**
     * Adds \a rule to the tree
     */
    void add(const Rule &rule);

    /**
     * Gets the list of responses for the given input
     */
    QStringList getResponses(const QString &input, Engine::MatchList &matches);

private:
    Tree(Tree&);
    Tree& operator=(Tree&);

    Node *m_root;
    MatchPolicy *m_matchPolicy;
    ScoringAlgorithm *m_scoringAlg;

    // TODO move this out. Replace old MatchList with ResultList
    struct Result
    {
        Result(const QString &output = "", RuleId ruleId = 0, int inputIndex = 0, float score = 0)
            : output(output), ruleId(ruleId), inputIndex(inputIndex), score(score) { }

        QString output;
        RuleId ruleId;
        int inputIndex;
        float score;
    };

    typedef QList<Result> ResultList;

    Nlp::Node * addNode(const Nlp::Word &w, Nlp::Node *parent);
    void setNodeOutput(Nlp::Node *node, const QStringList &output);
    void scoredDFS(ResultList &r, const Nlp::Node *root, const Nlp::WordList &words,
                   int offset = 0);
    QString getOutput(const Nlp::Node *node);
    void filterPunct(Nlp::WordList &words);
};

/// @}

} // namespace Nlp

/// @}

} // namespace Lvk


#endif // LVK_NLP_TREE_H

