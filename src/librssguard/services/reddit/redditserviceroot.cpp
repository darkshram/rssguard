// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/reddit/redditserviceroot.h"

#include "database/databasequeries.h"
#include "exceptions/feedfetchexception.h"
#include "miscellaneous/application.h"
#include "miscellaneous/iconfactory.h"
#include "network-web/oauth2service.h"
#include "services/abstract/importantnode.h"
#include "services/abstract/recyclebin.h"
#include "services/reddit/definitions.h"
#include "services/reddit/gui/formeditredditaccount.h"
#include "services/reddit/redditcategory.h"
#include "services/reddit/redditentrypoint.h"
#include "services/reddit/redditnetworkfactory.h"
#include "services/reddit/redditsubscription.h"

#include <QFileDialog>

RedditServiceRoot::RedditServiceRoot(RootItem* parent)
  : ServiceRoot(parent), m_network(new RedditNetworkFactory(this)) {
  m_network->setService(this);
  setIcon(RedditEntryPoint().icon());
}

void RedditServiceRoot::updateTitle() {
  setTitle(TextFactory::extractUsernameFromEmail(m_network->username()) + QSL(" (Reddit)"));
}

RootItem* RedditServiceRoot::obtainNewTreeForSyncIn() const {
  auto* root = new RootItem();

  auto feeds = m_network->subreddits(networkProxy());

  for (auto* feed : feeds) {
    root->appendChild(feed);
  }

  return root;
}

QVariantHash RedditServiceRoot::customDatabaseData() const {
  QVariantHash data;

  data[QSL("username")] = m_network->username();
  data[QSL("batch_size")] = m_network->batchSize();
  data[QSL("download_only_unread")] = m_network->downloadOnlyUnreadMessages();
  data[QSL("client_id")] = m_network->oauth()->clientId();
  data[QSL("client_secret")] = m_network->oauth()->clientSecret();
  data[QSL("refresh_token")] = m_network->oauth()->refreshToken();
  data[QSL("redirect_uri")] = m_network->oauth()->redirectUrl();

  return data;
}

void RedditServiceRoot::setCustomDatabaseData(const QVariantHash& data) {
  m_network->setUsername(data[QSL("username")].toString());
  m_network->setBatchSize(data[QSL("batch_size")].toInt());
  m_network->setDownloadOnlyUnreadMessages(data[QSL("download_only_unread")].toBool());
  m_network->oauth()->setClientId(data[QSL("client_id")].toString());
  m_network->oauth()->setClientSecret(data[QSL("client_secret")].toString());
  m_network->oauth()->setRefreshToken(data[QSL("refresh_token")].toString());
  m_network->oauth()->setRedirectUrl(data[QSL("redirect_uri")].toString(), true);
}

QList<Message> RedditServiceRoot::obtainNewMessages(Feed* feed,
                                                    const QHash<ServiceRoot::BagOfMessages, QStringList>&
                                                      stated_messages,
                                                    const QHash<QString, QStringList>& tagged_messages) {
  Q_UNUSED(stated_messages)
  Q_UNUSED(tagged_messages)
  Q_UNUSED(feed)

  QList<Message> messages = m_network->hot(qobject_cast<RedditSubscription*>(feed)->prefixedName(), networkProxy());

  return messages;
}

bool RedditServiceRoot::isSyncable() const {
  return true;
}

bool RedditServiceRoot::canBeEdited() const {
  return true;
}

bool RedditServiceRoot::editViaGui() {
  FormEditRedditAccount form_pointer(qApp->mainFormWidget());

  form_pointer.addEditAccount(this);
  return true;
}

bool RedditServiceRoot::supportsFeedAdding() const {
  return false;
}

bool RedditServiceRoot::supportsCategoryAdding() const {
  return false;
}

void RedditServiceRoot::start(bool freshly_activated) {
  if (!freshly_activated) {
    DatabaseQueries::loadRootFromDatabase<RedditCategory, RedditSubscription>(this);
    loadCacheFromFile();
  }

  updateTitle();

  if (getSubTreeFeeds().isEmpty()) {
    m_network->oauth()->login([this]() {
      syncIn();
    });
  }
  else {
    m_network->oauth()->login();
  }
}

QString RedditServiceRoot::code() const {
  return RedditEntryPoint().code();
}

QString RedditServiceRoot::additionalTooltip() const {
  return tr("Authentication status: %1\n"
            "Login tokens expiration: %2")
    .arg(network()->oauth()->isFullyLoggedIn() ? tr("logged-in") : tr("NOT logged-in"),
         network()->oauth()->tokensExpireIn().isValid() ? network()->oauth()->tokensExpireIn().toString() : QSL("-"));
}

void RedditServiceRoot::saveAllCachedData(bool ignore_errors) {
  Q_UNUSED(ignore_errors)
  auto msg_cache = takeMessageCache();
}
