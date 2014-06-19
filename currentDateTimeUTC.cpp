#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <QPair>
#include <QDateTime>
#include <QString>

QPair<QDateTime, long>
currentDateTimeUTC()
{
  // TODO: add ifdefs for WIN and NIX
  FILETIME ft_utc;
  GetSystemTimeAsFileTime(&ft_utc);

  // shift is difference between 1970-Jan-01 & 1601-Jan-01 in 100-nanosecond intervals
  const quint64 shift = 116444736000000000ULL;        // (27111902 << 32) + 3577643008

  union {
      FILETIME  as_file_time;
      quint64   as_integer;                           // 100-nanos since 1601-Jan-01
  } caster;

  caster.as_file_time = ft_utc;
  caster.as_integer  -= shift;                        // filetime is now 100-nanos since 1970-Jan-01
  quint64 micros = caster.as_integer / 10;            // truncate to microseconds
  time_t t = static_cast<time_t>(micros / 1000000UL); // seconds since epoch
  long mic = static_cast<long>  (micros % 1000000UL);
  QDateTime now;
  now.setTime_t(t);
  now.setTimeSpec(Qt::UTC);
  return qMakePair(now, mic);
}

TEST_CASE( "currentDateTimeUTC", "[util]" ) {
    QPair<QDateTime, long> boo = currentDateTimeUTC();
    QDateTime now = QDateTime::currentDateTimeUts();
    QString s1 = boo.first.toString("yyyy-MM-dd-hh-mm-ss");
    QString s2 = now      .toString("yyyy-MM-dd-hh-mm-ss");
    REQUIRE( s1 = s2 );
}


