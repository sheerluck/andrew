Python Tricks
=============

Here, I am collecting python snippets that I find enlightening and/or
just useful.  On a subpage__, you find a JPype-using hack to access
Weka's Java classes from Python.

__ weka-python/

Additionally, I want to share some interesting links:

- When working with files or directories (using ``os.path``), you can
  make your live much easier if you check out the ``path`` `Python
  module`__. It is very simple, but incredibly useful!
- Another great python module is BeautifulSoup_, which can be used
  to parse real-world HTML (or XML) files.  It offers a very simple
  API for traversing the resulting parse tree, and one of its main
  features is that it also groks non-conformant HTML code (as good as
  possible) without choking.
- Whenever you want to parse anything more complex than what regular
  expressions support (i.e., sth. beyond `regular languages
  <http://en.wikipedia.org/wiki/Regular_language>`_), e.g. expressions
  that need to be properly quoted or with nested parentheses, I
  propose to use pyparsing_, another real gem which allows you to
  write parsers in the most intuitive way possible.  (Incidentally, it
  is similar to the `boost::spirit <http://spirit.sourceforge.net/>`_
  C++ library.)
- In the `Python Wiki`_, you can find some hints on speeding up your
  programs in the PythonSpeed_ page, and its PerformanceTips_
  sub-page.
- Concerning speed, I found Psyco_ particularly interesting, which is
  an easy-to-use extension module that can speed up the execution of
  any Python code (I purposely stripped the "massively" from this
  description, but still think it's cool, since you basically get the
  speed "for free").
- In general, I don't like these code snippet-websites (especially if
  you have to register only to discover ugly code), and this is not
  very different for ASPN, but I like this `function for
  pretty-printing a table
  <http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/267662>`_.
  It's self-contained, just don't look at the code but give it a list
  of rows (lists of column contents), and it will give you a layouted
  string for printing a table.
- Andrew Kuchling has written a `tutorial on Unicode in Python
  <http://www.amk.ca/python/howto/unicode>`_ which was recommended on
  the IPython mailing list.
- http://codespeak.net/py/current/doc/ - XP Testing framework

__ http://www.jorendorff.com/articles/python/path/
.. _BeautifulSoup: http://www.crummy.com/software/BeautifulSoup/
.. _pyparsing: http://pyparsing.sourceforge.net/
.. _Python Wiki: http://wiki.python.org/moin/
.. _PythonSpeed: http://wiki.python.org/moin/PythonSpeed
.. _PerformanceTips: http://wiki.python.org/moin/PythonSpeed/PerformanceTips
.. _Psyco: http://psyco.sourceforge.net/

Uncluttered, Clean Imports
--------------------------

I tend to write code in an unordered way, usually copy-pasting stuff
from and to IPython_.  However, eventually I clean up the code and
want to make it reusable as a module, and for a .py file to be
importable, it is often necessary to add a number of missing imports.
Due to dynamic binding, Python does not complain about missing imports
until the code in question is actually run, which can be a problem if
e.g. you have a function that outputs an error via sys.stderr only in
a seldom error case, but you have not imported sys.

pyflakes_ is the solution to this problem.  It neatly checks whether
you have imported everything you use, and also whether you have
imported something twice or are not using an imported module at all,
which lets you quickly clean up your module - as a side note: Of
course, ``from foo import *`` makes problems (since the code is not
run at all), but I would not recommend its use anyway.  It's usually
better (and more zen-ish) to explicitly import all names you're using,
and pyflakes_ perfectly helps with that, too: simply comment out the
import and fix the warnings that appear.  Also recall that you can do
things like ``import numpy as np`` and then explicitly prefix all
numpy usage with "np.".

.. _pyflakes: http://divmod.org/trac/wiki/DivmodPyflakes

Completion in the Python Console
--------------------------------

It is handy to be able to tab-complete properties of python objects at
the python prompt.  Nowadays, I am always using IPython_, a
*significantly* enhanced interactive python console which is really
worth the installation.  (It helps you with completion, indentation,
syntax highlighting, macros, input/output caching, session management,
improved history, debugger, and tracebacks. And more. ;-) )

However, I find it interesting to note that it is possible to have
*completion in the standard python console* (if compiled with
readline, which it really should be)!  The following code can be used
to activate it:

.. code-block:: python

  # .pythonrc.py
  import readline, rlcompleter
  readline.parse_and_bind("tab: complete")

Put this code in a file ``~/.pythonrc.py`` or similar, and use the
variable ``PYTHONSTARTUP`` to point python to it!  (I.e., I put
"``export PYTHONSTARTUP=$HOME/.pythonrc.py``" into my shell
environment setup.)

Now, go check out IPython_. ;-)

.. _IPython: http://ipython.scipy.org/

Removing Duplicates from Lists
------------------------------

If you want to remove duplicates from a list, just put every element
into a dict as a key (e.g. with ``None`` as value) and check
``dict.keys()``.  I found this optimized version in the WWW:

.. code-block:: python

  from operator import setitem
  def distinct(l):
      d = {}
      map(setitem, (d,)*len(l), l, [])
      return d.keys()

This makes use of the fact that ``map`` fills up shorter lists (in
this case the empty one) with ``None``.  Newer Python versions
allow for an even more concise formulation of the above: [*]_

.. code-block:: python

  def distinct(l):
      return dict.fromkeys(l).keys() # works with python 2.3+

  def distinct24(l):
      return list(set(l)) # needs python 2.4 or later

These are clearly concise enough to be used in-place.  Note that all
variants so far have two limitations:

* The elements are returned in arbitrary order.

* Lists with unhashable elements (e.g. sub-lists) may not be
  processed.

Flattening Lists
----------------

I guess one reason why there is no built-in ``flatten`` function in
Python is that there are several open semantic questions which are not
intuitively answered:

* *What* is flattened, e.g. all lists within a list?  Or tuples, too?
  What about other sequence types (sub-classes of the above,
  home-brewn vectors, ...)?  For my version of ``flatten`` below, I
  assumed that all iterable types (except strings) should be
  flattened.  It is straight-forward to change that, e.g. by
  uncommenting the isinstance-check.

* Does the function work recursively or not?  I feel that most people
  would expect the function to really return a *flat* list,
  i.e. recursivly flatten all contained sequences.

But talking about python tricks, I should first mention my solution to
the most common instance of the flattening problem:

.. code-block:: python

  # pass all elements from all lists within someLists to someFunc:
  someFunc(sum(someLists, []))

Actually, this is precisely the only flattening which I regularly
need, and it's a neat in-place solution.  But anyhow, here's the
complete, recursive variant:

.. code-block:: python

  def flatten(x):
      """flatten(sequence) -> list

      Returns a single, flat list which contains all elements retrieved
      from the sequence and all recursively contained sub-sequences
      (iterables).

      Examples:
      >>> [1, 2, [3,4], (5,6)]
      [1, 2, [3, 4], (5, 6)]
      >>> flatten([[[1,2,3], (42,None)], [4,5], [6], 7, MyVector(8,9,10)])
      [1, 2, 3, 42, None, 4, 5, 6, 7, 8, 9, 10]"""

      result = []
      for el in x:
          #if isinstance(el, (list, tuple)):
          if hasattr(el, "__iter__") and not isinstance(el, basestring):
              result.extend(flatten(el))
          else:
              result.append(el)
      return result

Float Formatting
----------------

Somehow, I often don't find the right formatting flags for nice output
of floating point numbers.  Thus, I created this "little" table
demonstrating many of the `available options`__ with some simple
numbers:

=======  ==============  ==============  ====================  ====================  ==============  ==============  ==============
     %s               0               1         3.14159265359                   2.3           0.001           1e-10             100
   %.3s               0               1                   3.1                   2.3             0.0             1e-             100
     %r               0               1    3.1415926535897931    2.2999999999999998           0.001           1e-10             100
     %f        0.000000        1.000000              3.141593              2.300000        0.001000        0.000000      100.000000
   %.2f            0.00            1.00                  3.14                  2.30            0.00            0.00          100.00
    %.f               0               1                     3                     2               0               0             100
   %#.f              0.              1.                    3.                    2.              0.              0.            100.
     %e    0.000000e+00    1.000000e+00          3.141593e+00          2.300000e+00    1.000000e-03    1.000000e-10    1.000000e+02
   %.2e        0.00e+00        1.00e+00              3.14e+00              2.30e+00        1.00e-03        1.00e-10        1.00e+02
   %#.e          0.e+00          1.e+00                3.e+00                2.e+00          1.e-03          1.e-10          1.e+02
     %g               0               1               3.14159                   2.3           0.001           1e-10             100
    %#g         0.00000         1.00000               3.14159               2.30000      0.00100000     1.00000e-10         100.000
   %.2g               0               1                   3.1                   2.3           0.001           1e-10           1e+02
  %#.2g             0.0             1.0                   3.1                   2.3          0.0010         1.0e-10         1.0e+02
    %.g               0               1                     3                     2           0.001           1e-10           1e+02
   %#.g              0.              1.                    3.                    2.           0.001          1.e-10          1.e+02
=======  ==============  ==============  ====================  ====================  ==============  ==============  ==============

Alas, I still did not find the optimal formatting.  What I am looking
for, is a quick way to

a) display pi as 3.14159 (i.e. have 5 fractional digits)
b) display 3.1 as 3.1 (no trailing zeros)
c) display 1.234e-13 as 0 and 2.3e+02 as 230 (no exponential display)
d) ideally, display -1.234e-13 as 0, too (not as -0)

Obviously, there is no single format specification that produces the
desired output in all these cases, but I would like to be taught
otherwise.  The closest thing seems to be %s, which only uses
exponential display for extreme cases, but defaults to too many
decimal digits for my taste.  %g is better with the latter, but
already uses exponential notation for smaller exponents.

__ http://docs.python.org/lib/typesseq-strings.html

Reverse Iteration of Lists / Generators
---------------------------------------

The following handy little function fulfills two purposes.  First, it
lets you conveniently write

.. code-block:: python

  for el in reviter(somelist):
      do_something(el)

following the ``iter(somelist)`` example, and second, it reminds you
of the incredibly useful ``yield``-construct (which is `new since 2.3
<http://www.python.org/doc/2.3/whatsnew/section-generators.html>`_),
which lets you define `generators`.  This is the most natural way of
defining complicated iterators IMO:

.. code-block:: python

  def reviter(x):
      if hasattr(x, 'keys'):
          raise ValueError("mappings do not support reverse iteration")
      i = len(x)
      while i > 0:
          i -= 1
          yield x[i]

The first ``yield`` will store the entire state of the function in an
object that serves as an iterator which returns all yield`ed values
and throws a ``StopIteration`` when the original function returns.

Note that Python 2.4 `already brings "reversed"
<http://www.python.org/doc/2.4.4/whatsnew/node7.html>`_ for this exact
purpose.

It's even possible - and very useful indeed - to have several yields
in one generator function.  E.g., the following code is from my
`fig.py <software/figpy>`_ module and is used for writing coordinate
pairs of open/closed polygons in rows of 6 points / 12 coordinates
each (beyond ``yield``, it also demonstrates a clever use of ``map`` for
grouping N=12 iterated elements per row):

.. code-block:: python

  class PolylineBase(Object):
      # ...

      def _savePointIter(self):
          # flatten a point list into x1, y1, x2, y2, x2, ...
          for p in self.points:
              yield p[0]
              yield p[1]
          # of course, yields can also be conditional,
          # here for repeating the first coordinate pair of closed polygons:
          if self.closed():
              yield self.points[0][0]
              yield self.points[0][1]

      def __str__(self):
          # ...
          i = self._savePointIter()
          # multiply (i, ) with 12 to get 12 references of the same iterator,
          # and exploit the fact that map(None, ...) "fills up"
          # exhausted iterators with None:
          for linePoints in map(None, *(i, )*12):
              # linePoints now contains 12 coordinates (or None values at the end):
              result += "\t" + " ".join(*[str(p) for p in linePoints if p is not None]) + "\n"
          # ...

Substitute for Missing ?: Operator
----------------------------------

If you are used to programming in C-like languages, and you are
working with Python versions before 2.5, you probably missed an
if-then-else operator.  There *is* something that can fill the gap in
many cases, just not in all: You can use the boolean operators'
short-circuiting property by writing:

.. code-block:: python

  something = condition and true_value or false_value

If ``condition`` has a value that counts as ''False'', the
``and``-operator will not evaluate ``true_value`` but return
''False''.  Actually, it will *not* simply return ''False'', but it
will return the value of ``condition`` which is known to be some kind
of ``False``.  This special way in which the boolean operators work,
that they simply return one of their arguments and not just ``True``
or ``False``, is used to assign ``true_value`` or ``false_value`` to
the variable ``something``.

.. CAUTION::
  ``true_value`` must be ``__nonzero__`` for the above to work,
  otherwise you'll get strange results:

  .. code-block:: python

    this_will_be_two = cond and None or 2 # WRONG!

  Since ``None`` will be like ``False`` to the operators, the result
  will be ``2`` regardless of the value of ``cond``.  (You *could* use
  ``not cond and 2 or None`` instead.)

So, this is actually an unreadable, confusing, and possibly dangerous
syntax, but nevertheless handy sometimes. ;-)

You will also occasionally find the variant [*]_

.. code-block:: python

  something = (false_value, true_value)[condition]

which exploits the fact that ``bool`` is in fact derived from ``int``
and ``False``/``True`` is ``0``/``1``, too (write ``bool(condition)``
if condition is not ``0``/``1``).  However, there is no
short-circuiting here but both ``false_value`` and ``true_value`` will
be evaluated, which is another big caveat for many applications.

If you wonder why this operator is missing, and whether you're the
first to miss it; you're of course *not*, but for a long time, the
Python community could not agree on a syntax for the ternary operator,
see the discussion in `PEP 308`__. Nevertheless, Guido himself at last
decided to integrate it into the language, choosing the syntax
``true_value if condition else false_value`` from the most popular
variants (this is finally `included in Python 2.5
<http://www.python.org/doc/2.5/whatsnew/pep-308.html>`_).

__ http://www.python.org/peps/pep-0308.html

Do-While Loops
--------------

Since python has no do-while or do-until loop constructs (yet), the
following idiom has become very common:

.. code-block:: python

  while True:
      do_something()
      if condition():
          break

Stick to it, and it'll soon become very familiar to you.

.. [*] Thanks to Paddy McCarthy for feedback on all this and some
       pointers to variants using newer Python features.

.. [*] And thanks to Pete Forman for reminding me of the no-short-circuit
       ``cond[bool]`` variant.
