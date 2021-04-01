// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/gmail/gui/formeditgmailaccount.h"

#include "gui/guiutilities.h"
#include "miscellaneous/application.h"
#include "miscellaneous/iconfactory.h"
#include "network-web/oauth2service.h"
#include "network-web/webfactory.h"
#include "services/gmail/definitions.h"
#include "services/gmail/gmailserviceroot.h"
#include "services/gmail/gui/gmailaccountdetails.h"

FormEditGmailAccount::FormEditGmailAccount(QWidget* parent)
  : FormAccountDetails(qApp->icons()->miscIcon(QSL("gmail")), parent), m_details(new GmailAccountDetails(this)) {
  insertCustomTab(m_details, tr("Server setup"), 0);
  activateTab(0);

  m_details->m_ui.m_txtUsername->setFocus();
}

void FormEditGmailAccount::apply() {
  FormAccountDetails::apply();

  // Make sure that the data copied from GUI are used for brand new login.
  account<GmailServiceRoot>()->network()->oauth()->logout(false);
  account<GmailServiceRoot>()->network()->oauth()->setClientId(m_details->m_ui.m_txtAppId->lineEdit()->text());
  account<GmailServiceRoot>()->network()->oauth()->setClientSecret(m_details->m_ui.m_txtAppKey->lineEdit()->text());
  account<GmailServiceRoot>()->network()->oauth()->setRedirectUrl(m_details->m_ui.m_txtRedirectUrl->lineEdit()->text());

  account<GmailServiceRoot>()->network()->setUsername(m_details->m_ui.m_txtUsername->lineEdit()->text());
  account<GmailServiceRoot>()->network()->setBatchSize(m_details->m_ui.m_spinLimitMessages->value());
  account<GmailServiceRoot>()->network()->setDownloadOnlyUnreadMessages(m_details->m_ui.m_cbDownloadOnlyUnreadMessages->isChecked());

  account<GmailServiceRoot>()->saveAccountDataToDatabase();
  accept();

  if (!m_creatingNew) {
    account<GmailServiceRoot>()->completelyRemoveAllData();
    account<GmailServiceRoot>()->start(true);
  }
}

void FormEditGmailAccount::loadAccountData() {
  FormAccountDetails::loadAccountData();

  m_details->m_oauth = account<GmailServiceRoot>()->network()->oauth();
  m_details->hookNetwork();

  // Setup the GUI.
  m_details->m_ui.m_txtAppId->lineEdit()->setText(m_details->m_oauth->clientId());
  m_details->m_ui.m_txtAppKey->lineEdit()->setText(m_details->m_oauth->clientSecret());
  m_details->m_ui.m_txtRedirectUrl->lineEdit()->setText(m_details->m_oauth->redirectUrl());

  m_details->m_ui.m_txtUsername->lineEdit()->setText(account<GmailServiceRoot>()->network()->username());
  m_details->m_ui.m_spinLimitMessages->setValue(account<GmailServiceRoot>()->network()->batchSize());
  m_details->m_ui.m_cbDownloadOnlyUnreadMessages->setChecked(account<GmailServiceRoot>()->network()->downloadOnlyUnreadMessages());
}
