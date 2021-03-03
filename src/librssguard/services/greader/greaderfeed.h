// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef GREADERFEED_H
#define GREADERFEED_H

#include "services/abstract/feed.h"

class GreaderServiceRoot;

class GreaderFeed : public Feed {
  public:
    explicit GreaderFeed(RootItem* parent = nullptr);

    GreaderServiceRoot* serviceRoot() const;

    virtual QList<Message> obtainNewMessages(bool* error_during_obtaining);
};

#endif // GREADERFEED_H
