#ifndef REMOTEBRIDGE_HPP
#define REMOTEBRIDGE_HPP

#include <QObject>
#include <QUrl>
#include <QAbstractSocket>

#include "Radio.hpp"

class QWebSocket;
class QTimer;
class QJsonObject;

// WSJT-Y remote-control bridge.
//
// Connects OUTBOUND over WSS to a relay (e.g. wss://aprsnet.uk/wsjty-relay)
// so a paired companion app (phone/tablet) can see decodes, station status,
// and logged QSOs, and send back a small command set (reply to a CQ, halt
// Tx, change band) — without WSJT-Y ever accepting an inbound connection
// itself. Works over WAN with no port forwarding; TLS + auth are handled
// by the relay, not by WSJT-Y.
//
// This class deliberately knows nothing about MainWindow/UI — it only
// emits Qt signals for incoming commands and exposes plain send*() calls
// for outgoing events, mirroring the shape of Network/MessageClient so the
// two can be wired into MainWindow the same way.
class RemoteBridge : public QObject
{
  Q_OBJECT

public:
  using Frequency = Radio::Frequency;

  explicit RemoteBridge (QObject * parent = nullptr);
  ~RemoteBridge () override;

  // (Re)configure and connect. An empty relay_url disables the bridge
  // (socket is closed, no reconnect attempts).
  void configure (QUrl const& relay_url, QString const& token);
  bool is_connected () const;

  // ---- Outgoing: WSJT-Y -> relay -> companion app ----
  void send_decode (QString const& utc_hms, int snr, double dt, quint32 audio_freq_hz,
                     QString const& mode, QString const& message, bool is_cq, bool for_me);
  void send_status (Frequency dial_frequency, QString const& mode, QString const& dx_call,
                     QString const& dx_grid, bool tx_enabled, bool transmitting);
  void send_qso_logged (QString const& call, QString const& grid, Frequency dial_frequency,
                         QString const& mode, QString const& report_sent, QString const& report_rcvd);
  void send_decodes_cleared ();

Q_SIGNALS:
  // ---- Incoming: companion app -> relay -> WSJT-Y ----
  void reply_requested (QString call, QString grid, quint32 audio_freq_hz);
  void halt_tx_requested ();
  void set_band_requested (Frequency freq_hz, QString band_name);
  void connection_state_changed (bool connected);

private Q_SLOTS:
  void on_connected ();
  void on_disconnected ();
  void on_text_message_received (QString message);
  void on_error (QAbstractSocket::SocketError);
  void attempt_reconnect ();

private:
  void open_socket ();
  void send_json (QJsonObject const& obj);

  QWebSocket * socket_ = nullptr;
  QTimer     * reconnect_timer_ = nullptr;
  QUrl         relay_url_;
  QString      token_;
  bool         enabled_ = false;
};

#endif
