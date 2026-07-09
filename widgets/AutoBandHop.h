#ifndef AUTOBANDHOP_H
#define AUTOBANDHOP_H

// AutoBandHop — selects the best FT8 band based on UTC time and calendar month.
//
// Scoring model derived from:
//   "Comprehensive Analysis of High-Frequency Amateur Radio Digital Mode
//    Allocations and Propagation Optimization" (RSGB 2026 / Ofcom 2024)
//
// Each band is scored on:
//   basePriority  — general reliability of the band
//   +10 if current UTC hour is inside the optimal window AND month is optimal
//   +5  if UTC hour is inside the optimal window only
//   +3  if month is optimal only
//
// The highest-scoring band wins. On equal scores the more reliable band wins.

#include <QObject>
#include <QTimer>
#include <QString>
#include <QDateTime>

struct BandEntry {
    const char *name;
    double      freqMHz;      // FT8 dial frequency
    int         startUTC;     // optimal window start hour (0-23, UTC)
    int         endUTC;       // optimal window end hour (exclusive, wraps midnight if start > end)
    int         monthMask;    // bitmask, bit0=Jan…bit11=Dec; 0xFFF = year-round
    int         basePriority; // higher = prefer when scores are equal
    const char *reason;       // short description shown in status bar
};

class AutoBandHop : public QObject
{
    Q_OBJECT
public:
    explicit AutoBandHop(QObject *parent = nullptr);

    void setEnabled(bool on);
    bool enabled() const { return m_enabled; }

    void setIntervalMinutes(int minutes);
    int  intervalMinutes()  const { return m_intervalMin; }

    // Returns the best FT8 frequency (MHz) for the given UTC datetime.
    // Also sets *reason (if non-null) to a human-readable explanation.
    static double bestFrequencyMHz(QDateTime const& utc, QString *reason = nullptr);

    // All band entries (for display in settings)
    static const BandEntry* bands();
    static int               bandCount();

signals:
    void hopRequested(double freqMHz, QString reason);

private slots:
    void evaluate();

private:
    QTimer m_timer;
    bool   m_enabled = false;
    int    m_intervalMin = 5;

    static int scoreFor(BandEntry const& b, int utcHour, int monthIndex);
    static bool inWindow(int hour, int start, int end);
};

#endif // AUTOBANDHOP_H
