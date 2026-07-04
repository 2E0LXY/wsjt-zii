#include "qrzlookup.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>

constexpr char QRZLookup::URL[];

QRZLookup::QRZLookup(QString const& user, QString const& pass, QObject *parent)
    : QObject(parent), m_user(user), m_pass(pass)
{
    connect(&m_sessionNam, &QNetworkAccessManager::finished, this, &QRZLookup::onSessionReply);
    connect(&m_lookupNam,  &QNetworkAccessManager::finished, this, &QRZLookup::onLookupReply);
    connect(&m_photoNam,   &QNetworkAccessManager::finished, this, &QRZLookup::onPhotoReply);
    if (!m_user.isEmpty()) startSession();
}

void QRZLookup::startSession()
{
    QUrl url{URL};
    QUrlQuery q;
    q.addQueryItem("username", m_user);
    q.addQueryItem("password", m_pass);
    q.addQueryItem("agent",    "WSJT-Y");
    url.setQuery(q);
    m_sessionNam.get(QNetworkRequest{url});
}

void QRZLookup::onSessionReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) return;
    m_sessionKey = tagText(reply->readAll(), "Key");
    if (!m_pendingCall.isEmpty()) {
        auto c = m_pendingCall; m_pendingCall.clear(); lookup(c);
    }
}

void QRZLookup::lookup(QString const& call)
{
    if (m_user.isEmpty()) return;   // no credentials configured
    if (m_sessionKey.isEmpty()) { m_pendingCall = call; startSession(); return; }
    QUrl url{URL};
    QUrlQuery q;
    q.addQueryItem("s",        m_sessionKey);
    q.addQueryItem("callsign", call.toUpper());
    url.setQuery(q);
    m_lookupNam.get(QNetworkRequest{url});
}

void QRZLookup::onLookupReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) return;

    QrzRecord r;
    QXmlStreamReader xml{reply->readAll()};
    while (!xml.atEnd()) {
        if (xml.readNextStartElement()) {
            auto tag = xml.name().toString();
            if (tag=="call")    r.call    = xml.readElementText();
            else if (tag=="fname")   r.fname   = xml.readElementText();
            else if (tag=="name")    r.name    = xml.readElementText();
            else if (tag=="addr1")   r.addr1   = xml.readElementText();
            else if (tag=="addr2")   r.addr2   = xml.readElementText();
            else if (tag=="state")   r.state   = xml.readElementText();
            else if (tag=="country") r.country = xml.readElementText();
            else if (tag=="county")  r.county  = xml.readElementText();
            else if (tag=="grid")    r.grid    = xml.readElementText();
            else if (tag=="email")   r.email   = xml.readElementText();
            else if (tag=="bio")     r.bio     = xml.readElementText();
            else if (tag=="image")   r.imageUrl= QUrl{xml.readElementText()};
            else if (tag=="Error") { xml.skipCurrentElement(); r.valid=false; return; }
        }
    }
    r.valid = !r.call.isEmpty();
    if (r.valid && r.imageUrl.isValid()) {
        // Fetch photo separately
        m_photoNam.get(QNetworkRequest{r.imageUrl});
        m_pendingRecord = r;
    } else if (r.valid) {
        emit lookupResult(r);
    }
}

void QRZLookup::onPhotoReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError)
        m_pendingRecord.photo.loadFromData(reply->readAll());
    emit lookupResult(m_pendingRecord);
    m_pendingRecord = QrzRecord{};
}

QString QRZLookup::tagText(QByteArray const& data, QString const& tag)
{
    QXmlStreamReader xml{data};
    while (!xml.atEnd())
        if (xml.readNextStartElement() && xml.name().toString() == tag)
            return xml.readElementText();
    return {};
}
