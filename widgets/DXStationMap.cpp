#include "DXStationMap.h"
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <cmath>

static constexpr double DEG = M_PI / 180.0;

DXStationMap::DXStationMap(QWidget *parent)
  : QWidget(parent)
{
    setMinimumSize(200, 140);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: #080d14;");
}

void DXStationMap::setHomeGrid(QString const& grid)
{
    m_homeGrid = grid.toUpper().left(6);
    gridToLatLon(m_homeGrid, m_homeLat, m_homeLon);
    update();
}

void DXStationMap::showStation(QString const& call, QString const& grid,
                                int snr, bool isCQ, bool forMe)
{
    m_selCall = call;
    m_selGrid = grid.toUpper().left(4);
    m_selSNR  = snr;
    gridToLatLon(m_selGrid, m_selLat, m_selLon);

    PlottedStation s;
    s.call = call; s.grid = m_selGrid; s.snr = snr; s.isCQ = isCQ; s.forMe = forMe;
    for (auto &existing : m_stations)
        if (existing.call == call) { existing = s; update(); return; }
    m_stations.append(s);
    update();
}

void DXStationMap::clearStations()
{
    m_stations.clear();
    m_selCall.clear();
    update();
}

void DXStationMap::addStation(PlottedStation const& s)
{
    for (auto &e : m_stations) if (e.call == s.call) { e = s; update(); return; }
    m_stations.append(s);
    update();
}

// Map a (lon, lat) pair to widget pixel coordinates.
// Flat equirectangular projection — simple but familiar for ham radio use.
QPointF DXStationMap::project(double lon, double lat) const
{
    const int h = height(), w = width();
    // Leave a 14px info bar at the bottom
    const int mapH = h - 14;
    const double x = (lon + 180.0) / 360.0 * w;
    const double y = (90.0 - lat) / 180.0 * mapH;
    return {x, y};
}

bool DXStationMap::gridToLatLon(QString const& grid, double &lat, double &lon) const
{
    if (grid.length() < 2) return false;
    const auto g = grid.toUpper();
    lon = (g[0].unicode() - 'A') * 20.0 - 180.0;
    lat = (g[1].unicode() - 'A') * 10.0 - 90.0;
    if (g.length() >= 4) {
        lon += (g[2].unicode() - '0') * 2.0;
        lat += (g[3].unicode() - '0') * 1.0;
    }
    lon += (g.length() >= 4) ? 1.0 : 10.0;
    lat += (g.length() >= 4) ? 0.5 : 5.0;
    return true;
}

double DXStationMap::haversineKm(double lat1, double lon1,
                                  double lat2, double lon2) const
{
    const double R = 6371.0;
    const double dLat = (lat2 - lat1) * DEG;
    const double dLon = (lon2 - lon1) * DEG;
    const double a = std::sin(dLat/2)*std::sin(dLat/2)
                   + std::cos(lat1*DEG)*std::cos(lat2*DEG)
                   * std::sin(dLon/2)*std::sin(dLon/2);
    return R * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1-a));
}

double DXStationMap::bearingDeg(double lat1, double lon1,
                                 double lat2, double lon2) const
{
    const double y = std::sin((lon2-lon1)*DEG) * std::cos(lat2*DEG);
    const double x = std::cos(lat1*DEG)*std::sin(lat2*DEG)
                   - std::sin(lat1*DEG)*std::cos(lat2*DEG)*std::cos((lon2-lon1)*DEG);
    return std::fmod(std::atan2(y,x)/DEG + 360.0, 360.0);
}

void DXStationMap::drawGridSquare(QPainter &p, QString const& grid,
                                   QColor fill, QColor stroke, int strokeW) const
{
    if (grid.length() < 2) return;
    const auto g = grid.toUpper();
    double lon0, lat0, lon1, lat1;
    lon0 = (g[0].unicode()-'A') * 20.0 - 180.0;
    lat0 = (g[1].unicode()-'A') * 10.0 - 90.0;
    if (g.length() >= 4) {
        lon0 += (g[2].unicode()-'0') * 2.0;
        lat0 += (g[3].unicode()-'0') * 1.0;
        lon1 = lon0 + 2.0; lat1 = lat0 + 1.0;
    } else {
        lon1 = lon0 + 20.0; lat1 = lat0 + 10.0;
    }
    const QPointF tl = project(lon0, lat1);
    const QPointF br = project(lon1, lat0);
    p.setBrush(fill);
    p.setPen(QPen(stroke, strokeW));
    p.drawRect(QRectF(tl, br));
}

void DXStationMap::drawArc(QPainter &p, double lat1, double lon1,
                            double lat2, double lon2) const
{
    // Approximate great circle as 60 interpolated points (slerp)
    QPainterPath path;
    bool first = true;
    for (int i = 0; i <= 60; ++i) {
        const double t = double(i) / 60.0;
        const double d = 2.0 * std::asin(std::sqrt(
            std::pow(std::sin((lat2-lat1)*DEG/2),2) +
            std::cos(lat1*DEG)*std::cos(lat2*DEG)*
            std::pow(std::sin((lon2-lon1)*DEG/2),2)));
        if (d < 1e-6) { path.lineTo(project(lon2,lat2)); break; }
        const double A = std::sin((1-t)*d)/std::sin(d);
        const double B = std::sin(t*d)/std::sin(d);
        const double x = A*std::cos(lat1*DEG)*std::cos(lon1*DEG)+B*std::cos(lat2*DEG)*std::cos(lon2*DEG);
        const double y = A*std::cos(lat1*DEG)*std::sin(lon1*DEG)+B*std::cos(lat2*DEG)*std::sin(lon2*DEG);
        const double z = A*std::sin(lat1*DEG)+B*std::sin(lat2*DEG);
        const double lat = std::atan2(z,std::sqrt(x*x+y*y))/DEG;
        const double lon = std::atan2(y,x)/DEG;
        const QPointF pt = project(lon, lat);
        if (first) { path.moveTo(pt); first = false; } else { path.lineTo(pt); }
    }
    QPen arcPen(QColor(77,166,255), 1.5, Qt::DashLine);
    arcPen.setDashPattern({4.0, 3.0});
    p.setPen(arcPen);
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
}

void DXStationMap::drawHomeMarker(QPainter &p) const
{
    if (m_homeGrid.isEmpty()) return;
    const QPointF pt = project(m_homeLon, m_homeLat);
    p.setBrush(QColor(63,185,80));
    p.setPen(QPen(QColor(46,160,67), 1));
    p.drawEllipse(pt, 4, 4);
    p.setPen(QColor(63,185,80));
    p.setFont(QFont("sans-serif", 7));
    p.drawText(int(pt.x())+6, int(pt.y())-3, m_homeGrid.left(4));
}

void DXStationMap::drawStationMarker(QPainter &p, double lat, double lon,
                                      QString const& label, QColor col) const
{
    const QPointF pt = project(lon, lat);
    p.setBrush(col);
    p.setPen(QPen(col.darker(140), 1));
    p.drawEllipse(pt, 5, 5);
    p.setPen(col);
    p.setFont(QFont("Courier New", 7, QFont::Bold));
    p.drawText(int(pt.x())+7, int(pt.y())-4, label.left(12));
}

void DXStationMap::drawInfoBar(QPainter &p) const
{
    const int h = height(), w = width();
    p.fillRect(0, h-14, w, 14, QColor(15,20,28));
    p.setPen(QColor(90,100,120));
    p.drawLine(0, h-14, w, h-14);
    p.setFont(QFont("sans-serif", 7));

    if (!m_selCall.isEmpty()) {
        const double km   = haversineKm(m_homeLat, m_homeLon, m_selLat, m_selLon);
        const double brg  = bearingDeg(m_homeLat, m_homeLon, m_selLat, m_selLon);
        const QString txt = QString("%1  %2  Az:%3°  %4km  SNR:%5dB")
            .arg(m_selCall, -9)
            .arg(m_selGrid, -4)
            .arg(int(brg), 3)
            .arg(int(km + 0.5))
            .arg(m_selSNR);
        p.setPen(QColor(200,210,230));
        p.drawText(6, h-3, txt);
    } else {
        p.setPen(QColor(60,70,90));
        p.drawText(6, h-3, "click a callsign to plot on map");
    }
}

void DXStationMap::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width(), h = height(), mapH = h - 14;
    if (w < 10 || mapH < 10) return;

    // Background gradient: deep navy, poles slightly darker
    for (int y = 0; y < mapH; ++y) {
        const double t = double(y) / mapH;
        const int r = 4 + int(t * (1-t) * 16);
        const int g = 8 + int(t * (1-t) * 24);
        const int b = 20 + int(t * (1-t) * 40);
        p.fillRect(0, y, w, 1, QColor(r, g, b));
    }

    // Continent shading (very subtle fill for each 20°×10° field square)
    // Uses a look-up of which fields are predominantly land vs ocean
    static const char *landFields[] = {
        // Europe / Asia band
        "IA","IB","IC","ID","IE","IF","IG","IH","II","IJ","IK","IL","IM","IN","IO","IP","IQ","IR",
        "JA","JB","JC","JD","JE","JF","JG","JH","JI","JJ","JK","JL","JM","JN","JO","JP","JQ","JR",
        "KA","KB","KC","KD","KE","KF","KG","KH","KI","KJ","KK","KL","KM","KN","KO","KP",
        "LA","LB","LC","LD","LE","LF","LG","LH","LI","LJ","LK","LL","LM","LN","LO","LP",
        "MA","MB","MC","MD","ME","MF","MG","MH","MI","MJ","MK","ML","MM","MN","MO","MP",
        "NA","NB","NC","ND","NE","NF","NG","NH","NI","NJ","NK","NL","NM","NN","NO","NP",
        "OA","OB","OC","OD","OE","OF","OG","OH","OI","OJ","OK","OL","OM","ON","OO","OP",
        "PA","PB","PC","PD","PE","PF","PG","PH","PI","PJ","PK","PL","PM","PN","PO","PP",
        // Africa band
        "HA","HB","HC","HD","HE","HF","HG","HH","HI","HJ","HK","HL","HM","HN","HO","HP",
        "GA","GB","GC","GD","GE","GF","GG","GH","GI","GJ","GK","GL","GM","GN","GO","GP","GQ",
        "FA","FB","FC","FD","FE","FF","FG","FH","FI","FJ","FK","FL","FM","FN","FO","FP","FQ","FR",
        // Americas
        "DM","DN","DO","DP","DQ","DR","DL","DK","DJ",
        "EM","EN","EO","EP","EQ","ER","EL","EK","EJ","EH","EG","EF","EE","ED","EC","EB","EA",
        "FM","FN","FO","FP","FQ","FR",
        "GF","GG","GH","GI","GJ","GK",
        // Australia
        "QF","QG","QH","RF","RG",
        nullptr
    };
    QSet<QString> landSet;
    for (int i = 0; landFields[i]; ++i)
        landSet.insert(QString(landFields[i]));

    // Draw field squares
    for (int fi = 0; fi < 18; ++fi) {
        for (int li = 0; li < 18; ++li) {
            QString code;
            code.append(QChar('A'+fi));
            code.append(QChar('A'+li));
            const double lon0 = fi*20.0-180.0, lat0 = li*10.0-90.0;
            const QPointF tl = project(lon0, lat0+10.0);
            const QPointF br = project(lon0+20.0, lat0);
            const bool land = landSet.contains(code);
            p.setPen(Qt::NoPen);
            if (land)
                p.setBrush(QColor(8, 22, 45, 120));   // subtle land tint
            else
                p.setBrush(Qt::NoBrush);
            if (land) p.drawRect(QRectF(tl, br));
        }
    }

    // Grid lines — visible teal-blue
    p.setBrush(Qt::NoBrush);
    for (int i = 0; i <= 18; ++i) {
        const double lon = i*20.0-180.0;
        p.setPen(QPen(QColor(18, 55, 100, 180), 1));
        p.drawLine(project(lon, -90), project(lon, 90));
    }
    for (int j = 0; j <= 18; ++j) {
        const double lat = j*10.0-90.0;
        p.setPen(QPen(QColor(18, 55, 100, 180), 1));
        p.drawLine(project(-180, lat), project(180, lat));
    }

    // Field letter labels at grid intersections
    p.setFont(QFont("sans-serif", qMax(5, w/52)));
    p.setPen(QColor(30, 80, 140, 160));
    for (int fi = 0; fi < 18; ++fi) {
        for (int li = 0; li < 18; ++li) {
            const double lon = fi*20.0-180.0+10.0, lat = li*10.0-90.0+5.0;
            const QPointF c = project(lon, lat);
            if (c.x() < 0 || c.x() > w || c.y() < 0 || c.y() > mapH) continue;
            QString lbl;
            lbl.append(QChar('A'+fi));
            lbl.append(QChar('A'+li));
            const QFontMetrics fm(p.font());
            const int tw = fm.horizontalAdvance(lbl);
            p.drawText(QPointF(c.x()-tw/2, c.y()+fm.ascent()/2), lbl);
        }
    }

    // Equator and prime meridian — slightly brighter
    p.setPen(QPen(QColor(30, 90, 160, 120), 1, Qt::DashLine));
    p.drawLine(project(-180,0), project(180,0));
    p.drawLine(project(0,-90), project(0,90));

    // Highlighted field/square for selected station
    if (!m_selGrid.isEmpty()) {
        const QString field = m_selGrid.left(2);
        if (field.length() == 2) {
            drawGridSquare(p, field, QColor(31,111,235,45), QColor(40,130,255,160), 1);
        }
        if (m_selGrid.length() >= 4) {
            drawGridSquare(p, m_selGrid.left(4), QColor(77,166,255,70), QColor(100,200,255), 2);
        }
    }

    // All period stations — dim dots
    for (auto const& s : m_stations) {
        if (s.call == m_selCall) continue;
        double lat, lon;
        if (!gridToLatLon(s.grid, lat, lon)) continue;
        QColor col = s.forMe ? QColor(80,220,120,200) :
                     s.isCQ  ? QColor(80,160,255,200) :
                                QColor(90,110,140,160);
        const QPointF pt = project(lon, lat);
        p.setBrush(col); p.setPen(Qt::NoPen);
        p.drawEllipse(pt, qMax(2.0, w*0.012), qMax(2.0, w*0.012));
    }

    // Great circle arc
    if (!m_selGrid.isEmpty() && !m_homeGrid.isEmpty())
        drawArc(p, m_homeLat, m_homeLon, m_selLat, m_selLon);

    // Home QTH marker
    drawHomeMarker(p);

    // Selected station marker
    if (!m_selCall.isEmpty())
        drawStationMarker(p, m_selLat, m_selLon, m_selCall, QColor(255,100,80));

    drawInfoBar(p);
}

void DXStationMap::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    update();
}

void DXStationMap::mouseMoveEvent(QMouseEvent *e)
{
    // Show field square label as tooltip
    const double lon = (double(e->pos().x()) / width()) * 360.0 - 180.0;
    const double lat = 90.0 - (double(e->pos().y()) / (height()-14)) * 180.0;
    if (lat >= -90 && lat <= 90) {
        const int fi = int((lon + 180.0) / 20.0);
        const int li = int((lat + 90.0) / 10.0);
        if (fi >= 0 && fi < 18 && li >= 0 && li < 18) {
            QString field;
            field.append(QChar('A' + fi));
            field.append(QChar('A' + li));
            const int sqi = int(std::fmod(lon + 180.0, 20.0) / 2.0);
            const int sqj = int(std::fmod(lat + 90.0, 10.0));
            field.append(QChar('0' + sqi));
            field.append(QChar('0' + sqj));
            QToolTip::showText(e->globalPos(), field, this);
        }
    }
    QWidget::mouseMoveEvent(e);
}

void DXStationMap::mouseDoubleClickEvent(QMouseEvent *)
{
    clearStations();
}
