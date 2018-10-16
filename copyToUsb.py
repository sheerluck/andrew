import os
import abc
import sys
import six
import math
import time
import collections
from datetime import datetime, timedelta, date as dtdate
from typing import List, Dict


def to_unicode(x):
    return x


def scale_1024(x, n_prefixes):
    if x <= 0:
        power = 0
    else:
        power = min(int(math.log(x, 2) / 10), n_prefixes - 1)
    scaled = float(x) / (2 ** (10 * power))
    return scaled, power


def timedelta_to_seconds(delta):
    if delta.microseconds:
        total = delta.microseconds * 1e-6
    else:
        total = 0
    total += delta.seconds
    total += delta.days * 60 * 60 * 24
    return total


class WrappingIO:

    def __init__(self, target, capturing=False):
        self.buffer = six.StringIO()
        self.target = target
        self.capturing = capturing

    def write(self, value):
        if self.capturing:
            self.buffer.write(value)
        else:
            self.target.write(value)

    def flush(self):
        self.buffer.flush()

    def _flush(self):
        value = self.buffer.getvalue()
        if value:
            self.flush()
            self.target.write(value)
            self.buffer.seek(0)
            self.buffer.truncate(0)


class StreamWrapper(object):

    def __init__(self):
        self.stdout = self.original_stdout = sys.stdout
        self.stderr = self.original_stderr = sys.stderr
        self.original_excepthook = sys.excepthook
        self.wrapped_stdout = 0
        self.wrapped_stderr = 0
        self.capturing = 0

    def start_capturing(self):
        self.capturing += 1
        self.update_capturing()

    def stop_capturing(self):
        self.capturing -= 1
        self.update_capturing()

    def update_capturing(self):
        if isinstance(self.stdout, WrappingIO):
            self.stdout.capturing = self.capturing > 0

        if isinstance(self.stderr, WrappingIO):
            self.stderr.capturing = self.capturing > 0

        if self.capturing <= 0:
            self.flush()

    def wrap_stdout(self):
        if not self.wrapped_stdout:
            self.stdout = sys.stdout = WrappingIO(self.original_stdout)
        self.wrapped_stdout += 1

        return sys.stdout

    def wrap_stderr(self):
        if not self.wrapped_stderr:
            self.stderr = sys.stderr = WrappingIO(self.original_stderr)
        self.wrapped_stderr += 1

        return sys.stderr

    def unwrap_stdout(self):
        if self.wrapped_stdout > 1:
            self.wrapped_stdout -= 1
        else:
            sys.stdout = self.original_stdout
            self.wrapped_stdout = 0

    def unwrap_stderr(self):
        if self.wrapped_stderr > 1:
            self.wrapped_stderr -= 1
        else:
            sys.stderr = self.original_stderr
            self.wrapped_stderr = 0

    def flush(self):
        if self.wrapped_stdout:
            try:
                self.stdout._flush()
            except AttributeError:
                self.wrapped_stdout = False

        if self.wrapped_stderr:
            try:
                self.stderr._flush()
            except AttributeError:
                self.wrapped_stderr = False

    def excepthook(self, exc_type, exc_value, exc_traceback):
        self.original_excepthook(exc_type, exc_value, exc_traceback)
        self.flush()


streams = StreamWrapper()


def string_or_lambda(input_):
    if isinstance(input_, str):
        def render_input(progress, data, width):
            return input_ % data
        return render_input
    else:
        return input_


def format_time(time, precision=timedelta(seconds=1)):
    precision_seconds = precision.total_seconds()

    if isinstance(time, float):
        try:
            time = timedelta(seconds=int(time))
        except OverflowError:
            time = None

    if isinstance(time, timedelta):
        seconds = time.total_seconds()
        seconds = seconds - (seconds % precision_seconds)
        return str(timedelta(seconds=seconds))
    elif isinstance(time, datetime):
        seconds = time.timestamp()
        seconds = seconds - (seconds % precision_seconds)

        try:
            dt = datetime.fromtimestamp(seconds)
        except ValueError:
            dt = datetime.max
        return str(dt)
    elif isinstance(time, dtdate):
        return str(time)
    elif time is None:
        return '--:--:--'
    else:
        raise TypeError(f"format_time: Unknown type {type(time)}")


class ProgressBarMixinBase:

    def __init__(self, **kwargs):
        pass

    def start(self, **kwargs):
        pass

    def update(self, value=None):
        pass

    def finish(self, *args, **kwargs):
        pass


class ProgressBarBase(collections.abc.Iterable, ProgressBarMixinBase):
    pass


class DefaultFdMixin(ProgressBarMixinBase):

    def __init__(self, fd=sys.stderr, **kwargs):
        if fd is sys.stdout:
            fd = streams.original_stdout

        elif fd is sys.stderr:
            fd = streams.original_stderr

        self.fd = fd
        ProgressBarMixinBase.__init__(self, **kwargs)

    def update(self, *args, **kwargs):
        ProgressBarMixinBase.update(self, *args, **kwargs)
        line = to_unicode('\r' + self._format_line())
        self.fd.write(line)

    def finish(self, *args, **kwargs):
        end = kwargs.pop('end', '\n')
        ProgressBarMixinBase.finish(self, *args, **kwargs)
        if end:
            self.fd.write(end)
        self.fd.flush()


class WidgetBase(metaclass=abc.ABCMeta):
    def __init__(self, **kwargs):
        pass

    @abc.abstractmethod
    def __call__(self, progress, data):
        pass


class AutoWidthWidgetBase(WidgetBase):
    @abc.abstractmethod
    def __call__(self, progress, data, width):
        pass


class FormatWidgetMixin:
    def __init__(self, format_, **kwargs):
        self.format_ = format_

    def __call__(self, progress, data, format_=None):
        try:
            return (format_ or self.format_) % data
        except (TypeError, KeyError) as e:
            print(f"Error while formatting {self.format_}")
            raise


class WidthWidgetMixin:
    def __init__(self, min_width=None, max_width=None, **kwargs):
        self.min_width = min_width
        self.max_width = max_width

    def check_size(self, progress):
        if self.min_width and self.min_width > progress.term_width:
            return False
        elif self.max_width and self.max_width < progress.term_width:
            return False
        else:
            return True


class Percentage(FormatWidgetMixin, WidgetBase):
    def __init__(self, format_='%(percentage)3d%%', **kwargs):
        FormatWidgetMixin.__init__(self, format_=format_, **kwargs)
        WidgetBase.__init__(self, format_=format_, **kwargs)

    def __call__(self, progress, data, format_=None):
        # If percentage is not available, display N/A%
        if 'percentage' in data and not data['percentage']:
            return FormatWidgetMixin.__call__(self, progress, data,
                                              format_='N/A%%')

        return FormatWidgetMixin.__call__(self, progress, data)


def create_marker(marker):
    def _marker(progress, data, width):
        if progress.max_value is not None \
                and progress.max_value > 0:
            length = int(progress.value / progress.max_value * width)
            return (marker * length)
        else:
            return marker
    return _marker


class Bar(AutoWidthWidgetBase):
    def __init__(self, marker='#', left='|', right='|', fill=' ', fill_left=True, **kwargs):
        self.marker = create_marker(marker)
        self.left = string_or_lambda(left)
        self.right = string_or_lambda(right)
        self.fill = string_or_lambda(fill)
        self.fill_left = fill_left

        AutoWidthWidgetBase.__init__(self, **kwargs)

    def __call__(self, progress, data, width):
        left = to_unicode(self.left(progress, data, width))
        right = to_unicode(self.right(progress, data, width))
        width -= progress.custom_len(left) + progress.custom_len(right)
        marker = to_unicode(self.marker(progress, data, width))
        fill = to_unicode(self.fill(progress, data, width))

        if self.fill_left:
            marker = marker.ljust(width, fill)
        else:
            marker = marker.rjust(width, fill)

        return left + marker + right


class FormatLabel(FormatWidgetMixin, WidthWidgetMixin):
    mapping = {
        'finished': ('end_time', None),
        'last_update': ('last_update_time', None),
        'max': ('max_value', None),
        'seconds': ('seconds_elapsed', None),
        'start': ('start_time', None),
        'elapsed': ('total_seconds_elapsed', format_time),
        'value': ('value', None),
    }

    def __init__(self, format_, **kwargs):
        FormatWidgetMixin.__init__(self, format_=format_, **kwargs)
        WidthWidgetMixin.__init__(self, **kwargs)

    def __call__(self, progress, data, **kwargs):
        if not self.check_size(progress):
            return ''

        for name, (key, transform) in list(self.mapping.items()):
            try:
                if transform is None:
                    data[name] = data[key]
                else:
                    data[name] = transform(data[key])
            except (KeyError, ValueError, IndexError):
                pass

        return FormatWidgetMixin.__call__(self, progress, data, **kwargs)


class TimeSensitiveWidgetBase(WidgetBase):
    INTERVAL = timedelta(milliseconds=100)


class Timer(FormatLabel, TimeSensitiveWidgetBase):
    def __init__(self, format_='Elapsed Time: %(elapsed)s', **kwargs):
        FormatLabel.__init__(self, format_=format_, **kwargs)
        TimeSensitiveWidgetBase.__init__(self, **kwargs)

    format_time = staticmethod(format_time)


class ETA(Timer):
    def __init__(
            self,
            format_not_started='ETA:  --:--:--',
            format_finished='Time: %(elapsed)s',
            format_='ETA:  %(eta)s',
            format_zero='ETA:  0:00:00',
            format_NA='ETA:      N/A',
            **kwargs):

        Timer.__init__(self, **kwargs)
        self.format_not_started = format_not_started
        self.format_finished = format_finished
        self.format_ = format_
        self.format_zero = format_zero
        self.format_NA = format_NA

    def _calculate_eta(self, progress, data, value, elapsed):
        if elapsed:
            per_item = elapsed / max(value, 0.000000001)
            remaining = progress.max_value - data['value']
            eta_seconds = remaining * per_item
        else:
            eta_seconds = 0
        return eta_seconds

    def __call__(self, progress, data, value=None, elapsed=None):
        if value is None:
            value = data['value']
        if elapsed is None:
            elapsed = data['total_seconds_elapsed']

        ETA_NA = False
        try:
            data['eta_seconds'] = self._calculate_eta(progress, data, value=value, elapsed=elapsed)
        except TypeError:
            data['eta_seconds'] = None
            ETA_NA = True

        if data['eta_seconds']:
            data['eta'] = format_time(data['eta_seconds'])
        else:
            data['eta'] = None

        if data['value'] == progress.min_value:
            format_ = self.format_not_started
        elif progress.end_time:
            format_ = self.format_finished
        elif data['eta']:
            format_ = self.format_
        elif ETA_NA:
            format_ = self.format_NA
        else:
            format_ = self.format_zero

        return Timer.__call__(self, progress, data, format_=format_)


class FileTransferSpeed(FormatWidgetMixin, TimeSensitiveWidgetBase):
    def __init__(
            self, format_='%(scaled)5.1f %(prefix)s%(unit)-s/s',
            inverse_format='%(scaled)5.1f s/%(prefix)s%(unit)-s', unit='B',
            prefixes=('', 'Ki', 'Mi', 'Gi', 'Ti', 'Pi', 'Ei', 'Zi', 'Yi'),
            **kwargs):
        self.unit = unit
        self.prefixes = prefixes
        self.inverse_format = inverse_format
        FormatWidgetMixin.__init__(self, format_=format_, **kwargs)
        TimeSensitiveWidgetBase.__init__(self, **kwargs)

    def _speed(self, value, elapsed):
        speed = float(value) / elapsed
        return scale_1024(speed, len(self.prefixes))

    def __call__(self, progress, data, value=None, total_seconds_elapsed=None):
        value = data['value'] or value
        elapsed = data['total_seconds_elapsed'] or total_seconds_elapsed

        if value is not None and elapsed is not None \
                and elapsed > 2e-6 and value > 2e-6:  # =~ 0
            scaled, power = self._speed(value, elapsed)
        else:
            scaled = power = 0

        data['unit'] = self.unit
        if power == 0 and scaled < 0.1:
            if scaled > 0:
                scaled = 1 / scaled
            data['scaled'] = scaled
            data['prefix'] = self.prefixes[0]
            return FormatWidgetMixin.__call__(self, progress, data,
                                              self.inverse_format)
        else:
            data['scaled'] = scaled
            data['prefix'] = self.prefixes[power]
            return FormatWidgetMixin.__call__(self, progress, data)


def get_terminal_size():
    import shutil
    w, h = shutil.get_terminal_size()
    return w - 1, h


class ResizableMixin(ProgressBarMixinBase):

    def __init__(self, term_width=None, **kwargs):
        ProgressBarMixinBase.__init__(self, **kwargs)

        self.signal_set = False
        if term_width:
            self.term_width = term_width
        else:
            try:
                self._handle_resize()
                import signal
                self._prev_handle = signal.getsignal(signal.SIGWINCH)
                signal.signal(signal.SIGWINCH, self._handle_resize)
                self.signal_set = True
            except Exception:
                pass

    def _handle_resize(self):
        w, h = get_terminal_size()
        self.term_width = w

    def finish(self):
        ProgressBarMixinBase.finish(self)
        if self.signal_set:
            try:
                import signal
                signal.signal(signal.SIGWINCH, self._prev_handle)
            except Exception:
                pass


class StdRedirectMixin(DefaultFdMixin):

    def __init__(self, redirect_stderr=False, redirect_stdout=False, **kwargs):
        DefaultFdMixin.__init__(self, **kwargs)
        self.redirect_stderr = redirect_stderr
        self.redirect_stdout = redirect_stdout

    def start(self, *args, **kwargs):
        if self.redirect_stdout:
            streams.wrap_stdout()

        if self.redirect_stderr:
            streams.wrap_stderr()

        self.stdout = streams.stdout
        self.stderr = streams.stderr

        streams.start_capturing()
        DefaultFdMixin.start(self, **kwargs)

    def update(self, value=None):
        self.fd.write('\r' + ' ' * self.term_width + '\r')
        streams.flush()
        DefaultFdMixin.update(self, value=value)

    def finish(self, end='\n'):
        DefaultFdMixin.finish(self, end=end)
        streams.stop_capturing()
        if self.redirect_stdout:
            streams.unwrap_stdout()

        if self.redirect_stderr:
            streams.unwrap_stderr()


class ProgressBar(StdRedirectMixin, ResizableMixin, ProgressBarBase):

    _DEFAULT_MAXVAL = 100
    _MINIMUM_UPDATE_INTERVAL = 0.05  # update up to a 20 times per second

    def __init__(self, widgets, min_value=0, max_value=None,
                 left_justify=True, initial_value=0, poll_interval=None,
                 custom_len=len, max_error=True,
                 **kwargs):
        StdRedirectMixin.__init__(self, **kwargs)
        ResizableMixin.__init__(self, **kwargs)
        ProgressBarBase.__init__(self, **kwargs)
        if not max_value and kwargs.get('maxval') is not None:
            max_value = kwargs.get('maxval')

        if not poll_interval and kwargs.get('poll'):
            poll_interval = kwargs.get('poll')

        if max_value:
            if min_value > max_value:
                raise ValueError('Max value needs to be bigger than the min value')
        self.min_value = min_value
        self.max_value = max_value
        self.max_error = max_error
        self.widgets = widgets
        self.left_justify = left_justify
        self.value = initial_value
        self._iterable = None
        self.custom_len = custom_len
        self.init()

        if poll_interval and isinstance(poll_interval, (int, float)):
            poll_interval = timedelta(seconds=poll_interval)

        self.poll_interval = poll_interval
        self.dynamic_messages: Dict = {}

    def init(self):
        self.previous_value = None
        self.start_time = None
        self.updates = 0
        self.end_time = None

    @property
    def percentage(self):
        if self.max_value is None:
            return None
        elif self.max_value:
            todo = self.value - self.min_value
            total = self.max_value - self.min_value
            percentage = todo / total
        else:
            percentage = 1

        return percentage * 100

    def get_last_update_time(self):
        if self._last_update_time:
            return datetime.fromtimestamp(self._last_update_time)

    def set_last_update_time(self, value):
        if value:
            self._last_update_time = time.mktime(value.timetuple())
        else:
            self._last_update_time = None

    last_update_time = property(get_last_update_time, set_last_update_time)

    def data(self):
        self._last_update_time = time.time()
        elapsed = self.last_update_time - self.start_time
        total_seconds_elapsed = timedelta_to_seconds(elapsed)
        return dict(
            max_value=self.max_value,
            start_time=self.start_time,
            last_update_time=self.last_update_time,
            end_time=self.end_time,
            value=self.value,
            previous_value=self.previous_value,
            updates=self.updates,
            total_seconds_elapsed=total_seconds_elapsed,
            seconds_elapsed=(elapsed.seconds % 60) +
            (elapsed.microseconds / 1000000.),
            minutes_elapsed=(elapsed.seconds / 60) % 60,
            hours_elapsed=(elapsed.seconds / (60 * 60)) % 24,
            days_elapsed=(elapsed.seconds / (60 * 60 * 24)),
            time_elapsed=elapsed,
            percentage=self.percentage,
            dynamic_messages=self.dynamic_messages
        )

    def __call__(self, iterable, max_value=None):
        if max_value is None:
            try:
                self.max_value = len(iterable)
            except TypeError:
                self.max_value = None
        else:
            self.max_value = max_value

        self._iterable = iter(iterable)
        return self

    def __iter__(self):
        return self

    def __next__(self):
        try:
            value = next(self._iterable)
            if self.start_time is None:
                self.start()
            else:
                self.update(self.value + 1)
            return value
        except StopIteration:
            self.finish()
            raise

    def __exit__(self, exc_type, exc_value, traceback):
        self.finish()

    def __enter__(self):
        return self.start()

    def __iadd__(self, value):
        self.update(self.value + value)
        return self

    def _format_widgets(self):
        result: List = []
        expanding: List = []
        width = self.term_width
        data = self.data()

        for index, widget in enumerate(self.widgets):
            if isinstance(widget, AutoWidthWidgetBase):
                result.append(widget)
                expanding.insert(0, index)
            elif isinstance(widget, str):
                result.append(widget)
                width -= self.custom_len(widget)
            else:
                widget_output = to_unicode(widget(self, data))
                result.append(widget_output)
                width -= self.custom_len(widget_output)

        count = len(expanding)
        while expanding:
            portion = max(int(math.ceil(width * 1. / count)), 0)
            index = expanding.pop()
            widget = result[index]
            count -= 1

            widget_output = widget(self, data, portion)
            width -= self.custom_len(widget_output)
            result[index] = widget_output

        return result

    @classmethod
    def _to_unicode(cls, args):
        for arg in args:
            yield to_unicode(arg)

    def _format_line(self):

        widgetzzz = ''.join(self._to_unicode(self._format_widgets()))

        if self.left_justify:
            return widgetzzz.ljust(self.term_width)
        else:
            return widgetzzz.rjust(self.term_width)

    def _needs_update(self):
        if self.poll_interval:
            delta = datetime.now() - self.last_update_time
            poll_status = delta > self.poll_interval
        else:
            poll_status = False

        try:
            divisor = self.max_value / self.term_width  # float division
            if self.value // divisor == self.previous_value // divisor:
                return poll_status or self.end_time
        except Exception:
            pass

        return self.value > self.next_update or poll_status or self.end_time

    def update(self, value=None, force=False, **kwargs):
        if self.start_time is None:
            self.start()
            return self.update(value, force=force, **kwargs)

        if value is not None:
            if self.max_value is None:
                pass
            elif self.min_value <= value <= self.max_value:
                pass
            elif self.max_error:
                msg_tail = f"should be between {self.min_value} and {self.max_value}"
                raise ValueError(f"Value {value} is out of range, {msg_tail}")
            else:
                self.max_value = value

            self.previous_value = self.value
            self.value = value

        minimum_update_interval = self._MINIMUM_UPDATE_INTERVAL
        update_delta = time.time() - self._last_update_time
        if update_delta < minimum_update_interval and not force:
            return

        for key in kwargs:
            if key in self.dynamic_messages:
                self.dynamic_messages[key] = kwargs[key]
            else:
                raise TypeError(f"update() got an unexpected keyword {key}")

        if self._needs_update() or force:
            self.updates += 1
            ResizableMixin.update(self, value=value)
            ProgressBarBase.update(self, value=value)
            StdRedirectMixin.update(self, value=value)

            self.fd.flush()

    def start(self, max_value=None, init=True):
        if init:
            self.init()

        StdRedirectMixin.start(self, max_value=max_value)
        ResizableMixin.start(self, max_value=max_value)
        ProgressBarBase.start(self, max_value=max_value)

        self.max_value = max_value or self.max_value
        if self.max_value is None:
            self.max_value = self._DEFAULT_MAXVAL

        for widget in self.widgets:
            interval = getattr(widget, 'INTERVAL', None)
            if interval is not None:
                self.poll_interval = min(
                    self.poll_interval or interval,
                    interval,
                )

        self.next_update = 0

        if self.max_value is None and self.max_value < 0:
            raise ValueError('Value out of range')

        self.start_time = self.last_update_time = datetime.now()
        self.update(self.min_value, force=True)

        return self

    def finish(self, end='\n'):
        self.end_time = datetime.now()
        self.update(self.max_value, force=True)

        StdRedirectMixin.finish(self, end=end)
        ResizableMixin.finish(self)
        ProgressBarBase.finish(self)


def very_pretty_print(s, d):
    ss = s.split(os.sep)
    dd = d.split(os.sep)
    del ss[:3]  # no /home/user
    del dd[:4]  # no /run/media/user
    s = os.sep.join(ss)
    d = os.sep.join(dd)
    print(f"\n\n{s} -> {d}")


def copy_with_progress(fn, s, d):
    very_pretty_print(s, d)
    blocksize = 16 * 1024
    filesize = os.path.getsize(s)
    if filesize < blocksize:
        return  # ^_^
    numblocks = filesize / blocksize
    math_ceil = math.ceil(numblocks)
    if math_ceil > 10:
        blocksize = (filesize + 2000) // 10
        numblocks = filesize / blocksize
        math_ceil = math.ceil(numblocks)
    if filesize > 32 * 1024 * 1024:
        blocksize = 3 * 1024 * 1024
        numblocks = filesize / blocksize
        math_ceil = math.ceil(numblocks)
    print(f"{fn}: {filesize:_} bytes, {math_ceil} blocks")

    widgets = [
        f"{fn}: ", Percentage(),
        ' ', Bar(),
        ' ', ETA(),
        ' ', FileTransferSpeed(),
    ]
    bar = ProgressBar(widgets, max_value=filesize).start()

    try:
        os.makedirs(os.path.dirname(d))
    except Exception as err:
        pass

    with open(s, 'rb') as fsrc, open(d, 'wb') as fdst:
        copied = 0
        for dummy in range(math_ceil):
            buf = fsrc.read(blocksize)
            if not buf:
                break
            fdst.write(buf)
            if 0 == dummy % 10:
                os.sync()
            copied += len(buf)
            bar.update(copied)
        bar.finish()


def walk(p, dest, callback):
    old  = os.listdir(p)
    opj  = os.path.join
    dirs = sorted([x for x in old if os.path.isdir(opj(p, x))])
    filz = sorted([x for x in old if os.path.isfile(opj(p, x))])
    for dn in dirs:
        src = opj(p,    dn)
        dst = opj(dest, dn)
        walk(src, dst, callback)
    if filz:
        callback(p, dest, filz)


def getdest(path):
    runm = "/run/media/"
    users = os.listdir(runm)
    if 1 != len(users):
        raise Exception(f"WTF in {runm} --> {users}")
    user = users[0]
    runm = os.path.join(runm, user)
    usbs = os.listdir(runm)
    if 1 != len(usbs):
        raise Exception(f"WTF in {runm} --> {usbs}")
    usb = usbs[0]
    runm = os.path.join(runm, usb)
    return os.path.join(runm, path)  # "/run/media/user/usb/Music


if __name__ == "__main__":
    def process(src, dest, files):
        for fn in files:
            fullsrc = os.path.join(src, fn)
            fulldest = os.path.join(dest, fn)
            copy_with_progress(fn, fullsrc, fulldest)

    src = "/home/sheerluck/Архив/Музыка"
    dest = getdest(os.path.basename(src))
    walk(src, dest, process)
