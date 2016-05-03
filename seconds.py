#-*- coding: utf-8  -*-


class Seconds(object):
    def __init__(self, count):
        self._seconds = count

    def this(self):
        return "{} seconds".format(self._seconds)


class Minute(object):
    def __init__(self, count):
        self._minutes = count
        self._seconds = Seconds(self._minutes * 60)

    def seconds(self):
        return self._seconds.this()

    def this(self):
        return "{} minutes".format(self._minutes)


class Hour(object):
    def __init__(self, count):
        self._hours   = count
        self._minutes = Minute(self._hours * 60)

    def seconds(self):
        return self._minutes.seconds()

    def minutes(self):
        return self._minutes.this()

    def this(self):
        return "{} hours".format(self._hours)


class Day(object):
    def __init__(self, count):
        self._days   = count
        self._hours  = Hour(self._days * 24)

    def seconds(self):
        return self._hours.seconds()

    def minutes(self):
        return self._hours.minutes()

    def hours(self):
        return self._hours.this()

    def this(self):
        return "{} days".format(self._days)


class Week(object):
    def __init__(self, count):
        self._weeks  = count
        self._days   = Day(self._weeks * 7)

    def seconds(self):
        return self._days.seconds()

    def minutes(self):
        return self._days.minutes()

    def hours(self):
        return self._days.hours()

    def days(self):
        return self._days.this()

    def this(self):
        return "{:.1f} weeks".format(self._weeks)


class Month(object):
    def __init__(self, count):
        self._months = count
        self._weeks  = Week(self._months * 4)

    def seconds(self):
        return self._weeks.seconds()

    def minutes(self):
        return self._weeks.minutes()

    def hours(self):
        return self._weeks.hours()

    def days(self):
        return self._weeks.days()

    def weeks(self):
        return self._weeks.this()

    def this(self):
        return "{} months".format(self._months)


class Year(object):
    def __init__(self, count):
        self._years  = count
        self._months = Month(self._years * 12)
        self._weeks  = Week( self._years * 365/7)
        self._days   = Day(  self._years * 365)

    def seconds(self):
        return self._days.seconds()

    def insec(self):
        return self._days._hours._minutes._seconds._seconds

    def minutes(self):
        return self._days.minutes()

    def hours(self):
        return self._days.hours()

    def days(self):
        return self._days.this()

    def weeks(self):
        return self._weeks.this()

    def months(self):
        return self._months.this()

    def this(self):
        return "{} years".format(self._years)


sec_in_year = Year(1).insec()
print(sec_in_year)

se7en_bil = 7 * 1000**3
print(se7en_bil)

years = se7en_bil / sec_in_year

print(years) # 222 years

two2two = Year(222)

print()
print(two2two.this())
print(two2two.months())
print(two2two.weeks())
print(two2two.days())
print(two2two.hours())
print(two2two.minutes())
print(two2two.seconds())


